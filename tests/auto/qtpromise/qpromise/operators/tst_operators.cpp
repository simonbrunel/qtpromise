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
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_operators : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void move();
    void move_void();
    void copy();
    void copy_void();
    void equalTo();
    void equalTo_void();
    void notEqualTo();
    void notEqualTo_void();
    void chaining();
    void chaining_void();
};

QTEST_MAIN(tst_qpromise_operators)
#include "tst_operators.moc"

void tst_qpromise_operators::move()
{
    auto p0 = QPromise<int>::resolve(42);

    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(waitForValue(p0, -1), 42);

    p0 = QPromise<int>([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        QtPromisePrivate::qtpromise_defer([=]() {
            reject(QString("foo"));
        });
    });

    QCOMPARE(p0.isPending(), true);
    QCOMPARE(waitForError(p0, QString()), QString("foo"));

    p0 = QPromise<int>([](const QPromiseResolve<int>& resolve) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve(43);
        });
    });

    QCOMPARE(p0.isPending(), true);
    QCOMPARE(waitForValue(p0, -1), 43);
}

void tst_qpromise_operators::move_void()
{
    auto p0 = QPromise<void>::resolve();

    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(waitForValue(p0, -1, 42), 42);

    p0 = QPromise<void>([](const QPromiseResolve<void>&, const QPromiseReject<void>& reject) {
        QtPromisePrivate::qtpromise_defer([=]() {
            reject(QString("foo"));
        });
    });

    QCOMPARE(p0.isPending(), true);
    QCOMPARE(waitForError(p0, QString()), QString("foo"));

    p0 = QPromise<void>([](const QPromiseResolve<void>& resolve) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve();
        });
    });

    QCOMPARE(p0.isPending(), true);
    QCOMPARE(waitForValue(p0, -1, 43), 43);
}

void tst_qpromise_operators::copy()
{
    auto p0 = QPromise<int>([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        QtPromisePrivate::qtpromise_defer([=]() {
            reject(QString("foo"));
        });
    });

    auto p1 = QPromise<int>([](const QPromiseResolve<int>& resolve) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve(42);
        });
    });

    QCOMPARE(p0 == p1, false);
    QCOMPARE(p0.isPending(), true);
    QCOMPARE(p1.isPending(), true);

    p0 = p1;

    QCOMPARE(p0 == p1, true);
    QCOMPARE(p0.isPending(), true);
    QCOMPARE(p1.isPending(), true);
    QCOMPARE(waitForValue(p0, -1), 42);
    QCOMPARE(waitForValue(p1, -1), 42);
}

void tst_qpromise_operators::copy_void()
{
    auto p0 = QPromise<void>([](const QPromiseResolve<void>&, const QPromiseReject<void>& reject) {
        QtPromisePrivate::qtpromise_defer([=]() {
            reject(QString("foo"));
        });
    });

    auto p1 = QPromise<void>([](const QPromiseResolve<void>& resolve) {
        QtPromisePrivate::qtpromise_defer([=]() {
            resolve();
        });
    });

    QCOMPARE(p0 == p1, false);
    QCOMPARE(p0.isPending(), true);
    QCOMPARE(p1.isPending(), true);

    p0 = p1;

    QCOMPARE(p0 == p1, true);
    QCOMPARE(p0.isPending(), true);
    QCOMPARE(p1.isPending(), true);

    p0.wait();

    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
}

void tst_qpromise_operators::equalTo()
{
    auto p0 = QPromise<int>::resolve(42);
    auto p1 = QPromise<int>::resolve(42);
    auto p2 = p1;
    auto p3(p2);

    QCOMPARE(p0 == p1, false);
    QCOMPARE(p0 == p2, false);
    QCOMPARE(p0 == p3, false);
    QCOMPARE(p1 == p2, true);
    QCOMPARE(p1 == p3, true);
    QCOMPARE(p2 == p3, true);
}

void tst_qpromise_operators::equalTo_void()
{
    auto p0 = QPromise<void>::resolve();
    auto p1 = QPromise<void>::resolve();
    auto p2 = p1;
    auto p3(p2);

    QCOMPARE(p0 == p1, false);
    QCOMPARE(p0 == p2, false);
    QCOMPARE(p0 == p3, false);
    QCOMPARE(p1 == p2, true);
    QCOMPARE(p1 == p3, true);
    QCOMPARE(p2 == p3, true);
}

void tst_qpromise_operators::notEqualTo()
{
    auto p0 = QPromise<int>::resolve(42);
    auto p1 = QPromise<int>::resolve(42);
    auto p2 = p1;
    auto p3(p2);

    QCOMPARE(p0 != p1, true);
    QCOMPARE(p0 != p2, true);
    QCOMPARE(p0 != p3, true);
    QCOMPARE(p1 != p2, false);
    QCOMPARE(p1 != p3, false);
    QCOMPARE(p2 != p3, false);
}

void tst_qpromise_operators::notEqualTo_void()
{
    auto p0 = QPromise<void>::resolve();
    auto p1 = QPromise<void>::resolve();
    auto p2 = p1;
    auto p3(p2);

    QCOMPARE(p0 != p1, true);
    QCOMPARE(p0 != p2, true);
    QCOMPARE(p0 != p3, true);
    QCOMPARE(p1 != p2, false);
    QCOMPARE(p1 != p3, false);
    QCOMPARE(p2 != p3, false);
}

void tst_qpromise_operators::chaining()
{
    auto p = QPromise<int>::resolve(1);
    for (int i=0; i<4; ++i) {
        p = p.then([](int res) {
            return QPromise<int>::resolve(res * 2);
        });
    }

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1), 16);
}

void tst_qpromise_operators::chaining_void()
{
    QVector<int> values;

    auto p = QPromise<void>::resolve();

    for (int i=0; i<4; ++i) {
        p = p.then([i, &values]() {
            values.append(i * 2);
            return QPromise<void>::resolve();
        });
    }

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
    QCOMPARE(values, QVector<int>({0, 2, 4, 6}));
}
