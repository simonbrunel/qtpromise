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

namespace QtPromisePrivate {

// TODO: Suppress QPrivateSignal trailing private signal args
// TODO: Support deducing tuple from args (might require MSVC2017)

template<typename Signal>
using PromiseFromSignal = typename QtPromise::QPromise<Unqualified<typename ArgsOf<Signal>::first>>;

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

// Connect signal(args...) to QPromiseResolve
template<typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count >= 1)>::type
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

// Connect signal(args...) to QPromiseReject
template<typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count >= 1)>::type
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

template<typename T, typename U>
struct AreArithmeticTypes
    : std::integral_constant<bool, std::is_arithmetic<T>::value && std::is_arithmetic<U>::value>
{ };

template<typename From, typename To, bool = AreArithmeticTypes<From, To>::value>
struct PromiseConverter;

template<typename From, typename To>
struct PromiseConverter<From, To, false>
{
    static inline QtPromise::QPromise<To> call(const QtPromise::QPromise<From>& p)
    {
        return p.then([](const From& value) {
            return To{value};
        });
    }
};

template<typename From, typename To>
struct PromiseConverter<From, To, true>
{
    static inline QtPromise::QPromise<To> call(const QtPromise::QPromise<From>& p)
    {
        return p.then([](From value) {
            return static_cast<To>(value);
        });
    }
};

template<typename From>
struct PromiseConverter<From, void, false>
{
    static inline QtPromise::QPromise<void> call(const QtPromise::QPromise<From>& p)
    {
        return p.then([]() {});
    }
};

template<typename To>
struct PromiseConverter<QVariant, To, false>
{
    static inline QtPromise::QPromise<To> call(const QtPromise::QPromise<QVariant>& p)
    {
        return p.then([](QVariant value) {
            // https://doc.qt.io/qt-5/qvariant.html#using-canconvert-and-convert-consecutively
            if (value.canConvert<To>() && value.convert(qMetaTypeId<To>())) {
                return value.value<To>();
            } else {
                throw QtPromise::QPromiseConversionException{};
            }
        });
    }
};

} // namespace QtPromisePrivate

#endif // QTPROMISE_QPROMISEHELPERS_P_H
