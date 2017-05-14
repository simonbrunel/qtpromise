#ifndef _QTPROMISE_QPROMISE_P_H
#define _QTPROMISE_QPROMISE_P_H

// QPromise
#include "qpromiseglobal.h"

// Qt
#include <QCoreApplication>
#include <QTimer>
#include <QSharedData>
#include <QVector>

// STL
#include <functional>

namespace QtPromise {
template <typename T = void>
class QPromise;
}

namespace QtPromisePrivate {

using namespace QtPromise;

template <typename F>
inline void qtpromise_defer(F f)
{
    QTimer::singleShot(0, f);
}

template <typename T>
struct QPromiseDeduce
{
    using Type = QPromise<Unqualified<T> >;
};

template <typename T>
struct QPromiseDeduce<QPromise<T> >
    : public QPromiseDeduce<T>
{ };

template <typename T, typename TPromise = typename QPromiseDeduce<T>::Type>
struct QPromiseFulfill
{
    static void call(TPromise next, const T& value)
    {
        next.fulfill(value);
    }
};

template <typename T>
struct QPromiseFulfill<QPromise<T>, QPromise<T> >
{
    static void call(QPromise<T> next, QPromise<T> promise)
    {
        promise.then(
            [=](const T& value) mutable {
                next.fulfill(value);
            },
            [=]() mutable {
                next.reject(std::current_exception());
            });
    }
};

template <>
struct QPromiseFulfill<QPromise<void>, QPromise<void> >
{
    template <typename TPromise>
    static void call(TPromise next, TPromise promise)
    {
        promise.then(
            [=]() mutable {
                next.fulfill();
            },
            [=]() mutable {
                next.reject(std::current_exception());
            });
    }
};

template <typename T, typename TRes>
struct QPromiseDispatch
{
    using Promise = typename QPromiseDeduce<TRes>::Type;
    using ResType = Unqualified<TRes>;

    template <typename TPromise, typename THandler>
    static void call(TPromise next, THandler handler, const T& value)
    {
        ResType res;
        try {
            res = handler(value);
        } catch (...) {
            next.reject(std::current_exception());
            return;
        }
        QPromiseFulfill<ResType>::call(next, res);
    }
};

template <typename T>
struct QPromiseDispatch<T, void>
{
    using Promise = QPromise<void>;

    template <typename TPromise, typename THandler>
    static void call(TPromise next, THandler handler, const T& value)
    {
        try {
            handler(value);
        } catch (...) {
            next.reject(std::current_exception());
            return;
        }
        next.fulfill();
    }
};

template <typename TRes>
struct QPromiseDispatch<void, TRes>
{
    using Promise = typename QPromiseDeduce<TRes>::Type;
    using ResType = Unqualified<TRes>;

    template <typename TPromise, typename THandler>
    static void call(TPromise next, THandler handler)
    {
        ResType res;
        try {
            res = handler();
        } catch (...) {
            next.reject(std::current_exception());
            return;
        }
        QPromiseFulfill<ResType>::call(next, res);
    }
};

template <>
struct QPromiseDispatch<void, void>
{
    using Promise = QPromise<void>;

    template <typename TPromise, typename THandler>
    static void call(TPromise next, THandler handler)
    {
        try {
            handler();
        } catch (...) {
            next.reject(std::current_exception());
            return;
        }
        next.fulfill();
    }
};

template <typename T, typename THandler, typename TArg = typename ArgsOf<THandler>::first>
struct QPromiseHandler
{
    using ResType = typename std::result_of<THandler(T)>::type;
    using Promise = typename QPromiseDispatch<T, ResType>::Promise;

    static std::function<void(T)> create(const Promise& next, THandler handler)
    {
        return [=](const T& value) mutable {
            QPromiseDispatch<T, ResType>::call(next, handler, value);
        };
    }
};

template <typename T, typename THandler>
struct QPromiseHandler<T, THandler, void>
{
    using ResType = typename std::result_of<THandler()>::type;
    using Promise = typename QPromiseDispatch<T, ResType>::Promise;

