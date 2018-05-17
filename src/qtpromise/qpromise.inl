// Qt
#include <QCoreApplication>
#include <QSharedPointer>
#include <QTimer>

namespace QtPromise {

template <class T>
class QPromiseResolve
{
public:
    QPromiseResolve(QtPromisePrivate::PromiseResolver<T> resolver)
        : m_resolver(std::move(resolver))
    { }

    template <typename V>
    void operator()(V&& value) const
    {
        m_resolver.resolve(std::forward<V>(value));
    }

    void operator()() const
    {
        m_resolver.resolve();
    }

private:
    mutable QtPromisePrivate::PromiseResolver<T> m_resolver;
};

template <class T>
class QPromiseReject
{
public:
    QPromiseReject(QtPromisePrivate::PromiseResolver<T> resolver)
        : m_resolver(std::move(resolver))
    { }

    template <typename E>
    void operator()(E&& error) const
    {
        m_resolver.reject(std::forward<E>(error));
    }

private:
    mutable QtPromisePrivate::PromiseResolver<T> m_resolver;
};

template <typename T>
template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count == 1, int>::type>
inline QPromiseBase<T>::QPromiseBase(F callback)
    : m_d(new QtPromisePrivate::PromiseData<T>())
{
    QtPromisePrivate::PromiseResolver<T> resolver(*this);

    try {
        callback(QPromiseResolve<T>(resolver));
    } catch (...) {
        resolver.reject(std::current_exception());
    }
}

template <typename T>
template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count != 1, int>::type>
inline QPromiseBase<T>::QPromiseBase(F callback)
    : m_d(new QtPromisePrivate::PromiseData<T>())
{
    QtPromisePrivate::PromiseResolver<T> resolver(*this);

    try {
        callback(QPromiseResolve<T>(resolver), QPromiseReject<T>(resolver));
    } catch (...) {
        resolver.reject(std::current_exception());
    }
}

template <typename T>
template <typename TFulfilled, typename TRejected>
inline typename QtPromisePrivate::PromiseHandler<T, TFulfilled>::Promise
QPromiseBase<T>::then(const TFulfilled& fulfilled, const TRejected& rejected) const
{
    using namespace QtPromisePrivate;
    using PromiseType = typename PromiseHandler<T, TFulfilled>::Promise;

    PromiseType next([&](
        const QPromiseResolve<typename PromiseType::Type>& resolve,
        const QPromiseReject<typename PromiseType::Type>& reject) {
        m_d->addHandler(PromiseHandler<T, TFulfilled>::create(fulfilled, resolve, reject));
        m_d->addCatcher(PromiseCatcher<T, TRejected>::create(rejected, resolve, reject));
    });

    if (!m_d->isPending()) {
        m_d->dispatch();
    }

    return next;
}

template <typename T>
template <typename TFulfilled>
inline typename QtPromisePrivate::PromiseHandler<T, TFulfilled>::Promise
QPromiseBase<T>::then(TFulfilled&& fulfilled) const
{
    return then(std::forward<TFulfilled>(fulfilled), nullptr);
}

template <typename T>
template <typename TRejected>
inline typename QtPromisePrivate::PromiseHandler<T, std::nullptr_t>::Promise
QPromiseBase<T>::fail(TRejected&& rejected) const
{
    return then(nullptr, std::forward<TRejected>(rejected));
}

template <typename T>
template <typename THandler>
inline QPromise<T> QPromiseBase<T>::finally(THandler handler) const
{
    QPromise<T> p = *this;
    return p.then(handler, handler).then([=]() {
        return p;
    });
}

template <typename T>
template <typename THandler>
inline QPromise<T> QPromiseBase<T>::tap(THandler handler) const
{
    QPromise<T> p = *this;
    return p.then(handler).then([=]() {
        return p;
    });
}

template <typename T>
template <typename THandler>
inline QPromise<T> QPromiseBase<T>::tapFail(THandler handler) const
{
    QPromise<T> p = *this;
    return p.then([](){}, handler).then([=]() {
        return p;
    });
}

template <typename T>
template <typename E>
inline QPromise<T> QPromiseBase<T>::timeout(int msec, E&& error) const
{
    QPromise<T> p = *this;
    return QPromise<T>([&](
        const QPromiseResolve<T>& resolve,
        const QPromiseReject<T>& reject) {

        QTimer::singleShot(msec, [=]() {
            // we don't need to verify the current promise state, reject()
            // takes care of checking if the promise is already resolved,
            // and thus will ignore this rejection.
            reject(std::move(error));
        });

        QtPromisePrivate::PromiseFulfill<QPromise<T>>::call(p, resolve, reject);
    });
}

template <typename T>
inline QPromise<T> QPromiseBase<T>::delay(int msec) const
{
    return tap([=]() {
        return QPromise<void>([&](const QPromiseResolve<void>& resolve) {
            QTimer::singleShot(msec, resolve);
        });
    });
}

template <typename T>
inline QPromise<T> QPromiseBase<T>::wait() const
{
    // @TODO wait timeout + global timeout
    while (m_d->isPending()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    return *this;
}

template <typename T>
template <typename E>
inline QPromise<T> QPromiseBase<T>::reject(E&& error)
{
    return QPromise<T>([&](const QPromiseResolve<T>&, const QPromiseReject<T>& reject) {
        reject(std::forward<E>(error));
    });
}

template <typename T>
template <typename MapFunctor, typename Mapper>
inline QPromise<typename Mapper::ResultType>
QPromise<T>::map(MapFunctor fn) const
{
    return this->then([=](const T& values) {
        return QPromise<T>::map(values, fn);
    });
}

template <typename T>
template <typename EachFunctor>
inline QPromise<T>
QPromise<T>::each(EachFunctor fn) const
{
    return this->then([=](const T& values) {
        return QPromise<T>::each(values, fn);
    });
}

template <typename T>
template <typename FilterFunctor>
inline QPromise<T>
QPromise<T>::filter(FilterFunctor fn) const
{
    return this->then([=](const T& values) {
        return QPromise<T>::filter(values, fn);
    });
}

template <typename T>
template <typename ReduceFunctor, typename R>
inline QPromise<R>
QPromise<T>::reduce(ReduceFunctor fn, const R &initial) const
{
    return this->then([=](const T& values) {
        return QPromise<T>::reduce(values, fn, initial);
    });
}


template <typename T>
template <template <typename, typename...> class Sequence, typename ...Args>
inline QPromise<QVector<T>> QPromise<T>::all(const Sequence<QPromise<T>, Args...>& promises)
{
    const int count = static_cast<int>(promises.size());
    if (count == 0) {
        return QPromise<QVector<T>>::resolve({});
    }

    return QPromise<QVector<T>>([=](
        const QPromiseResolve<QVector<T>>& resolve,
        const QPromiseReject<QVector<T>>& reject) {

        QSharedPointer<int> remaining(new int(count));
        QSharedPointer<QVector<T>> results(new QVector<T>(count));

        int i = 0;
        for (const auto& promise: promises) {
            promise.then([=](const T& res) mutable {
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

            i++;
        }
    });
}

template <typename T>
template <typename MapFunctor, typename Mapper>
inline QPromise<typename Mapper::ResultType>
QPromise<T>::map(const T& values, MapFunctor fn)
{
    using namespace QtPromisePrivate;
    using RetType = typename Mapper::ReturnType;
    using ResType = typename Mapper::ResultType::value_type;

    int i = 0;

    std::vector<QPromise<ResType>> mapped;
    for (const auto& v : values) {
        mapped.push_back(QPromise<ResType>([&](
            const QPromiseResolve<ResType>& resolve,
            const QPromiseReject<ResType>& reject) {
                PromiseFulfill<RetType>::call(fn(v, i), resolve, reject);
            }));

        i++;
    }

    return QPromise<ResType>::all(mapped);
}

template <typename T>
template <typename EachFunctor>
inline QPromise<T>
QPromise<T>::each(const T& values, EachFunctor fn)
{
    using namespace QtPromisePrivate;
    using ResType = typename T::value_type;
    using ReturnType = typename std::result_of<EachFunctor(ResType, int)>::type;
    using ResultType = typename PromiseDeduce<ReturnType>::Type::Type;

    int i = 0;

    std::vector<QPromise<void>> promises;
    for (const ResType& v : values) {
        promises.push_back(QPromise<void>([&](
            const QPromiseResolve<void>& resolve) {
                fn(v,i);
                resolve();
            }));

        i++;
    }

    return QPromise<void>::all(promises)
            .then([values](){
                return values;
            });
}

template <typename T>
template <typename FilterFunctor>
inline QPromise<T>
QPromise<T>::filter(const T& values, FilterFunctor fn)
{
    using namespace QtPromisePrivate;
    using ResType = typename T::value_type;
    using ReturnType = typename std::result_of<FilterFunctor(ResType, int)>::type;
    using ResultType = typename PromiseDeduce<ReturnType>::Type::Type;

    int i = 0;

    std::vector<QPromise<ResultType>> promises;
    for (const auto& v : values) {
        promises.push_back(QPromise<ResultType>([&](
            const QPromiseResolve<ResultType>& resolve,
            const QPromiseReject<ResultType>& reject) {
                PromiseFulfill<ReturnType>::call(fn(v, i), resolve, reject);
            }));

        i++;
    }

    return QPromise<ResultType>::all(promises)
            .then([values](const QVector<ResultType> &filtered){
                T results;
                int index = 0;
                for(bool filter : filtered) {
                    if (filter) {
                        results.push_back(values[index]);
                    }
                    index++;
                }
                return results;
            });
}

template <typename T>
template <typename ReduceFunctor, typename R>
inline QPromise<R>
QPromise<T>::reduce(const T& values, ReduceFunctor fn, const R &initial)
{
    using ResType = typename T::value_type;
    int index = 0;
    QPromise<R> promise = QPromise<R>::resolve(initial);
    for( ResType value : values ) {
        promise = promise.then([=](const R& acc){
            return fn(value,acc,index);
        });
        index++;
    }
    return promise;
}

template <typename T>
inline QPromise<T> QPromise<T>::resolve(const T& value)
{
    return QPromise<T>([&](const QPromiseResolve<T>& resolve) {
       resolve(value);
    });
}

template <typename T>
inline QPromise<T> QPromise<T>::resolve(T&& value)
{
    return QPromise<T>([&](const QPromiseResolve<T>& resolve) {
       resolve(std::forward<T>(value));
    });
}

template <template <typename, typename...> class Sequence, typename ...Args>
inline QPromise<void> QPromise<void>::all(const Sequence<QPromise<void>, Args...>& promises)
{
    const int count = static_cast<int>(promises.size());
    if (count == 0) {
        return QPromise<void>::resolve();
    }

    return QPromise<void>([=](
        const QPromiseResolve<void>& resolve,
        const QPromiseReject<void>& reject) {

        QSharedPointer<int> remaining(new int(count));

        for (const auto& promise: promises) {
            promise.then([=]() {
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
} // namespace QtPromise
