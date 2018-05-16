#ifndef QTPROMISE_QPROMISE_P_H
#define QTPROMISE_QPROMISE_P_H

// QtPromise
#include "qpromiseglobal.h"

// Qt
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QThread>
#include <QVector>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QSharedData>
#include <QPointer>

namespace QtPromise {

template <typename T>
class QPromise;

template <typename T>
class QPromiseResolve;

template <typename T>
class QPromiseReject;

} // namespace QtPromise

namespace QtPromisePrivate {

// https://stackoverflow.com/a/21653558
template <typename F>
static void qtpromise_defer(F&& f, const QPointer<QThread>& thread)
{
    using FType = typename std::decay<F>::type;

    struct Event : public QEvent
    {
        Event(FType&& f) : QEvent(QEvent::None), m_f(std::move(f)) { }
        Event(const FType& f) : QEvent(QEvent::None), m_f(f) { }
        ~Event() { m_f(); }
        FType m_f;
    };

    if (!thread || thread->isFinished()) {
        // Make sure to not call `f` if the captured thread doesn't exist anymore,
        // which would potentially result in dispatching to the wrong thread (ie.
        // nullptr == current thread). Since the target thread is gone, it should
        // be safe to simply skip that notification.
        return;
    }

    QObject* target = QAbstractEventDispatcher::instance(thread);
    if (!target && QCoreApplication::closingDown()) {
        // When the app is shutting down, the even loop is not anymore available
        // so we don't have any way to dispatch `f`. This case can happen when a
        // promise is resolved after the app is requested to close, in which case
        // we should not trigger any error and skip that notification.
        return;
    }

    Q_ASSERT_X(target, "postMetaCall", "Target thread must have an event loop");
    QCoreApplication::postEvent(target, new Event(std::forward<F>(f)));
}

template <typename F>
static void qtpromise_defer(F&& f)
{
    Q_ASSERT(QThread::currentThread());
    qtpromise_defer(std::forward<F>(f), QThread::currentThread());
}

template <typename T>
class PromiseValue
{
public:
    PromiseValue() { }
    PromiseValue(const T& data) : m_data(new T(data)) { }
    PromiseValue(T&& data) : m_data(new T(std::move(data))) { }
    bool isNull() const { return m_data.isNull(); }
    const T& data() const { return *m_data; }

private:
    QSharedPointer<T> m_data;
};

class PromiseError
{
public:
    template <typename T>
    PromiseError(const T& value)
    {
        try {
            throw value;
        } catch (...) {
            m_data = std::current_exception();
        }
    }

    PromiseError() { }
    PromiseError(const std::exception_ptr& exception) : m_data(exception) { }
    void rethrow() const { std::rethrow_exception(m_data); }
    bool isNull() const { return m_data == nullptr; }

private:
    // NOTE(SB) std::exception_ptr is already a shared pointer
    std::exception_ptr m_data;
};

template <typename T>
struct PromiseDeduce
{
    using Type = QtPromise::QPromise<Unqualified<T>>;
};

template <typename T>
struct PromiseDeduce<QtPromise::QPromise<T>>
    : public PromiseDeduce<T>
{ };

template <typename T>
struct PromiseFulfill
{
    static void call(
        T&& value,
        const QtPromise::QPromiseResolve<T>& resolve,
        const QtPromise::QPromiseReject<T>&)
    {
        resolve(std::move(value));
    }
};

template <typename T>
struct PromiseFulfill<QtPromise::QPromise<T>>
{
    static void call(
        const QtPromise::QPromise<T>& promise,
        const QtPromise::QPromiseResolve<T>& resolve,
        const QtPromise::QPromiseReject<T>& reject)
    {
        if (promise.isFulfilled()) {
            resolve(promise.m_d->value());
        } else if (promise.isRejected()) {
            reject(promise.m_d->error());
        } else {
            promise.then([=]() {
                resolve(promise.m_d->value());
            }, [=]() { // catch all
                reject(promise.m_d->error());
            });
        }
    }
};

template <>
struct PromiseFulfill<QtPromise::QPromise<void>>
{
    template <typename TPromise, typename TResolve, typename TReject>
    static void call(
        const TPromise& promise,
        const TResolve& resolve,
        const TReject& reject)
    {
        if (promise.isFulfilled()) {
            resolve();
        } else if (promise.isRejected()) {
            reject(promise.m_d->error());
        } else {
            promise.then([=]() {
                resolve();
            }, [=]() { // catch all
                reject(promise.m_d->error());
            });
        }
    }
};

template <typename T, typename TRes>
struct PromiseDispatch
{
    using Promise = typename PromiseDeduce<TRes>::Type;
    using ResType = Unqualified<TRes>;