    static std::function<void(T)> create(const Promise& next, THandler handler)
    {
        return [=](const T&) mutable {
            QPromiseDispatch<void, ResType>::call(next, handler);
        };
    }
};

template <typename THandler>
struct QPromiseHandler<void, THandler, void>
{
    using ResType = typename std::result_of<THandler()>::type;
    using Promise = typename QPromiseDispatch<void, ResType>::Promise;

    static std::function<void()> create(const Promise& next, THandler handler)
    {
        return [=]() {
            QPromiseDispatch<void, ResType>::call(next, handler);
        };
    }
};

template <typename T>
struct QPromiseHandler<T, std::nullptr_t, void>
{
    using Promise = QPromise<T>;

    static std::function<void(T)> create(const Promise& next, std::nullptr_t)
    {
        return [next](const T& value) mutable {
            // 2.2.7.3. If onFulfilled is not a function and promise1 is fulfilled,
            // promise2 must be fulfilled with the same value as promise1.
            QPromiseFulfill<T>::call(next, value);
        };
    }
};

template <>
struct QPromiseHandler<void, std::nullptr_t, void>
{
    using Promise = QPromise<void>;

    template <typename TPromise>
    static std::function<void()> create(const TPromise& next, std::nullptr_t)
    {
        return [=]() mutable {
            // 2.2.7.3. If onFulfilled is not a function and promise1 is fulfilled,
            // promise2 must be fulfilled with the same value as promise1.
            TPromise(next).fulfill();
        };
    }
};

template <typename T, typename THandler, typename TArg = typename ArgsOf<THandler>::first>
struct QPromiseCatcher
{
    using Type = std::function<void(std::exception_ptr)>;
    using ResType = typename std::result_of<THandler(TArg)>::type;

    template <typename TPromise>
    static Type create(const TPromise& next, THandler handler)
    {
        return [=](const std::exception_ptr& eptr) mutable {
            try {
                std::rethrow_exception(eptr);
            } catch (const TArg& error) {
                QPromiseDispatch<TArg, ResType>::call(next, handler, error);
            } catch(...) {
                TPromise(next).reject(std::current_exception());
            }
        };
    }
};

template <typename T, typename THandler>
struct QPromiseCatcher<T, THandler, void>
{
    using Type = std::function<void(std::exception_ptr)>;
    using ResType = typename std::result_of<THandler()>::type;

    template <typename TPromise>
    static Type create(const TPromise& next, THandler handler)
    {
        return [=](const std::exception_ptr& eptr) mutable {
            try {
                std::rethrow_exception(eptr);
            } catch (...) {
                QPromiseDispatch<void, ResType>::call(next, handler);
            }
        };
    }
};

template <typename T>
struct QPromiseCatcher<T, std::nullptr_t, void>
{
    using Type = std::function<void(std::exception_ptr)>;

    template <typename TPromise>
    static Type create(const TPromise& next, std::nullptr_t)
    {
        return [=](const std::exception_ptr& eptr) mutable {
            // 2.2.7.4. If onRejected is not a function and promise1 is rejected,
            // promise2 must be rejected with the same reason as promise1
            TPromise(next).reject(eptr);
        };
    }
};

template <class T>
struct QPromiseDataBase: public QSharedData
{
    using ErrorType = std::exception_ptr;
    using CatcherList = QVector<std::function<void(ErrorType)> >;

    bool resolved;
    bool rejected;
    ErrorType error;
    CatcherList catchers;
};

template <typename T>
struct QPromiseData: QPromiseDataBase<T>
{
    using HandlerList = QVector<std::function<void(T)> >;

    HandlerList handlers;
    T value;
};

template <>
struct QPromiseData<void>: QPromiseDataBase<void>
{
    using HandlerList = QVector<std::function<void()> >;

    HandlerList handlers;
};

} // namespace QtPromise

#endif // ifndef _QTPROMISE_QPROMISE_H
