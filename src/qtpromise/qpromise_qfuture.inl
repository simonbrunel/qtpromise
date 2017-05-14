#include <QFutureWatcher>
#include <QFuture>

namespace QtPromisePrivate {

template <typename T>
struct QPromiseDeduce<QFuture<T> >
    : public QPromiseDeduce<T>
{ };

template <typename T>
struct QPromiseFulfill<QFuture<T>, QPromise<T> >
{
    static void call(QPromise<T> next, const QFuture<T>& future)
    {
        using Watcher = QFutureWatcher<T>;

        Watcher* watcher = new Watcher();
        QObject::connect(
            watcher, &Watcher::finished,
            [next, watcher]() mutable {
                T res;
                try {
                    res = watcher->result();
                } catch(...) {
                    next.reject(std::current_exception());
                }

                watcher->deleteLater();
                if (next.isPending()) {
                    QPromiseFulfill<T>::call(next, res);
                }
            });

        watcher->setFuture(future);
    }
};

template <>
struct QPromiseFulfill<QFuture<void>, QPromise<void> >
{
    static void call(QPromise<void> next, const QFuture<void>& future)
    {
        using Watcher = QFutureWatcher<void>;

        Watcher* watcher = new Watcher();
        QObject::connect(
            watcher, &Watcher::finished,
            [next, watcher]() mutable {
                try {
                    // let's rethrown possibe exception
                    watcher->waitForFinished();
                } catch(...) {
                    next.reject(std::current_exception());
                }

                watcher->deleteLater();
                if (next.isPending()) {
                    next.fulfill();
                }
            });

        watcher->setFuture(future);
    }
};

} // namespace QtPromisePrivate

namespace QtPromise {

template <typename T>
QPromise<T> qPromise(const QFuture<T>& future)
{
    QPromise<T> next;
    QPromiseFulfill<QFuture<T> >::call(next, future);
    return next;
}

} // namespace QtPromise