    template <typename THandler, typename TResolve, typename TReject>
    static void call(const T& value, THandler handler, const TResolve& resolve, const TReject& reject)
    {
        try {
            PromiseFulfill<ResType>::call(handler(value), resolve, reject);
        } catch (...) {
            reject(std::current_exception());
        }
    }
};

template <typename T>
struct PromiseDispatch<T, void>
{
    using Promise = QtPromise::QPromise<void>;

    template <typename THandler, typename TResolve, typename TReject>
    static void call(const T& value, THandler handler, const TResolve& resolve, const TReject& reject)
    {
        try {
            handler(value);
            resolve();
        } catch (...) {
            reject(std::current_exception());
        }
    }
};

template <typename TRes>
struct PromiseDispatch<void, TRes>
{
    using Promise = typename PromiseDeduce<TRes>::Type;
    using ResType = Unqualified<TRes>;

    template <typename THandler, typename TResolve, typename TReject>
    static void call(THandler handler, const TResolve& resolve, const TReject& reject)
    {
        try {
            PromiseFulfill<ResType>::call(handler(), resolve, reject);
        } catch (...) {
            reject(std::current_exception());
        }
    }
};

template <>
struct PromiseDispatch<void, void>
{
    using Promise = QtPromise::QPromise<void>;

    template <typename THandler, typename TResolve, typename TReject>
    static void call(THandler handler, const TResolve& resolve, const TReject& reject)
    {
        try {
            handler();
            resolve();
        } catch (...) {
            reject(std::current_exception());
        }
    }
};

template <typename T, typename THandler, typename TArg = typename ArgsOf<THandler>::first>
struct PromiseHandler
{
    using ResType = typename std::result_of<THandler(T)>::type;
    using Promise = typename PromiseDispatch<T, ResType>::Promise;

    template <typename TResolve, typename TReject>
    static std::function<void(const T&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const T& value) {
            PromiseDispatch<T, ResType>::call(value, std::move(handler), resolve, reject);
        };
    }
};

template <typename T, typename THandler>
struct PromiseHandler<T, THandler, void>
{
    using ResType = typename std::result_of<THandler()>::type;
    using Promise = typename PromiseDispatch<T, ResType>::Promise;

    template <typename TResolve, typename TReject>
    static std::function<void(const T&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const T&) {
            PromiseDispatch<void, ResType>::call(handler, resolve, reject);
        };
    }
};

template <typename THandler>
struct PromiseHandler<void, THandler, void>
{
    using ResType = typename std::result_of<THandler()>::type;
    using Promise = typename PromiseDispatch<void, ResType>::Promise;

    template <typename TResolve, typename TReject>
    static std::function<void()> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=]() {
            PromiseDispatch<void, ResType>::call(handler, resolve, reject);
        };
    }
};

template <typename T>
struct PromiseHandler<T, std::nullptr_t, void>
{
    using Promise = QtPromise::QPromise<T>;

    template <typename TResolve, typename TReject>
    static std::function<void(const T&)> create(
        std::nullptr_t,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const T& value) {
            // 2.2.7.3. If onFulfilled is not a function and promise1 is fulfilled,
            // promise2 must be fulfilled with the same value as promise1.
            PromiseFulfill<T>::call(std::move(T(value)), resolve, reject);
        };
    }
};

template <>
struct PromiseHandler<void, std::nullptr_t, void>
{
    using Promise = QtPromise::QPromise<void>;

    template <typename TResolve, typename TReject>
    static std::function<void()> create(
        std::nullptr_t,
        const TResolve& resolve,
        const TReject&)
    {
        return [=]() {
            // 2.2.7.3. If onFulfilled is not a function and promise1 is fulfilled,
            // promise2 must be fulfilled with the same value as promise1.
            resolve();
        };
    }
};

