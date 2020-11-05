/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
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
    void fulfillTAsU();
    void fulfillTAsVoid();
    void fulfillTAsQVariant();
    void fulfillQVariantAsT();
    void fulfillQVariantAsVoid();
    void fulfillVoidAsQVariant();

    void rejectUnconvertibleQVariant();
};

QTEST_MAIN(tst_qpromise_as)
#include "tst_as.moc"

namespace {
struct Foo
{
    Foo() = default;
    Foo(int bar_) : bar{bar_} { }

    int bar{-1};
};

bool operator==(const Foo& lhs, const Foo& rhs)
{
    return lhs.bar == rhs.bar;
}
} // namespace

Q_DECLARE_METATYPE(Foo)

void tst_qpromise_as::fulfillTAsU()
{
    // Static cast between primitive types
    {
        auto p = QtPromise::resolve(42.13).as<int>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));

        QCOMPARE(waitForValue(p, -1), 42);
        QVERIFY(p.isFulfilled());
    }

    // Converting constructor
    // https://en.cppreference.com/w/cpp/language/converting_constructor
    {
        auto p = QtPromise::resolve(QByteArray{"foo"}).as<QString>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));

        QCOMPARE(waitForValue(p, QString{}), QString{"foo"});
        QVERIFY(p.isFulfilled());
    }
}

void tst_qpromise_as::fulfillTAsVoid()
{
    auto p = QtPromise::resolve(42).as<void>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));

    QCOMPARE(waitForValue(p, -1, 42), 42);
    QVERIFY(p.isFulfilled());
}

void tst_qpromise_as::fulfillTAsQVariant()
{
    // Primitive type to QVariant
    {
        auto p = QtPromise::resolve(42).as<QVariant>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVariant>>::value));

        QCOMPARE(waitForValue(p, QVariant{}), QVariant{42});
        QVERIFY(p.isFulfilled());
    }

    // Non-Qt user-defined type to QVariant
    {
        auto p = QtPromise::resolve(Foo{42}).as<QVariant>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVariant>>::value));

        QVariant value = waitForValue(p, QVariant{});
        QCOMPARE(value, QVariant::fromValue(Foo{42}));
        QCOMPARE(value.value<Foo>().bar, 42);
        QVERIFY(p.isFulfilled());
    }
}

void tst_qpromise_as::fulfillQVariantAsT()
{
    // Test whether a directly stored value can be extracted
    {
        auto p = QtPromise::resolve(QVariant{42}).as<int>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));

        QCOMPARE(waitForValue(p, -1), 42);
        QVERIFY(p.isFulfilled());
    }

    // Test automatic conversion from string performed by QVariant
    // https://doc.qt.io/qt-5/qvariant.html#toInt
    {
        auto p = QtPromise::resolve(QVariant{"42"}).as<int>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));

        QCOMPARE(waitForValue(p, -1), 42);
        QVERIFY(p.isFulfilled());
    }

    // Non-Qt user-defined type
    {
        auto p = QtPromise::resolve(QVariant::fromValue(Foo{42})).as<Foo>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Foo>>::value));

        QCOMPARE(waitForValue(p, Foo{}), Foo{42});
        QVERIFY(p.isFulfilled());
    }
}

void tst_qpromise_as::fulfillQVariantAsVoid()
{
    auto p = QtPromise::resolve(QVariant{42}).as<void>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));

    QCOMPARE(waitForValue(p, -1, 42), 42);
    QVERIFY(p.isFulfilled());
}

void tst_qpromise_as::fulfillVoidAsQVariant()
{
    auto p = QtPromise::resolve().as<QVariant>();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVariant>>::value));

    QCOMPARE(waitForValue(p, QVariant{42}), QVariant{});
    QVERIFY(p.isFulfilled());
}

void tst_qpromise_as::rejectUnconvertibleQVariant()
{
    // User-defined type incompatible with int
    {
        auto p = QtPromise::resolve(QVariant{"42foo"}).as<int>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));

        QVERIFY(waitForRejected<QPromiseConversionException>(p));
    }

    // Incompatible user-defined types
    {
        auto p = QtPromise::resolve(QVariant::fromValue(Foo{42})).as<QString>();

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));

        QVERIFY(waitForRejected<QPromiseConversionException>(p));
    }
}
