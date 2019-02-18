#ifndef QTPROMISE_QPROMISEERROR_H
#define QTPROMISE_QPROMISEERROR_H

#include "qpromise_p.h"
#include "qpromiseglobal.h"

// Qt
#include <QException>

namespace QtPromise {

class QPromiseTimeoutException : public QException
{
public:
    void raise() const Q_DECL_OVERRIDE { throw *this; }
    QPromiseTimeoutException* clone() const Q_DECL_OVERRIDE
    {
        return new QPromiseTimeoutException(*this);
    }
};

class QPromiseUndefinedException : public QException
{
public:
    void raise() const Q_DECL_OVERRIDE { throw *this; }
    QPromiseUndefinedException* clone() const Q_DECL_OVERRIDE
    {
        return new QPromiseUndefinedException(*this);
    }
};

// QPromiseError is provided for backward compatibility and will be
// removed in the next major version: it wasn't intended to be used
// directly and thus should not be part of the public API.
// TODO Remove QPromiseError at version 1.0
using QPromiseError = QtPromisePrivate::PromiseError;

} // namespace QtPromise

#endif // QTPROMISE_QPROMISEERROR_H
