/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#ifndef QTPROMISE_QPROMISEHELPERS_P_H
#define QTPROMISE_QPROMISEHELPERS_P_H

#include "qpromiseconnections.h"
#include "qpromiseexceptions.h"
#include "qpromiseresolver.h"

namespace QtPromisePrivate {

// TODO: Suppress QPrivateSignal trailing private signal args

// Helper to apply Unqualified<> to every element of a std::tuple
template<typename Tuple>
struct TupleUnqualified;

template<typename... Args>
struct TupleUnqualified<std::tuple<Args...>>
{
    using type = std::tuple<Unqualified<Args>...>;
};

// Promise type deduced from a Qt signal
template<typename Signal>
using PromiseFromSignal = typename std::conditional<
    (ArgsOf<Signal>::count == 0),
    QtPromise::QPromise<void>,
    typename std::conditional<
        (ArgsOf<Signal>::count == 1),
        QtPromise::QPromise<Unqualified<typename ArgsOf<Signal>::first>>,
        QtPromise::QPromise<typename TupleUnqualified<typename ArgsOf<Signal>::types>::type>
    >::type
>::type;

// Connect signal() to QPromiseResolve
template<typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count == 0)>::type
connectSignalToResolver(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseResolve<void>& resolve,
                        const Sender* sender,
                        Signal signal)
{
    connections << QObject::connect(sender, signal, [=]() {
        connections.disconnect();
        resolve();
    });
}

// Connect signal() to QPromiseReject
template<typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count == 0)>::type
connectSignalToResolver(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseReject<T>& reject,
                        const Sender* sender,
                        Signal signal)
{
    connections << QObject::connect(sender, signal, [=]() {
        connections.disconnect();
        reject(QtPromise::QPromiseUndefinedException{});
    });
}

// Connect signal(args...) to QPromiseResolve (single argument)
template<typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count == 1)>::type
connectSignalToResolver(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseResolve<T>& resolve,
                        const Sender* sender,
                        Signal signal)
{
    connections << QObject::connect(sender, signal, [=](const T& value) {
        connections.disconnect();
        resolve(value);
    });
}

// Connect signal(args...) to QPromiseReject (single argument)
template<typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count == 1)>::type
connectSignalToResolver(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseReject<T>& reject,
                        const Sender* sender,
                        Signal signal)
{
    using V = Unqualified<typename ArgsOf<Signal>::first>;
    connections << QObject::connect(sender, signal, [=](const V& value) {
        connections.disconnect();
        reject(value);
    });
}

// Internal helpers for multi-argument signals (ArgsOf<Signal>::count >= 2)
template<typename T, typename Sender, typename Signal, typename Tuple>
struct MultiArgsSignalResolveConnector;

template<typename T, typename Sender, typename Signal, typename... Args>
struct MultiArgsSignalResolveConnector<T, Sender, Signal, std::tuple<Args...>>
{
    static void connect(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseResolve<T>& resolve,
                        const Sender* sender,
                        Signal signal)
    {
        connections << QObject::connect(sender, signal, [=](Args... args) {
            connections.disconnect();
            // T is expected to be std::tuple<Unqualified<Args>...>
            resolve(T(args...));
        });
    }
};

template<typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count >= 2)>::type
connectSignalToResolver(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseResolve<T>& resolve,
                        const Sender* sender,
                        Signal signal)
{
    typedef typename ArgsOf<Signal>::types ArgTuple;
    MultiArgsSignalResolveConnector<T, Sender, Signal, ArgTuple>::connect(
        connections, resolve, sender, signal);
}

template<typename T, typename Sender, typename Signal, typename Tuple>
struct MultiArgsSignalRejectConnector;

template<typename T, typename Sender, typename Signal, typename... Args>
struct MultiArgsSignalRejectConnector<T, Sender, Signal, std::tuple<Args...>>
{
    static void connect(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseReject<T>& reject,
                        const Sender* sender,
                        Signal signal)
    {
        connections << QObject::connect(sender, signal, [=](Args... args) {
            connections.disconnect();
            // Build an error payload as a tuple of unqualified argument types
            typedef typename TupleUnqualified<std::tuple<Args...>>::type ErrorTuple;
            reject(ErrorTuple(args...));
        });
    }
};

template<typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count >= 2)>::type
connectSignalToResolver(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseReject<T>& reject,
                        const Sender* sender,
                        Signal signal)
{
    typedef typename ArgsOf<Signal>::types ArgTuple;
    MultiArgsSignalRejectConnector<T, Sender, Signal, ArgTuple>::connect(
        connections, reject, sender, signal);
}

// Connect QObject::destroyed signal to QPromiseReject
template<typename T, typename Sender>
void connectDestroyedToReject(const QtPromise::QPromiseConnections& connections,
                              const QtPromise::QPromiseReject<T>& reject,
                              const Sender* sender)
{
    connections << QObject::connect(sender, &QObject::destroyed, [=]() {
        connections.disconnect();
        reject(QtPromise::QPromiseContextException{});
    });
}

} // namespace QtPromisePrivate

#endif // QTPROMISE_QPROMISEHELPERS_P_H
