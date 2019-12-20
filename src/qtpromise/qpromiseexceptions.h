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

#ifndef QTPROMISE_QPROMISEEXCEPTIONS_H
#define QTPROMISE_QPROMISEEXCEPTIONS_H

#include "qpromise_p.h"
#include "qpromiseglobal.h"

// Qt
#include <QException>

namespace QtPromise {

class QPromiseCanceledException : public QException
{
public:
    void raise() const Q_DECL_OVERRIDE { throw *this; }
    QPromiseCanceledException* clone() const Q_DECL_OVERRIDE
    {
        return new QPromiseCanceledException(*this);
    }
};

class QPromiseContextException : public QException
{
public:
    void raise() const Q_DECL_OVERRIDE { throw *this; }
    QPromiseContextException* clone() const Q_DECL_OVERRIDE
    {
        return new QPromiseContextException(*this);
    }
};

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

#endif // QTPROMISE_QPROMISEEXCEPTIONS_H
