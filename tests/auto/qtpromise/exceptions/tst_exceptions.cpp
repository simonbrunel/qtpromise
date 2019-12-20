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

#include "../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtConcurrent>
#include <QtTest>

using namespace QtPromise;

class tst_exceptions : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void canceled();
    void context();
    void timeout();
    void undefined();

}; // class tst_exceptions

QTEST_MAIN(tst_exceptions)
#include "tst_exceptions.moc"

namespace {

template <class E>
void verify()
{
    auto p = QtPromise::resolve(QtConcurrent::run([]() { throw E(); }));
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForRejected<E>(p), true);
    QCOMPARE(p.isRejected(), true);
}

} // anonymous namespace

void tst_exceptions::canceled()
{
    verify<QPromiseCanceledException>();
}

void tst_exceptions::context()
{
    verify<QPromiseContextException>();
}

void tst_exceptions::timeout()
{
    verify<QPromiseTimeoutException>();
}

void tst_exceptions::undefined()
{
    verify<QPromiseUndefinedException>();
}
