/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 * Copyright (c) Dmitriy Purgin, https://github.com/dpurgin
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#include "../shared/utils.h"

#include <QtPromise>
#include <QtTest>

#include <chrono>

using namespace QtPromise;

class tst_qpromise_as : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fulfillIntAsReal();
    void fulfillRealAsInt();
    void fulfillIntAsVoid();
    void fulfillQVariantAsInt();
    void fulfillQVariantQStringAsInt();
    void fulfillQByteArrayAsQString();

    void rejectUnconvertibleQVariant();
};

QTEST_MAIN(tst_qpromise_as)
#include "tst_as.moc"

void tst_qpromise_as::fulfillIntAsReal()
{
    auto p = QtPromise::resolve(42).as<qreal>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<qreal>>::value));

    QCOMPARE(waitForValue(p, -1.0), 42.0);
    QVERIFY(p.isFulfilled());
}

void tst_qpromise_as::fulfillRealAsInt()
{
    auto p = QtPromise::resolve(42.13).as<int>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));

    QCOMPARE(waitForValue(p, -1), 42);
    QVERIFY(p.isFulfilled());
}

void tst_qpromise_as::fulfillIntAsVoid()
{
    auto p = QtPromise::resolve(42).as<void>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));

    QCOMPARE(waitForValue(p, -1, 42), 42);
    QVERIFY(p.isFulfilled());
}

void tst_qpromise_as::fulfillQVariantAsInt()
{
    auto p = QtPromise::resolve(QVariant{42}).as<int>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));

    QCOMPARE(waitForValue(p, -1), 42);
    QVERIFY(p.isFulfilled());
}

void tst_qpromise_as::fulfillQVariantQStringAsInt()
{
    auto p = QtPromise::resolve(QVariant{"42"}).as<int>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));

    QCOMPARE(waitForValue(p, -1), 42);
    QVERIFY(p.isFulfilled());
}

void tst_qpromise_as::fulfillQByteArrayAsQString()
{
    auto p = QtPromise::resolve(QByteArray{"foo"}).as<QString>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));

    QCOMPARE(waitForValue(p, {}), QString{"foo"});
    QVERIFY(p.isFulfilled());
}

void tst_qpromise_as::rejectUnconvertibleQVariant()
{
    auto p = QtPromise::resolve(QVariant{"42foo"}).as<int>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));

    QVERIFY(waitForRejected<QPromiseConversionException>(p));
}
