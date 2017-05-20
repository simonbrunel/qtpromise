// Qt
#include <QCoreApplication>
#include <QSharedPointer>

namespace QtPromise {

template <class T = void>
class QPromiseResolve
{
public:
    QPromiseResolve(const QPromise<T>& p)
        : m_promise(p)
    { }

    void operator()(const T& value) const
    {
        auto p = m_promise;
        if (p.isPending()) {
            p.m_d->rejected = false;
            p.m_d->resolved = true;
            p.m_d->value = value;
            p.dispatch();
        }
    }

private:
    QPromise<T> m_promise;
};

template <>
class QPromiseResolve<void>
{
public:
    QPromiseResolve(const QPromise<void>& p)
        : m_promise(p)
    { }

    void operator()() const
    {
        auto p = m_promise;
        if (p.isPending()) {
            p.m_d->rejected = false;
            p.m_d->resolved = true;
            p.dispatch();
        }
    }

private:
    QPromise<void> m_promise;
};

template <class T = void>
class QPromiseReject
{
public:
    QPromiseReject()
    { }

    QPromiseReject(const QPromise<T>& p)
        : m_promise(p)
    { }

    void operator()(const QPromiseError& error) const
    {
        auto p = m_promise;
        if (p.isPending()) {
            p.m_d->rejected = true;
            p.m_d->resolved = true;
            p.m_d->error = error;
            p.dispatch();
        }
    }

private:
    QPromise<T> m_promise;
};

template <typename T>
template <typename TFulfilled, typename TRejected>
inline typename QtPromisePrivate::PromiseHandler<T, TFulfilled>::Promise
QPromiseBase<T>::then(TFulfilled fulfilled, TRejected rejected)
{
    using namespace QtPromisePrivate;
    using PromiseType = typename PromiseHandler<T, TFulfilled>::Promise;

    PromiseType next([=](
        const QPromiseResolve<typename PromiseType::Type>& resolve,
        const QPromiseReject<typename PromiseType::Type>& reject) {
        m_d->handlers << PromiseHandler<T, TFulfilled>::create(fulfilled, resolve, reject);
        m_d->catchers << PromiseCatcher<T, TRejected>::create(rejected, resolve, reject);
    });

    if (m_d->resolved) {
        dispatch();
    }

    return next;
}

template <typename T>
template <typename TRejected>
inline typename QtPromisePrivate::PromiseHandler<T, std::nullptr_t>::Promise
QPromiseBase<T>::fail(TRejected rejected)
{
    return then(nullptr, rejected);
}

template <typename T>
inline QPromise<T> QPromiseBase<T>::wait() const
{
    // @TODO wait timeout + global timeout
    while (!m_d->resolved) {
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    return *this;
}

template <typename T>
template <typename E>
inline QPromise<T> QPromiseBase<T>::reject(const E& error)
{
    return QPromise<T>([=](const QPromiseResolve<T>&) {
        throw error;
    });
}

template <typename T>
inline QPromiseBase<T>::QPromiseBase()
    : m_d(new QtPromisePrivate::PromiseData<T>())
{
}

template <typename T>
template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count == 1, int>::type>
inline QPromiseBase<T>::QPromiseBase(F resolver)
    : m_d(new QtPromisePrivate::PromiseData<T>())
{
    auto resolve = QPromiseResolve<T>(*this);
    auto reject = QPromiseReject<T>(*this);

    try {
        resolver(resolve);
    } catch(...) {
        reject(std::current_exception());
    }
}

template <typename T>
template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count != 1, int>::type>
inline QPromiseBase<T>::QPromiseBase(F resolver)
    : m_d(new QtPromisePrivate::PromiseData<T>())
{
    auto resolve = QPromiseResolve<T>(*this);
    auto reject = QPromiseReject<T>(*this);

    try {
        resolver(resolve, reject);
    } catch(...) {
        reject(std::current_exception());
    }
}

template <typename T>
inline void QPromiseBase<T>::dispatch()
{
    using namespace QtPromisePrivate;

    Q_ASSERT(m_d->resolved);

    typename PromiseData<T>::HandlerList handlers;
    typename PromiseData<T>::CatcherList catchers;

    handlers.swap(m_d->handlers);
    catchers.swap(m_d->catchers);

    if (m_d->rejected) {
        const QPromiseError error = m_d->error;
        qtpromise_defer([=]() {
            for (auto catcher: catchers) {
                catcher(error);
            }
        });
    } else {
        notify(handlers);
    }
}

template <typename T>
template <typename THandler>
inline QPromise<T> QPromise<T>::finally(THandler handler)
{
    return this->then([=](const T& res) {
        return QPromise<void>::resolve().then(handler).then([=](){
            return res;
        });
    }, [=]() {
        const auto exception = std::current_exception();
        return QPromise<void>::resolve().then(handler).then([=](){
            std::rethrow_exception(exception);
            return T();
        });
    });
}

template <typename T>
inline QPromise<QVector<T> > QPromise<T>::all(const QVector<QPromise<T> >& promises)
{
    const int count = promises.size();
    if (count == 0) {
        return QPromise<QVector<T> >::resolve({});
    }

    return QPromise<QVector<T> >([=](
        const QPromiseResolve<QVector<T> >& resolve,
        const QPromiseReject<QVector<T> >& reject) {

        QSharedPointer<int> remaining(new int(count));
        QSharedPointer<QVector<T> > results(new QVector<T>(count));

        for (int i=0; i<count; ++i) {
            QPromise<T>(promises[i]).then([=](const T& res) mutable {
                (*results)[i] = res;
                if (--(*remaining) == 0) {
                    resolve(*results);
                }
            }, [=]() mutable {
                if (*remaining != -1) {
                    *remaining = -1;
                    reject(std::current_exception());
                }
            });
        }
    });
}

template <typename T>
inline QPromise<T> QPromise<T>::resolve(const T& value)
{
    return QPromise<T>([=](const QPromiseResolve<T>& resolve) {
        resolve(value);
    });
}

template <typename T>
inline void QPromise<T>::notify(const typename QtPromisePrivate::PromiseData<T>::HandlerList& handlers) const
{
    const T value = this->m_d->value;
    QtPromisePrivate::qtpromise_defer([handlers, value]() {
        for (auto handler: handlers) {
            handler(value);
        }
    });
}

template <typename THandler>
inline QPromise<void> QPromise<void>::finally(THandler handler)
{
    return this->then([=]() {
        return QPromise<void>::resolve().then(handler).then([](){});
    }, [=]() {
        const auto exception = std::current_exception();
        return QPromise<void>::resolve().then(handler).then([=](){
            std::rethrow_exception(exception);
        });
    });
}

inline QPromise<void> QPromise<void>::all(const QVector<QPromise<void> >& promises)
{
    const int count = promises.size();
    if (count == 0) {
        return QPromise<void>::resolve();
    }

    return QPromise<void>([=](
        const QPromiseResolve<void>& resolve,
        const QPromiseReject<void>& reject) {

        QSharedPointer<int> remaining(new int(promises.size()));

        for (const auto& promise: promises) {
            QPromise<void>(promise).then([=]() {
                if (--(*remaining) == 0) {
                    resolve();
                }
            }, [=]() {
                if (*remaining != -1) {
                    *remaining = -1;
                    reject(std::current_exception());
                }
            });
        }
    });
}

inline QPromise<void> QPromise<void>::resolve()
{
    return QPromise<void>([](const QPromiseResolve<void>& resolve) {
        resolve();
    });
}

inline void QPromise<void>::notify(const typename QtPromisePrivate::PromiseData<void>::HandlerList& handlers) const
{
    QtPromisePrivate::qtpromise_defer([handlers]() {
        for (const auto& handler: handlers) {
            handler();
        }
    });
}

// Helpers

template <typename T>
typename QtPromisePrivate::PromiseDeduce<T>::Type qPromise(const T& value)
{
    using namespace QtPromisePrivate;
    using Promise = typename PromiseDeduce<T>::Type;
    return Promise([=](
        const QPromiseResolve<typename Promise::Type>& resolve,
        const QPromiseReject<typename Promise::Type>& reject) {
        PromiseFulfill<T>::call(value, resolve, reject);
    });
}

QPromise<void> qPromise()
{
    return QPromise<void>([](
        const QPromiseResolve<void>& resolve) {
        resolve();
    });
}

template <typename T>
QPromise<QVector<T> > qPromiseAll(const QVector<QPromise<T> >& promises)
{
    return QPromise<T>::all(promises);
}

QPromise<void> qPromiseAll(const QVector<QPromise<void> >& promises)
{
    return QPromise<void>::all(promises);
}

} // namespace QtPromise
