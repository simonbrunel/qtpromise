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

class tst_helpers_map : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptySequence();
    void modifyValues();
    void convertValues();
    void delayedFulfilled();
    void delayedRejected();
    void functorThrows();
    void functorArguments();
    void preserveOrder();
    void sequenceTypes();
};

QTEST_MAIN(tst_helpers_map)
#include "tst_map.moc"

namespace {

template <class Sequence>
struct SequenceTester
{
    static void exec()
    {
        auto p = QtPromise::map(Sequence{42, 43, 44}, [](int v, ...) {
            return QString::number(v + 1);
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<QString>>>::value));
        QCOMPARE(waitForValue(p, QVector<QString>()), QVector<QString>({"43", "44", "45"}));
    }
};

} // anonymous namespace

void tst_helpers_map::emptySequence()
{
    auto p = QtPromise::map(QVector<int>{}, [](int v, ...) {
        return v + 1;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({}));
}

void tst_helpers_map::modifyValues()
{
    auto p = QtPromise::map(QVector<int>{42, 43, 44}, [](int v, ...) {
        return v + 1;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({43, 44, 45}));
}

void tst_helpers_map::convertValues()
{
    auto p = QtPromise::map(QVector<int>{42, 43, 44}, [](int v, ...) {
        return QString::number(v + 1);
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<QString>>>::value));
    QCOMPARE(waitForValue(p, QVector<QString>()), QVector<QString>({"43", "44", "45"}));
}

void tst_helpers_map::delayedFulfilled()
{
    auto p = QtPromise::map(QVector<int>{42, 43, 44}, [](int v, ...) {
        return QPromise<int>([&](const QPromiseResolve<int>& resolve) {
                QtPromisePrivate::qtpromise_defer([=]() {
                    resolve(v + 1);
                });
            });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({43, 44, 45}));
}

void tst_helpers_map::delayedRejected()
{
    auto p = QtPromise::map(QVector<int>{42, 43, 44}, [](int v, ...) {
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

void tst_helpers_map::functorThrows()
{
    auto p = QtPromise::map(QVector<int>{42, 43, 44}, [](int v, ...) {
        if (v == 43) {
            throw QString("foo");
        }
        return v + 1;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_helpers_map::functorArguments()
{
    auto p = QtPromise::map(QVector<int>{42, 42, 42}, [](int v, int i) {
        return v * i;
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({0, 42, 84}));
}

void tst_helpers_map::preserveOrder()
{
    auto p = QtPromise::map(QVector<int>{500, 100, 250}, [](int v, ...) {
        return QPromise<int>::resolve(v + 1).delay(v);
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
    QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({501, 101, 251}));
}

void tst_helpers_map::sequenceTypes()
{
    SequenceTester<QList<int>>::exec();
    SequenceTester<QVector<int>>::exec();
    SequenceTester<std::list<int>>::exec();
    SequenceTester<std::vector<int>>::exec();
}