template <typename T, typename THandler, typename TArg = typename ArgsOf<THandler>::first>
struct PromiseCatcher
{
    using ResType = typename std::result_of<THandler(TArg)>::type;

    template <typename TResolve, typename TReject>
    static std::function<void(const PromiseError&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const PromiseError& error) {
            try {
                error.rethrow();
            } catch (const TArg& error) {
                PromiseDispatch<TArg, ResType>::call(error, handler, resolve, reject);
            } catch (...) {
                reject(std::current_exception());
            }
        };
    }
};

template <typename T, typename THandler>
struct PromiseCatcher<T, THandler, void>
{
    using ResType = typename std::result_of<THandler()>::type;

    template <typename TResolve, typename TReject>
    static std::function<void(const PromiseError&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const PromiseError& error) {
            try {
                error.rethrow();
            } catch (...) {
                PromiseDispatch<void, ResType>::call(handler, resolve, reject);
            }
        };
    }
};

template <typename T>
struct PromiseCatcher<T, std::nullptr_t, void>
{
    template <typename TResolve, typename TReject>
    static std::function<void(const PromiseError&)> create(
        std::nullptr_t,
        const TResolve&,
        const TReject& reject)
    {
        return [=](const PromiseError& error) {
            // 2.2.7.4. If onRejected is not a function and promise1 is rejected,
            // promise2 must be rejected with the same reason as promise1
            reject(error);
        };
    }
};

template <typename T> class PromiseData;

template <typename T, typename F>
class PromiseDataBase : public QSharedData
{
public:
    using Handler = std::pair<QPointer<QThread>, std::function<F>>;
    using Catcher = std::pair<QPointer<QThread>, std::function<void(const PromiseError&)>>;

    virtual ~PromiseDataBase() {}

    bool isFulfilled() const
    {
        return !isPending() && m_error.isNull();
    }

    bool isRejected() const
    {
        return !isPending() && !m_error.isNull();
    }

    bool isPending() const
    {
        QReadLocker lock(&m_lock);
        return !m_settled;
    }

    void addHandler(std::function<F> handler)
    {
        QWriteLocker lock(&m_lock);
        m_handlers.append({QThread::currentThread(), std::move(handler)});
    }

    void addCatcher(std::function<void(const PromiseError&)> catcher)
    {
        QWriteLocker lock(&m_lock);
        m_catchers.append({QThread::currentThread(), std::move(catcher)});
    }

    template <typename E>
    void reject(E&& error)
    {
        Q_ASSERT(isPending());
        Q_ASSERT(m_error.isNull());
        m_error = PromiseError(std::forward<E>(error));
        setSettled();
    }

    const PromiseError& error() const
    {
        Q_ASSERT(isRejected());
        return m_error;
    }

    void dispatch()
    {
        if (isPending()) {
            return;
        }

        // A promise can't be resolved multiple times so once settled, its state can't
        // change. When fulfilled, handlers must be called (a single time) and catchers
        // ignored indefinitely (or vice-versa when the promise is rejected), so make
        // sure to clear both handlers AND catchers when dispatching. This also prevents
        // shared pointer circular reference memory leaks when the owning promise is
        // captured in the handler and/or catcher lambdas.

        m_lock.lockForWrite();
        QVector<Handler> handlers(std::move(m_handlers));
        QVector<Catcher> catchers(std::move(m_catchers));
        m_lock.unlock();

        if (m_error.isNull()) {
            notify(handlers);
            return;
        }

        PromiseError error(m_error);
        Q_ASSERT(!error.isNull());

        for (const auto& catcher: catchers) {
            const auto& fn = catcher.second;
            qtpromise_defer([=]() {
                fn(error);
            }, catcher.first);
        }
    }

protected:
    mutable QReadWriteLock m_lock;

    void setSettled()
    {
        QWriteLocker lock(&m_lock);
        Q_ASSERT(!m_settled);
        m_settled = true;
    }

    virtual void notify(const QVector<Handler>&) = 0;

private:
    bool m_settled = false;
    QVector<Handler> m_handlers;
    QVector<Catcher> m_catchers;
    PromiseError m_error;
};

template <typename T>
class PromiseData : public PromiseDataBase<T, void(const T&)>
{
    using Handler = typename PromiseDataBase<T, void(const T&)>::Handler;

public:
    template <typename V>
    void resolve(V&& value)
    {
        Q_ASSERT(this->isPending());
        Q_ASSERT(m_value.isNull());
        m_value = PromiseValue<T>(std::forward<V>(value));
        this->setSettled();
    }

