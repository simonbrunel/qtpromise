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

#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtConcurrent>
#include <QtTest>

// STL
#include <memory>

using namespace QtPromise;

class tst_helpers_attempt : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void voidResult();
    void typedResult();
    void futureResult();
    void promiseResult();
    void functorThrows();
    void callWithParams();
};

QTEST_MAIN(tst_helpers_attempt)
#include "tst_attempt.moc"

void tst_helpers_attempt::voidResult()
{
    auto p = QtPromise::attempt([]() {});

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
}

void tst_helpers_attempt::typedResult()
{
    auto p = QtPromise::attempt([]() {
        return QString("foo");
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, QString()), QString("foo"));
}

void tst_helpers_attempt::futureResult()
{
    auto p = QtPromise::attempt([]() {
        return QtConcurrent::run([]() {
            return QString("foo");
        });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, QString()), QString("foo"));
}

void tst_helpers_attempt::promiseResult()
{
    auto p = QtPromise::attempt([]() {
        return QtPromise::resolve(42).delay(200);
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1), 42);
}

void tst_helpers_attempt::functorThrows()
{
    auto p = QtPromise::attempt([]() {
        if (true) {
            throw QString("bar");
        }
        return 42;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QCOMPARE(p.isRejected(), true);
    QCOMPARE(waitForError(p, QString()), QString("bar"));
}

void tst_helpers_attempt::callWithParams()
{
    auto p = QtPromise::attempt([&](int i, const QString& s) {
        return QString("%1:%2").arg(i).arg(s);
    }, 42, "foo");

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, QString()), QString("42:foo"));
}
