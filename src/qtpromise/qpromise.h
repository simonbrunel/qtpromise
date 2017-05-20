#ifndef _QTPROMISE_QPROMISE_H
#define _QTPROMISE_QPROMISE_H

// QtPromise
#include "qpromise_p.h"
#include "qpromiseglobal.h"

// Qt
#include <QExplicitlySharedDataPointer>

namespace QtPromise {

template <typename T>
class QPromiseBase
{
public:
    using Type = T;

    virtual ~QPromiseBase() { }

    bool isFulfilled() const { return m_d->resolved && !m_d->rejected; }
    bool isRejected() const { return m_d->resolved && m_d->rejected; }
    bool isPending() const { return !m_d->resolved; }

    template <typename TFulfilled, typename TRejected = std::nullptr_t>
    inline typename QtPromisePrivate::PromiseHandler<T, TFulfilled>::Promise
    then(TFulfilled fulfilled, TRejected rejected = nullptr);

    template <typename TRejected>
    inline typename QtPromisePrivate::PromiseHandler<T, std::nullptr_t>::Promise
    fail(TRejected rejected);

    inline QPromise<T> wait() const;

public: // STATIC
    template <typename E>
    inline static QPromise<T> reject(const E& error);

protected:
    friend class QPromiseResolve<T>;
    friend class QPromiseReject<T>;

    QExplicitlySharedDataPointer<QtPromisePrivate::PromiseData<T> > m_d;

    inline QPromiseBase();

    template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count == 1, int>::type = 0>
    inline QPromiseBase(F resolver);

    template <typename F, typename std::enable_if<QtPromisePrivate::ArgsOf<F>::count != 1, int>::type = 0>
    inline QPromiseBase(F resolver);

    virtual void notify(const typename QtPromisePrivate::PromiseData<T>::HandlerList& handlers) const = 0;

    inline void dispatch();
};

template <typename T>
class QPromise: public QPromiseBase<T>
{
public:
    template <typename F>
    QPromise(F resolver): QPromiseBase<T>(resolver) { }

    template <typename THandler>
    inline QPromise<T> finally(THandler handler);

public: // STATIC
    inline static QPromise<QVector<T> > all(const QVector<QPromise<T> >& promises);
    inline static QPromise<T> resolve(const T& value);

protected:
    inline void notify(const typename QtPromisePrivate::PromiseData<T>::HandlerList& handlers) const Q_DECL_OVERRIDE;

private:
    friend class QPromiseBase<T>;

    QPromise(const QPromiseBase<T>& p) : QPromiseBase<T>(p) { }
};

template <>
class QPromise<void>: public QPromiseBase<void>
{
public:
    template <typename F>
    QPromise(F resolver): QPromiseBase<void>(resolver) { }

    template <typename THandler>
    inline QPromise<void> finally(THandler handler);

public: // STATIC
    inline static QPromise<void> all(const QVector<QPromise<void> >& promises);
    inline static QPromise<void> resolve();

protected:
    inline void notify(const typename QtPromisePrivate::PromiseData<void>::HandlerList& handlers) const Q_DECL_OVERRIDE;

private:
    friend class QPromiseBase<void>;

    QPromise(const QPromiseBase<void>& p) : QPromiseBase<void>(p) { }
};

} // namespace QtPromise

#include "qpromise.inl"
#include "qpromise_qfuture.inl"

#endif // ifndef _QTPROMISE_QPROMISE_H
