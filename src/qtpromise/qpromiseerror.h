#ifndef QTPROMISE_QPROMISEERROR_H
#define QTPROMISE_QPROMISEERROR_H

// QtPromise
#include "qpromise_p.h"
#include "qpromiseglobal.h"

// Qt
#include <QException>

namespace QtPromisePrivate {

template <typename T>
class ExceptionBase : public QException
{
public:
    void raise() const Q_DECL_OVERRIDE { throw *this; }
    QException* clone() const Q_DECL_OVERRIDE
    {
        return new T(*reinterpret_cast<const T*>(this));
    }
};

} // namespace QtPromisePrivate

namespace QtPromise {

class QPromiseTimeoutException : public QtPromisePrivate::ExceptionBase<QPromiseTimeoutException>
{ };

class QPromiseContextException : public QtPromisePrivate::ExceptionBase<QPromiseContextException>
{ };

class QPromiseSignalException : public QtPromisePrivate::ExceptionBase<QPromiseSignalException>
{ };

// QPromiseError is provided for backward compatibility and will be
// removed in the next major version: it wasn't intended to be used
// directly and thus should not be part of the public API.
// TODO Remove QPromiseError at version 1.0
using QPromiseError = QtPromisePrivate::PromiseError;

} // namespace QtPromise

#endif // QTPROMISE_QPROMISEERROR_H