    const PromiseValue<T>& value() const
    {
        Q_ASSERT(this->isFulfilled());
        return m_value;
    }

    void notify(const QVector<Handler>& handlers) Q_DECL_OVERRIDE
    {
        PromiseValue<T> value(m_value);
        Q_ASSERT(!value.isNull());

        for (const auto& handler: handlers) {
            const auto& fn = handler.second;
            qtpromise_defer([=]() {
                fn(value.data());
            }, handler.first);
        }
    }

private:
    PromiseValue<T> m_value;
};

template <>
class PromiseData<void> : public PromiseDataBase<void, void()>
{
    using Handler = PromiseDataBase<void, void()>::Handler;

public:
    void resolve()
    {
        setSettled();
    }

protected:
    void notify(const QVector<Handler>& handlers) Q_DECL_OVERRIDE
    {
        for (const auto& handler: handlers) {
            qtpromise_defer(handler.second, handler.first);
        }
    }
};

template <typename T>
class PromiseResolver
{
public:
    PromiseResolver(QtPromise::QPromise<T> promise)
        : m_d(new Data())
    {
        m_d->promise = new QtPromise::QPromise<T>(std::move(promise));
    }

    template <typename E>
    void reject(E&& error)
    {
        auto promise = m_d->promise;
        if (promise) {
            Q_ASSERT(promise->isPending());
            promise->m_d->reject(std::forward<E>(error));
            promise->m_d->dispatch();
            release();
        }
    }

    template <typename V>
    void resolve(V&& value)
    {
        auto promise = m_d->promise;
        if (promise) {
            Q_ASSERT(promise->isPending());
            promise->m_d->resolve(std::forward<V>(value));
            promise->m_d->dispatch();
            release();
        }
    }

    void resolve()
    {
        auto promise = m_d->promise;
        if (promise) {
            Q_ASSERT(promise->isPending());
            promise->m_d->resolve();
            promise->m_d->dispatch();
            release();
        }
    }

private:
    struct Data : public QSharedData
    {
        QtPromise::QPromise<T>* promise = nullptr;
    };

    QExplicitlySharedDataPointer<Data> m_d;

    void release()
    {
        Q_ASSERT(m_d->promise);
        Q_ASSERT(!m_d->promise->isPending());
        delete m_d->promise;
        m_d->promise = nullptr;
    }
};


template<int ...> struct sequence {};
template<int N, int ...S> struct generator : generator<N-1, N-1, S...> {};
template<int ...S> struct generator<0, S...>{ typedef sequence<S...> type; };

template<typename T>
inline QtPromise::QPromise<T> maybeResolve(const T&val)
{
    return QtPromise::QPromise<T>::resolve(val);
}

template<typename T>
inline QtPromise::QPromise<T> maybeResolve(const QtPromise::QPromise<T> &val)
{
    return val;
}

template<typename T, typename Tfunc, typename ...Args, int ...S>
inline QtPromise::QPromise<T>
spreadDispatcher(Tfunc func, const std::tuple<Args...>& params, sequence<S...>)
{
    auto res = func(std::get<S>(params) ...);
    return maybeResolve(res);
}

template<std::size_t I = 0, typename To, typename... Tp>
static inline typename std::enable_if<I == sizeof...(Tp), QtPromise::QPromise<To>>::type
tuplePromiseResolver(const std::tuple<Tp...>& tin, To& tout)
{
    Q_UNUSED(tin);
    return QtPromise::QPromise<To>::resolve(tout);
}

template<std::size_t I = 0, typename To, typename... Tp>
static inline typename std::enable_if<I < sizeof...(Tp), QtPromise::QPromise<To>>::type
tuplePromiseResolver(const std::tuple<Tp...>& tin, To& tout)
{
    auto p = std::get<I>(tin);
    auto o = std::get<I>(tout);
    return p.then([=](const decltype (o) &value) mutable {
        std::get<I>(tout) = value;
        return tuplePromiseResolver<I + 1, To, Tp...>(tin, tout);
    });
}


} // namespace QtPromise

#endif // ifndef QTPROMISE_QPROMISE_H
