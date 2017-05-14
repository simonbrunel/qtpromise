#ifndef _QTPROMISE_QPROMISE_H
#define _QTPROMISE_QPROMISE_H

// QPromise
#include "qpromise_p.h"
#include "qpromiseglobal.h"

// Qt
#include <QExplicitlySharedDataPointer>
#include <QVariant>

namespace QtPromise {

using namespace QtPromisePrivate;

template <typename T>
class QPromiseBase
{
public:
    using Type = T;

    QPromiseBase() : m_d(new QPromiseData<T>()) { }
    virtual ~QPromiseBase() { }

    bool isFulfilled() const { return m_d->resolved && !m_d->rejected; }
    bool isRejected() const { return m_d->resolved && m_d->rejected; }
    bool isPending() const { return !m_d->resolved; }

    template <typename TFulfilled, typename TRejected = std::nullptr_t>
    inline auto then(TFulfilled fulfilled, TRejected rejected = nullptr)
        -> typename QPromiseHandler<T, TFulfilled>::Promise;

    template <typename TRejected>
    inline auto fail(TRejected rejected)
        -> typename QPromiseHandler<T, std::nullptr_t>::Promise;

    template <typename TError>
    inline QPromise<T> reject(const TError& error);

    inline QPromise<T> wait() const;

protected:
    QExplicitlySharedDataPointer<QPromiseData<T> > m_d;

    virtual void notify(const typename QPromiseData<T>::HandlerList& handlers) const = 0;

    inline void dispatch();
};

template <typename T>
class QPromise: public QPromiseBase<T>
{
public:
    QPromise() : QPromiseBase<T>() {}

    template <typename THandler>
    inline QPromise<T> finally(THandler handler);

    inline QPromise<T> fulfill(const T& value);

public: // STATIC
    inline static QPromise<QVector<T> > all(const QVector<QPromise<T> >& promises);

protected:
    inline void notify(const typename QPromiseData<T>::HandlerList& handlers) const Q_DECL_OVERRIDE;

private:
    friend class QPromiseBase<T>;

    QPromise(const QPromiseBase<T>& p) : QPromiseBase<T>(p) { }

};

template <>
class QPromise<void>: public QPromiseBase<void>
{
public:
    QPromise(): QPromiseBase<void>() {}

    template <typename THandler>
    inline QPromise<void> finally(THandler handler);

    inline QPromise<void> fulfill();

public: // STATIC
    inline static QPromise<void> all(const QVector<QPromise<void> >& promises);

protected:
    inline void notify(const typename QPromiseData<void>::HandlerList& handlers) const Q_DECL_OVERRIDE;

private:
    friend class QPromiseBase<void>;

    QPromise(const QPromiseBase<void>& p) : QPromiseBase<void>(p) { }
};

using QVariantPromise = QtPromise::QPromise<QVariant>;

} // namespace QtPromise

Q_DECLARE_TYPEINFO(QtPromise::QVariantPromise, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(QtPromise::QVariantPromise)

#include "qpromise.inl"
#include "qpromise_qfuture.inl"

#endif // ifndef _QTPROMISE_QPROMISE_H
