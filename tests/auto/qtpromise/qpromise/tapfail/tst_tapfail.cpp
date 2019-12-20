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

class tst_qpromise_tapfail : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fulfilled();
    void fulfilled_void();
    void rejected();
    void rejected_void();
    void throws();
    void throws_void();
    void delayedResolved();
    void delayedRejected();
};

QTEST_MAIN(tst_qpromise_tapfail)
#include "tst_tapfail.moc"

void tst_qpromise_tapfail::fulfilled()
{
    int value = -1;
    auto p = QPromise<int>::resolve(42).tapFail([&]() {
        value = 43;
    });

    QCOMPARE(waitForValue(p, 42), 42);
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(value, -1);
}

void tst_qpromise_tapfail::fulfilled_void()
{
    int value = -1;
    auto p = QPromise<void>::resolve().tapFail([&]() {
        value = 43;
    });

    QCOMPARE(waitForValue(p, -1, 42), 42);
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(value, -1);
}

void tst_qpromise_tapfail::rejected()
{
    QStringList errors;

    auto p0 = QPromise<int>::reject(QString("foo"))
        .tapFail([&](const QString& err) {
            errors << "1:" + err;
        });

    auto p1 = p0
        .fail([&](const QString& err) {
            errors << "2:" + err;
            return 43;
        });

    QCOMPARE(waitForError(p0, QString()), QString("foo"));
    QCOMPARE(waitForValue(p1, -1), 43);
    QCOMPARE(p0.isRejected(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(errors, QStringList() << "1:foo" << "2:foo");
}

void tst_qpromise_tapfail::rejected_void()
{
    QStringList errors;

    auto p0 = QPromise<void>::reject(QString("foo"))
        .tapFail([&](const QString& err) {
            errors << "1:" + err;
        });

    auto p1 = p0
        .fail([&](const QString& err) {
            errors << "2:" + err;
        });

    QCOMPARE(waitForError(p0, QString()), QString("foo"));
    QCOMPARE(waitForValue(p1, -1, 43), 43);
    QCOMPARE(p0.isRejected(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(errors, QStringList() << "1:foo" << "2:foo");
}

void tst_qpromise_tapfail::throws()
{
    auto p = QPromise<int>::reject(QString("foo"))
        .tapFail([&]() {
            throw QString("bar");
        });

    QCOMPARE(waitForError(p, QString()), QString("bar"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_tapfail::throws_void()
{
    auto p = QPromise<void>::reject(QString("foo"))
        .tapFail([&]() {
            throw QString("bar");
        });

    QCOMPARE(waitForError(p, QString()), QString("bar"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise_tapfail::delayedResolved()
{
    QVector<int> values;
    auto p = QPromise<int>::reject(QString("foo"))
        .tapFail([&]() {
            QPromise<void> p([&](const QPromiseResolve<void>& resolve) {
                    QtPromisePrivate::qtpromise_defer([=, &values]() {
                        values << 3;
                        resolve(); // ignored!
                    });
                });

            values << 2;
            return p;
        });

    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(values, QVector<int>({2, 3}));
}

void tst_qpromise_tapfail::delayedRejected()
{
    QVector<int> values;
    auto p = QPromise<int>::reject(QString("foo"))
        .tapFail([&]() {
            QPromise<void> p([&](
                const QPromiseResolve<void>&,
                const QPromiseReject<void>& reject){
                    QtPromisePrivate::qtpromise_defer([=, &values]() {
                        values << 3;
                        reject(QString("bar"));
                    });
                });

            values << 2;
            return p;
        });

    QCOMPARE(waitForError(p, QString()), QString("bar"));
    QCOMPARE(values, QVector<int>({2, 3}));
}
