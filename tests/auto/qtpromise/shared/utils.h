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

#ifndef QTPROMISE_TESTS_AUTO_SHARED_UTILS_H
#define QTPROMISE_TESTS_AUTO_SHARED_UTILS_H

#include <QtPromise>

template <typename T>
static inline T waitForValue(const QtPromise::QPromise<T>& promise, const T& initial)
{
    T value(initial);
    promise.then([&](const T& res) {
        value = res;
    }).wait();
    return value;
}

template <typename T>
static inline T waitForValue(const QtPromise::QPromise<void>& promise, const T& initial, const T& expected)
{
    T value(initial);
    promise.then([&]() {
        value = expected;
    }).wait();
    return value;
}

template <typename T, typename E>
static inline E waitForError(const QtPromise::QPromise<T>& promise, const E& initial)
{
    E error(initial);
    promise.fail([&](const E& err) {
        error = err;
        return T();
    }).wait();
    return error;
}

template <typename E>
static inline E waitForError(const QtPromise::QPromise<void>& promise, const E& initial)
{
    E error(initial);
    promise.fail([&](const E& err) {
        error = err;
    }).wait();
    return error;
}

template <typename E, typename T>
static inline bool waitForRejected(const T& promise)
{
    bool result = false;
    promise.tapFail([&](const E&) {
        result = true;
    }).wait();
    return result;
}

#endif // QTPROMISE_TESTS_AUTO_SHARED_UTILS_H
