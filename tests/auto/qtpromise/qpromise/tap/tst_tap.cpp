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

class tst_qpromise_tap : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fulfilledSync();
    void fulfilledSync_void();
    void fulfilledThrows();
    void fulfilledThrows_void();
    void fulfilledAsyncResolve();
    void fulfilledAsyncReject();
    void rejectedSync();
    void rejectedSync_void();
};

QTEST_MAIN(tst_qpromise_tap)
#include "tst_tap.moc"

void tst_qpromise_tap::fulfilledSync()
{
    int value = -1;
    auto p = QPromise<int>::resolve(42).tap([&](int res) {
        value = res + 1;
        return 8;
    });

    QCOMPARE(waitForValue(p, 42), 42);
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(value, 43);
}

void tst_qpromise_tap::fulfilledSync_void()
{
    int value = -1;
    auto p = QPromise<void>::resolve().tap([&]() {
        value = 43;
        return 8;
    });

    QCOMPARE(waitForValue(p, -1, 42), 42);
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(value, 43);
}

void tst_qpromise_tap::fulfilledThrows()
{
    auto p = QPromise<int>::resolve(42).tap([&](int) {
        throw QString("foo");
    });

    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_tap::fulfilledThrows_void()
{
    auto p = QPromise<void>::resolve().tap([&]() {
        throw QString("foo");
    });

    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_tap::fulfilledAsyncResolve()
{
    QVector<int> values;
    auto p = QPromise<int>::resolve(1).tap([&](int) {
        QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
            QtPromisePrivate::qtpromise_defer([=, &values]() {
                values << 3;
                resolve(4); // ignored!
            });
        });

        values << 2;
        return p;
    });

    p.then([&](int r) {
        values << r;
    }).wait();

    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(values, QVector<int>({2, 3, 1}));
}

void tst_qpromise_tap::fulfilledAsyncReject()
{
    QVector<int> values;
    auto p = QPromise<int>::resolve(1).tap([&](int) {
        QPromise<int> p([&](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
            QtPromisePrivate::qtpromise_defer([=, &values]() {
                values << 3;
                reject(QString("foo"));
            });
        });

        values << 2;
        return p;
    });

    p.then([&](int r) {
        values << r;
    }).wait();

    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
    QCOMPARE(values, QVector<int>({2, 3}));
}

void tst_qpromise_tap::rejectedSync()
{
    int value = -1;
    auto p = QPromise<int>::reject(QString("foo")).tap([&](int res) {
        value = res + 1;
    });

    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
    QCOMPARE(value, -1);
}

void tst_qpromise_tap::rejectedSync_void()
{
    int value = -1;
    auto p = QPromise<void>::reject(QString("foo")).tap([&]() {
        value = 43;
    });

    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
    QCOMPARE(value, -1);
}
