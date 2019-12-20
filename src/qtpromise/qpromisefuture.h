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

#ifndef QTPROMISE_QPROMISEFUTURE_P_H
#define QTPROMISE_QPROMISEFUTURE_P_H

#include "qpromiseexceptions.h"

// Qt
#include <QFutureWatcher>
#include <QFuture>

namespace QtPromisePrivate {

template <typename T>
struct PromiseDeduce<QFuture<T>>
    : public PromiseDeduce<T>
{ };

template <typename T>
struct PromiseFulfill<QFuture<T>>
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
                if (watcher->isCanceled()) {
                    // A QFuture is canceled if cancel() has been explicitly called OR if an
                    // exception has been thrown from the associated thread. Trying to call
                    // result() in the first case causes a "read access violation", so let's
                    // rethrown potential exceptions using waitForFinished() and thus detect
                    // if the future has been canceled by the user or an exception.
                    watcher->waitForFinished();
                    reject(QtPromise::QPromiseCanceledException());
                } else {
                    PromiseFulfill<T>::call(watcher->result(), resolve, reject);
                }
            } catch (...) {
                reject(std::current_exception());
            }

            watcher->deleteLater();
        });

        watcher->setFuture(future);
    }
};

template <>
struct PromiseFulfill<QFuture<void>>
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
                if (watcher->isCanceled()) {
                    // let's rethrown potential exception
                    watcher->waitForFinished();
                    reject(QtPromise::QPromiseCanceledException());
                } else {
                    resolve();
                }
            } catch (...) {
                reject(std::current_exception());
            }

            watcher->deleteLater();
        });

        watcher->setFuture(future);
    }
};

} // namespace QtPromisePrivate

#endif // QTPROMISE_QPROMISEFUTURE_P_H
