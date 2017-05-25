#ifndef _QTPROMISE_QPROMISEFUTURE_P_H
#define _QTPROMISE_QPROMISEFUTURE_P_H

#include <QFutureWatcher>
#include <QFuture>

namespace QtPromisePrivate {

template <typename T>
struct PromiseDeduce<QFuture<T> >
    : public PromiseDeduce<T>
{ };

template <typename T>
struct PromiseFulfill<QFuture<T> >
{
    static void call(
        const QFuture<T>& future,
        const QtPromise::QPromiseResolve<T>& resolve,
        const QtPromise::QPromiseReject<T>& reject)
    {
        using Watcher = QFutureWatcher<T>;

        Watcher* watcher = new Watcher();
        QObject::connect(watcher, &Watcher::finished, [=]() mutable {
            try {
                T res = watcher->result();
                PromiseFulfill<T>::call(res, resolve, reject);
            } catch(...) {
                reject(std::current_exception());
            }

            watcher->deleteLater();
        });

        watcher->setFuture(future);
    }
};

template <>
struct PromiseFulfill<QFuture<void> >
{
    static void call(
        const QFuture<void>& future,
        const QtPromise::QPromiseResolve<void>& resolve,
        const QtPromise::QPromiseReject<void>& reject)
    {
        using Watcher = QFutureWatcher<void>;

        Watcher* watcher = new Watcher();
        QObject::connect(watcher, &Watcher::finished, [=]() mutable {
            try {
                // let's rethrown possibe exception
                watcher->waitForFinished();
                resolve();
            } catch(...) {
                reject(std::current_exception());
            }

            watcher->deleteLater();
        });

        watcher->setFuture(future);
    }
};

} // namespace QtPromise

#endif // _QTPROMISE_QPROMISEFUTURE_P_H
