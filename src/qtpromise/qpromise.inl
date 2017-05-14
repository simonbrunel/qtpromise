// Qt
#include <QCoreApplication>
#include <QSharedPointer>

namespace QtPromise {

template <typename T>
template <typename TFulfilled, typename TRejected>
inline auto QPromiseBase<T>::then(TFulfilled fulfilled, TRejected rejected)
    -> typename QPromiseHandler<T, TFulfilled>::Promise
{
    typename QPromiseHandler<T, TFulfilled>::Promise next;

    m_d->handlers << QPromiseHandler<T, TFulfilled>::create(next, fulfilled);
    m_d->catchers << QPromiseCatcher<T, TRejected>::create(next, rejected);

    if (m_d->resolved) {
        dispatch();
    }

    return next;
}

template <typename T>
template <typename TRejected>
inline auto QPromiseBase<T>::fail(TRejected rejected)
    -> typename QPromiseHandler<T, std::nullptr_t>::Promise
{
    return then(nullptr, rejected);
}

template <typename T>
template <typename TError>
inline QPromise<T> QPromiseBase<T>::reject(const TError& error)
{
    if (!m_d->resolved) {
        m_d->error = QtPromisePrivate::to_exception_ptr(error);
        m_d->rejected = true;
        m_d->resolved = true;
        dispatch();
    }

    return *this;
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
inline void QPromiseBase<T>::dispatch()
{
    Q_ASSERT(m_d->resolved);

    typename QPromiseData<T>::HandlerList handlers;
    typename QPromiseData<T>::CatcherList catchers;

    handlers.swap(m_d->handlers);
    catchers.swap(m_d->catchers);

    if (m_d->rejected) {
        const std::exception_ptr error = m_d->error;
        qtpromise_defer([catchers, error]() {
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
        return QPromise<void>().fulfill().then(handler).then([=](){
            return res;
        });
    }, [=]() {
        const auto exception = std::current_exception();
        return QPromise<void>().fulfill().then(handler).then([=](){
            std::rethrow_exception(exception);
            return T();
        });
    });
}

template <typename T>
inline QPromise<T> QPromise<T>::fulfill(const T& value)
{
    if (!this->m_d->resolved) {
        this->m_d->rejected = false;
        this->m_d->resolved = true;
        this->m_d->value = value;
        this->dispatch();
    }

    return *this;
}

template <typename T>
inline QPromise<QVector<T> > QPromise<T>::all(const QVector<QPromise<T> >& promises)
{
    QPromise<QVector<T> > next;

    const int count = promises.size();
    if (count == 0) {
        return next.fulfill({});
    }

    QSharedPointer<int> remaining(new int(count));
    QSharedPointer<QVector<T> > results(new QVector<T>(count));

    for (int i=0; i<count; ++i) {
        QPromise<T>(promises[i]).then([=](const T& res) mutable {
            if (next.isPending()) {
                (*results)[i] = res;
                if (--(*remaining) == 0) {
                    next.fulfill(*results);
                }
            }
        }, [=]() mutable {
            if (next.isPending()) {
                next.reject(std::current_exception());
            }
        });
    }

    return next;
}

template <typename T>
inline void QPromise<T>::notify(const typename QPromiseData<T>::HandlerList& handlers) const
{
    const T value = this->m_d->value;
    qtpromise_defer([handlers, value]() {
        for (auto handler: handlers) {
            handler(value);
        }
    });
}

template <typename THandler>
inline QPromise<void> QPromise<void>::finally(THandler handler)
{
    return this->then([=]() {
        return QPromise<void>().fulfill().then(handler).then([](){});
    }, [=]() {
        const auto exception = std::current_exception();
        return QPromise<void>().fulfill().then(handler).then([=](){
            std::rethrow_exception(exception);
        });
    });
}

inline QPromise<void> QPromise<void>::fulfill()
{
    if (!m_d->resolved) {
        m_d->rejected = false;
        m_d->resolved = true;
        dispatch();
    }

    return *this;
}

inline QPromise<void> QPromise<void>::all(const QVector<QPromise<void> >& promises)
{
    QPromise<void> next;

    QSharedPointer<int> remaining(new int(promises.size()));

    for (const auto& promise: promises) {
        QPromise<void>(promise).then([=]() mutable {
            if (next.isPending()) {
                if (--(*remaining) == 0) {
                    next.fulfill();
                }
            }
        }, [=]() mutable {
            if (next.isPending()) {
                next.reject(std::current_exception());
            }
        });
    }

    return next;
}

inline void QPromise<void>::notify(const typename QPromiseData<void>::HandlerList& handlers) const
{
    qtpromise_defer([handlers]() {
        for (const auto& handler: handlers) {
            handler();
        }
    });
}

// Helpers

template <typename T>
QPromise<T> qPromise(const T& value)
{
    return QPromise<T>().fulfill(value);
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
