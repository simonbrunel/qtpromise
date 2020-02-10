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

#ifndef QTPROMISE_QPROMISE_H
#define QTPROMISE_QPROMISE_H

#include "qpromise_p.h"
#include "qpromiseexceptions.h"
#include "qpromiseglobal.h"
#include "qpromiseresolver.h"

// Qt
#include <QExplicitlySharedDataPointer>

#if __has_include(<chrono>)
#include <chrono>
#endif

namespace QtPromise {

template <typename T>
class QPromiseBase
{
public:
    using Type = T;

    template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count == 1, int>::type = 0>
    inline QPromiseBase(F resolver);

    template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count != 1, int>::type = 0>
    inline QPromiseBase(F resolver);

    QPromiseBase(const QPromiseBase<T>& other): m_d(other.m_d) {}
    QPromiseBase(const QPromise<T>& other): m_d(other.m_d) {}
    QPromiseBase(QPromiseBase<T>&& other) Q_DECL_NOEXCEPT { swap(other); }

    virtual ~QPromiseBase() { }

    QPromiseBase<T>& operator=(const QPromiseBase<T>& other) { m_d = other.m_d; return *this;}
    QPromiseBase<T>& operator=(QPromiseBase<T>&& other) Q_DECL_NOEXCEPT
    { QPromiseBase<T>(std::move(other)).swap(*this); return *this; }

    bool operator==(const QPromiseBase<T>& other) const { return (m_d == other.m_d); }
    bool operator!=(const QPromiseBase<T>& other) const { return (m_d != other.m_d); }

    void swap(QPromiseBase<T>& other) Q_DECL_NOEXCEPT { qSwap(m_d, other.m_d); }

    bool isFulfilled() const { return m_d->isFulfilled(); }
    bool isRejected() const { return m_d->isRejected(); }
    bool isPending() const { return m_d->isPending(); }

    template <typename TFulfilled, typename TRejected>
    inline typename QtPromisePrivate::PromiseHandler<T, TFulfilled>::Promise
    then(const TFulfilled& fulfilled, const TRejected& rejected) const;

    template <typename TFulfilled>
    inline typename QtPromisePrivate::PromiseHandler<T, TFulfilled>::Promise
    then(TFulfilled&& fulfilled) const;

    template <typename TRejected>
    inline typename QtPromisePrivate::PromiseHandler<T, std::nullptr_t>::Promise
    fail(TRejected&& rejected) const;

    template <typename THandler>
    inline QPromise<T> finally(THandler handler) const;

    template <typename THandler>
    inline QPromise<T> tap(THandler handler) const;

    template <typename THandler>
    inline QPromise<T> tapFail(THandler handler) const;

    template <typename E = QPromiseTimeoutException>
    inline QPromise<T> timeout(int msec, E&& error = E()) const;   
#if __has_include(<chrono>)
    template <typename E = QPromiseTimeoutException>
    inline QPromise<T> timeout(std::chrono::milliseconds msec, E&& error = E()) const
    {
        return timeout(static_cast<int>(msec.count()), std::forward<E>(error));
    }
#endif

    inline QPromise<T> delay(int msec) const;
#if __has_include(<chrono>)
    inline QPromise<T> delay(std::chrono::milliseconds msec) const
    {
        return delay(static_cast<int>(msec.count()));
    }
#endif
    inline QPromise<T> wait() const;

public: // STATIC
    template <typename E>
    inline static QPromise<T> reject(E&& error);

protected:
    friend struct QtPromisePrivate::PromiseFulfill<QPromise<T>>;
    friend class QtPromisePrivate::PromiseResolver<T>;
    friend struct QtPromisePrivate::PromiseInspect;

    QExplicitlySharedDataPointer<QtPromisePrivate::PromiseData<T>> m_d;
};

template <typename T>
class QPromise : public QPromiseBase<T>
{
public:
    template <typename F>
    QPromise(F&& resolver): QPromiseBase<T>(std::forward<F>(resolver)) { }

    template <typename Functor>
    inline QPromise<T>
    each(Functor fn);

    template <typename Functor>
    inline QPromise<T>
    filter(Functor fn);

    template <typename Functor>
    inline typename QtPromisePrivate::PromiseMapper<T, Functor>::PromiseType
    map(Functor fn);

    template <typename Functor, typename Input>
    inline typename QtPromisePrivate::PromiseDeduce<Input>::Type
    reduce(Functor fn, Input initial);

    template <typename Functor, typename U = T>
    inline typename QtPromisePrivate::PromiseDeduce<typename U::value_type>::Type
    reduce(Functor fn);

public: // STATIC

    // DEPRECATED (remove at version 1)
    template <template <typename, typename...> class Sequence = QVector, typename ...Args>
    Q_DECL_DEPRECATED_X("Use QtPromise::all instead") static inline QPromise<QVector<T>>
    all(const Sequence<QPromise<T>, Args...>& promises);

    inline static QPromise<T> resolve(const T& value);
    inline static QPromise<T> resolve(T&& value);

private:
    friend class QPromiseBase<T>;
};

template <>
class QPromise<void> : public QPromiseBase<void>
{
public:
    template <typename F>
    QPromise(F&& resolver): QPromiseBase<void>(std::forward<F>(resolver)) { }

public: // STATIC

    // DEPRECATED (remove at version 1)
    template <template <typename, typename...> class Sequence = QVector, typename ...Args>
    Q_DECL_DEPRECATED_X("Use QtPromise::all instead") static inline QPromise<void>
    all(const Sequence<QPromise<void>, Args...>& promises);

    inline static QPromise<void> resolve();

private:
    friend class QPromiseBase<void>;
};

} // namespace QtPromise

#include "qpromise.inl"

#endif // QTPROMISE_QPROMISE_H
