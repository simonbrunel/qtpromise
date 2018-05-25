#ifndef QTPROMISE_QPROMISEHELPERS_H
#define QTPROMISE_QPROMISEHELPERS_H

// QtPromise
#include "qpromise_p.h"

namespace QtPromise {

template <typename T>
static inline typename QtPromisePrivate::PromiseDeduce<T>::Type qPromise(T&& value)
{
    using namespace QtPromisePrivate;
    using Promise = typename PromiseDeduce<T>::Type;
    return Promise([&](
        const QPromiseResolve<typename Promise::Type>& resolve,
        const QPromiseReject<typename Promise::Type>& reject) {
        PromiseFulfill<T>::call(std::forward<T>(value), resolve, reject);
    });
}

static inline QPromise<void> qPromise()
{
    return QPromise<void>([](
        const QPromiseResolve<void>& resolve) {
        resolve();
    });
}

template <typename T, template <typename, typename...> class Sequence = QVector, typename ...Args>
static inline QPromise<QVector<T>> qPromiseAll(const Sequence<QPromise<T>, Args...>& promises)
{
    return QPromise<T>::all(promises);
}

template <template <typename, typename...> class Sequence = QVector, typename ...Args>
static inline QPromise<void> qPromiseAll(const Sequence<QPromise<void>, Args...>& promises)
{
    return QPromise<void>::all(promises);
}

template <typename Functor, typename... Args>
static inline typename QtPromisePrivate::PromiseFunctor<Functor, Args...>::PromiseType
attempt(Functor&& fn, Args&&... args)
{
    using namespace QtPromisePrivate;
    using FunctorType = PromiseFunctor<Functor, Args...>;
    using PromiseType = typename FunctorType::PromiseType;
    using ValueType = typename PromiseType::Type;

    // NOTE: std::forward<T<U>>: MSVC 2013 fails when forwarding
    // template type (error: "expects 4 arguments - 0 provided").
    // However it succeeds with type alias.
    // TODO: should we expose QPromise::ResolveType & RejectType?
    using ResolveType = QPromiseResolve<ValueType>;
    using RejectType = QPromiseReject<ValueType>;

    return PromiseType(
        [&](ResolveType&& resolve, RejectType&& reject) {
            PromiseDispatch<typename FunctorType::ResultType>::call(
                std::forward<ResolveType>(resolve),
                std::forward<RejectType>(reject),
                std::forward<Functor>(fn),
                std::forward<Args>(args)...);
        });
}

template <typename Sequence, typename Functor>
static inline QPromise<Sequence> each(const Sequence& values, Functor&& fn)
{
    return QPromise<Sequence>::resolve(values).each(std::forward<Functor>(fn));
}

template <typename Sequence, typename Functor>
static inline typename QtPromisePrivate::PromiseMapper<Sequence, Functor>::PromiseType
map(const Sequence& values, Functor fn)
{
    using namespace QtPromisePrivate;
    using MapperType = PromiseMapper<Sequence, Functor>;
    using ResType = typename MapperType::ResultType::value_type;
    using RetType = typename MapperType::ReturnType;

    int i = 0;

    std::vector<QPromise<ResType>> promises;
    for (const auto& v : values) {
        promises.push_back(QPromise<ResType>([&](
            const QPromiseResolve<ResType>& resolve,
            const QPromiseReject<ResType>& reject) {
                PromiseFulfill<RetType>::call(fn(v, i), resolve, reject);
            }));

        i++;
    }

    return QPromise<ResType>::all(promises);
}

template <typename Sequence, typename Functor>
static inline QPromise<Sequence> filter(const Sequence& values, Functor fn)
{
    return QtPromise::map(values, fn)
        .then([=](const QVector<bool>& filters) {
            Sequence filtered;

            auto filter = filters.begin();
            for (auto& value : values) {
                if (*filter) {
                    filtered.push_back(std::move(value));
                }

                filter++;
            }

            return filtered;
        });
}

} // namespace QtPromise

#endif // QTPROMISE_QPROMISEHELPERS_H
