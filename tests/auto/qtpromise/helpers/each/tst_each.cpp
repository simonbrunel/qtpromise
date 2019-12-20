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

class tst_helpers_each : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptySequence();
    void preserveValues();
    void ignoreResult();
    void delayedFulfilled();
    void delayedRejected();
    void functorThrows();
    void functorArguments();
    void sequenceTypes();
};

QTEST_MAIN(tst_helpers_each)
#include "tst_each.moc"

namespace {

template <class Sequence>
struct SequenceTester
{
    static void exec()
    {
        QVector<int> values;
        auto p = QtPromise::each(Sequence{42, 43, 44}, [&](int v, int i) {
            values << i << v;
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Sequence>>::value));
        QCOMPARE(waitForValue(p, Sequence()), Sequence({42, 43, 44}));
        QCOMPARE(values, QVector<int>({0, 42, 1, 43, 2, 44}));
    }
};

} // anonymous namespace

void tst_helpers_each::emptySequence()
{
    QVector<int> values;
    auto p = QtPromise::each(QVector<int>{}, [&](int v, ...) {
        values << v;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>());
    QCOMPARE(values, QVector<int>({}));
}

void tst_helpers_each::preserveValues()
{
    QVector<int> values;
    auto p = QtPromise::each(QVector<int>{42, 43, 44}, [&](int v, ...) {
        values << v + 1;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
    QCOMPARE(values, QVector<int>({43, 44, 45}));
}

void tst_helpers_each::ignoreResult()
{
    QVector<int> values;
    auto p = QtPromise::each(QVector<int>{42, 43, 44}, [&](int v, ...) {
        values << v + 1;
        return "Foo";
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
    QCOMPARE(values, QVector<int>({43, 44, 45}));
}

void tst_helpers_each::delayedFulfilled()
{
    QMap<int, int> values;
    auto p = QtPromise::each(QVector<int>{42, 43, 44}, [&](int v, int index) {
        return QPromise<int>([&](const QPromiseResolve<int>& resolve) {
                QtPromisePrivate::qtpromise_defer([=, &values]() {
                    values[v] = index;
                    resolve(42);
                });
            });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
    QMap<int, int> expected{{42, 0}, {43, 1}, {44, 2}};
    QCOMPARE(values, expected);
}

void tst_helpers_each::delayedRejected()
{
    auto p = QtPromise::each(QVector<int>{42, 43, 44}, [](int v, ...) {
        return QPromise<int>([&](
            const QPromiseResolve<int>& resolve,
            const QPromiseReject<int>& reject) {
                QtPromisePrivate::qtpromise_defer([=]() {
                    if (v == 43) {
                        reject(QString("foo"));
                    }
                    resolve(v);
                });
            });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_helpers_each::functorThrows()
{
    auto p = QtPromise::each(QVector<int>{42, 43, 44}, [](int v, ...) {
        if (v == 44) {
            throw QString("foo");
        }
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_helpers_each::functorArguments()
{
    QVector<int> values;
    auto p = QtPromise::each(QVector<int>{42, 43, 44}, [&](int v, int i) {
        values << i << v;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
    QCOMPARE(values, QVector<int>({0, 42, 1, 43, 2, 44}));
}

void tst_helpers_each::sequenceTypes()
{
    SequenceTester<QList<int>>::exec();
    SequenceTester<QVector<int>>::exec();
    SequenceTester<std::list<int>>::exec();
    SequenceTester<std::vector<int>>::exec();
}
