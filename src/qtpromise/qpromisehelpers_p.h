/*
 * Copyright (c) 2019 Simon Brunel, https://github.com/simonbrunel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef QTPROMISE_QPROMISEHELPERS_P_H
#define QTPROMISE_QPROMISEHELPERS_P_H

#include "qpromiseconnections.h"
#include "qpromiseexceptions.h"

namespace QtPromisePrivate {

// TODO: Suppress QPrivateSignal trailing private signal args
// TODO: Support deducing tuple from args (might require MSVC2017)

template <typename Signal>
using PromiseFromSignal = typename QtPromise::QPromise<Unqualified<typename ArgsOf<Signal>::first>>;

// Connect signal() to QPromiseResolve
template <typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count == 0)>::type
connectSignalToResolver(
    const QtPromise::QPromiseConnections& connections,
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
template <typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count == 0)>::type
connectSignalToResolver(
    const QtPromise::QPromiseConnections& connections,
    const QtPromise::QPromiseReject<T>& reject,
    const Sender* sender,
    Signal signal)
{
    connections << QObject::connect(sender, signal, [=]() {
        connections.disconnect();
        reject(QtPromise::QPromiseUndefinedException());
    });
}

// Connect signal(args...) to QPromiseResolve
template <typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count >= 1)>::type
connectSignalToResolver(
    const QtPromise::QPromiseConnections& connections,
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
template <typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count >= 1)>::type
connectSignalToResolver(
    const QtPromise::QPromiseConnections& connections,
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
template <typename T, typename Sender>
void connectDestroyedToReject(
    const QtPromise::QPromiseConnections& connections,
    const QtPromise::QPromiseReject<T>& reject,
    const Sender* sender)
{
    connections << QObject::connect(sender, &QObject::destroyed, [=]() {
        connections.disconnect();
        reject(QtPromise::QPromiseContextException());
    });
}

} // namespace QtPromisePrivate

#endif // QTPROMISE_QPROMISEHELPERS_P_H
