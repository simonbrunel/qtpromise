#ifndef _QTPROMISE_QPROMISE_P_H
#define _QTPROMISE_QPROMISE_P_H

// QtPromise
#include "qpromiseerror.h"
#include "qpromiseglobal.h"

// Qt
#include <QTimer>
#include <QSharedData>
#include <QVector>

namespace QtPromise {

template <typename T>
class QPromise;

template <typename T>
class QPromiseResolve;

template <typename T>
class QPromiseReject;

} // namespace QtPromise

namespace QtPromisePrivate {

template <typename F>
inline void qtpromise_defer(F f)
{
    QTimer::singleShot(0, f);
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
        const T& value,
        const QtPromise::QPromiseResolve<T>& resolve,
        const QtPromise::QPromiseReject<T>&)
    {
        resolve(value);
    }
};

template <typename T>
struct PromiseFulfill<QtPromise::QPromise<T> >
{
    static void call(
        QtPromise::QPromise<T> promise,
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
    static void call(TPromise promise, TResolve resolve, TReject reject)
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
    static void call(const T& value, THandler handler, TResolve resolve, TReject reject)
    {
        try {
            const ResType res = handler(value);
            PromiseFulfill<ResType>::call(res, resolve, reject);
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
    static void call(const T& value, THandler handler, TResolve resolve, TReject reject)
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
    static void call(THandler handler, TResolve resolve, TReject reject)
    {
        try {
            const ResType res = handler();
            PromiseFulfill<ResType>::call(res, resolve, reject);
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
    static void call(THandler handler, TResolve resolve, TReject reject)
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
    static std::function<void(T)> create(THandler handler, TResolve resolve, TReject reject)
    {
        return [=](const T& value) {
            PromiseDispatch<T, ResType>::call(value, handler, resolve, reject);
        };
    }
};

template <typename T, typename THandler>
struct PromiseHandler<T, THandler, void>
{
    using ResType = typename std::result_of<THandler()>::type;
    using Promise = typename PromiseDispatch<T, ResType>::Promise;

    template <typename TResolve, typename TReject>
    static std::function<void(T)> create(THandler handler, TResolve resolve, TReject reject)
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
    static std::function<void()> create(THandler handler, TResolve resolve, TReject reject)
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
    static std::function<void(T)> create(std::nullptr_t, TResolve resolve, TReject reject)
    {
        return [=](const T& value) {
            // 2.2.7.3. If onFulfilled is not a function and promise1 is fulfilled,
            // promise2 must be fulfilled with the same value as promise1.
            PromiseFulfill<T>::call(value, resolve, reject);
        };
    }
};

template <>
struct PromiseHandler<void, std::nullptr_t, void>
{
    using Promise = QtPromise::QPromise<void>;

    template <typename TResolve, typename TReject>
    static std::function<void()> create(std::nullptr_t, TResolve resolve, TReject)
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
    using Functor = std::function<void(QtPromise::QPromiseError)>;
    using ResType = typename std::result_of<THandler(TArg)>::type;

    template <typename TResolve, typename TReject>
    static Functor create(THandler handler, TResolve resolve, TReject reject)
    {
        return [=](const QtPromise::QPromiseError& error) {
            try {
                error.rethrow();
            } catch (const TArg& error) {
                PromiseDispatch<TArg, ResType>::call(error, handler, resolve, reject);
            } catch(...) {
                reject(std::current_exception());
            }
        };
    }
};

template <typename T, typename THandler>
struct PromiseCatcher<T, THandler, void>
{
    using Functor = std::function<void(QtPromise::QPromiseError)>;
    using ResType = typename std::result_of<THandler()>::type;

    template <typename TResolve, typename TReject>
    static Functor create(THandler handler, TResolve resolve, TReject reject)
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
    using Functor = std::function<void(QtPromise::QPromiseError)>;

    template <typename TResolve, typename TReject>
    static Functor create(std::nullptr_t, TResolve, TReject reject)
    {
        return [=](const QtPromise::QPromiseError& error) {
            // 2.2.7.4. If onRejected is not a function and promise1 is rejected,
            // promise2 must be rejected with the same reason as promise1
            reject(error);
        };
    }
};

template <class T>
struct PromiseDataBase: public QSharedData
{
    using ErrorType = QtPromise::QPromiseError;
    using CatcherList = QVector<std::function<void(ErrorType)> >;

    bool resolved;
    bool rejected;
    ErrorType error;
    CatcherList catchers;
};

template <typename T>
struct PromiseData: PromiseDataBase<T>
{
    using HandlerList = QVector<std::function<void(T)> >;

    HandlerList handlers;
    T value;
};

template <>
struct PromiseData<void>: PromiseDataBase<void>
{
    using HandlerList = QVector<std::function<void()> >;

    HandlerList handlers;
};

} // namespace QtPromise

#endif // ifndef _QTPROMISE_QPROMISE_H
