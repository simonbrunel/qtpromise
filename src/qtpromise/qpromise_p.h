#ifndef QTPROMISE_QPROMISE_P_H
#define QTPROMISE_QPROMISE_P_H

// QtPromise
#include "qpromiseerror.h"
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
static void qtpromise_defer(F&& f, QThread* thread = nullptr)
{
    struct Event : public QEvent
    {
        using FType = typename std::decay<F>::type;
        Event(FType&& f) : QEvent(QEvent::None), m_f(std::move(f)) { }
        Event(const FType& f) : QEvent(QEvent::None), m_f(f) { }
        ~Event() { m_f(); }
        FType m_f;
    };

    QObject* target = QAbstractEventDispatcher::instance(thread);
    Q_ASSERT_X(target, "postMetaCall", "Target thread must have an event loop");
    QCoreApplication::postEvent(target, new Event(std::forward<F>(f)));
}

template <typename T>
struct PromiseDeduce
{
    using Type = QtPromise::QPromise<Unqualified<T> >;
};

template <typename T>
struct PromiseDeduce<QtPromise::QPromise<T> >
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
struct PromiseFulfill<QtPromise::QPromise<T> >
{
    static void call(
        const QtPromise::QPromise<T>& promise,
        const QtPromise::QPromiseResolve<T>& resolve,
        const QtPromise::QPromiseReject<T>& reject)
    {
        promise.then(
            [=](const T& value) {
                resolve(value);
            },
            [=]() { // catch all
                reject(std::current_exception());
            });
    }
};

template <>
struct PromiseFulfill<QtPromise::QPromise<void> >
{
    template <typename TPromise, typename TResolve, typename TReject>
    static void call(
        const TPromise& promise,
        const TResolve& resolve,
        const TReject& reject)
    {
        promise.then(
            [=]() {
                resolve();
            },
            [=]() { // catch all
                reject(std::current_exception());
            });
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
    static std::function<void(const QtPromise::QPromiseError&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const QtPromise::QPromiseError& error) {
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
    static std::function<void(const QtPromise::QPromiseError&)> create(
        const THandler& handler,
        const TResolve& resolve,
        const TReject& reject)
    {
        return [=](const QtPromise::QPromiseError& error) {
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
    static std::function<void(const QtPromise::QPromiseError&)> create(
        std::nullptr_t,
        const TResolve&,
        const TReject& reject)
    {
        return [=](const QtPromise::QPromiseError& error) {
            // 2.2.7.4. If onRejected is not a function and promise1 is rejected,
            // promise2 must be rejected with the same reason as promise1
            reject(error);
        };
    }
};

template <typename T> class PromiseData;

template <typename T>
class PromiseDataBase : public QSharedData
{
public:
    using Error = QtPromise::QPromiseError;
    using Catcher = std::pair<QPointer<QThread>, std::function<void(const Error&)> >;

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

    void addCatcher(std::function<void(const Error&)> catcher)
    {
        QWriteLocker lock(&m_lock);
        m_catchers.append({QThread::currentThread(), std::move(catcher)});
    }

    void reject(Error error)
    {
        Q_ASSERT(isPending());
        Q_ASSERT(m_error.isNull());
        m_error.reset(new Error(std::move(error)));
        setSettled();
    }

    void dispatch()
    {
        if (isPending()) {
            return;
        }

        if (m_error.isNull()) {
            notify();
            return;
        }

        m_lock.lockForWrite();
        QVector<Catcher> catchers(std::move(m_catchers));
        m_lock.unlock();

        QSharedPointer<Error> error = m_error;
        Q_ASSERT(!error.isNull());

        for (const auto& catcher: catchers) {
            const auto& fn = catcher.second;
            qtpromise_defer([=]() {
                fn(*error);
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

    virtual void notify() = 0;

private:
    bool m_settled = false;
    QVector<Catcher> m_catchers;
    QSharedPointer<Error> m_error;
};

template <typename T>
class PromiseData : public PromiseDataBase<T>
{
    using Handler = std::pair<QPointer<QThread>, std::function<void(const T&)> >;

public:
    void addHandler(std::function<void(const T&)> handler)
    {
        QWriteLocker lock(&this->m_lock);
        m_handlers.append({QThread::currentThread(), std::move(handler)});
    }

    void resolve(T&& value)
    {
        Q_ASSERT(m_value.isNull());
        m_value.reset(new T(std::move(value)));
        this->setSettled();
    }

    void resolve(const T& value)
    {
        Q_ASSERT(m_value.isNull());
        m_value.reset(new T(value));
        this->setSettled();
    }

    void notify() Q_DECL_OVERRIDE
    {
        this->m_lock.lockForWrite();
        QVector<Handler> handlers(std::move(m_handlers));
        this->m_lock.unlock();

        QSharedPointer<T> value(m_value);
        Q_ASSERT(!value.isNull());

        for (const auto& handler: handlers) {
            const auto& fn = handler.second;
            qtpromise_defer([=]() {
                fn(*value);
            }, handler.first);
        }
    }

private:
    QVector<Handler> m_handlers;
    QSharedPointer<T> m_value;
};

template <>
class PromiseData<void> : public PromiseDataBase<void>
{
    using Handler = std::pair<QPointer<QThread>, std::function<void()> >;

public:
    void addHandler(std::function<void()> handler)
    {
        QWriteLocker lock(&m_lock);
        m_handlers.append({QThread::currentThread(), std::move(handler)});
    }

    void resolve() { setSettled(); }

protected:
    void notify() Q_DECL_OVERRIDE
    {
        this->m_lock.lockForWrite();
        QVector<Handler> handlers(std::move(m_handlers));
        this->m_lock.unlock();

        for (const auto& handler: handlers) {
            qtpromise_defer(handler.second, handler.first);
        }
    }

private:
    QVector<Handler> m_handlers;
};

} // namespace QtPromise

#endif // ifndef QTPROMISE_QPROMISE_H
