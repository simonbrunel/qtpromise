// Tests
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

// STL
#include <memory>

using namespace QtPromise;

class tst_helpers_resolve : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void resolveWithValue();
    void resolveWithNoValue();
    void resolveWithTypedPromise();
    void resolveWithVoidPromise();
    void resolveWithQSharedPtr();
    void resolveWithStdSharedPtr();
};

QTEST_MAIN(tst_helpers_resolve)
#include "tst_resolve.moc"

void tst_helpers_resolve::resolveWithValue()
{
    const int value = 42;
    auto p0 = QPromise<int>::resolve(value);
    auto p1 = QPromise<int>::resolve(43);

    Q_STATIC_ASSERT((std::is_same<decltype(p0), QPromise<int>>::value));
    Q_STATIC_ASSERT((std::is_same<decltype(p1), QPromise<int>>::value));
    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(waitForValue(p0, -1), 42);
    QCOMPARE(waitForValue(p1, -1), 43);
}

void tst_helpers_resolve::resolveWithNoValue()
{
    auto p = QPromise<void>::resolve();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QCOMPARE(p.isFulfilled(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
}

void tst_helpers_resolve::resolveWithTypedPromise()
{
    auto p = QtPromise::qPromise(
        QPromise<QString>([](const QPromiseResolve<QString>& resolve) {
            QtPromisePrivate::qtpromise_defer([=](){
                resolve("foo");
            });
        }));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, QString()), QString("foo"));
}

void tst_helpers_resolve::resolveWithVoidPromise()
{
    int check;
    auto p = QtPromise::qPromise(
        QPromise<void>([&](const QPromiseResolve<void>& resolve) {
            QtPromisePrivate::qtpromise_defer([=, &check](){
                check = 8;
                resolve();
            });
        }));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1, 42), 42);
    QCOMPARE(check, 8);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_helpers_resolve::resolveWithQSharedPtr()
{
    QWeakPointer<int> wptr;

    {
        QSharedPointer<int> sptr(new int(42));
        auto p = QPromise<QSharedPointer<int>>::resolve(sptr);

        QCOMPARE(waitForValue(p, QSharedPointer<int>()), sptr);

        wptr = sptr;
        sptr.reset();

        QCOMPARE(wptr.isNull(), false); // "p" still holds a reference
    }

    QCOMPARE(wptr.isNull(), true);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_helpers_resolve::resolveWithStdSharedPtr()
{
    std::weak_ptr<int> wptr;

    {
        std::shared_ptr<int> sptr(new int(42));
        auto p = QPromise<std::shared_ptr<int>>::resolve(sptr);

        QCOMPARE(waitForValue(p, std::shared_ptr<int>()), sptr);

        wptr = sptr;
        sptr.reset();

        QCOMPARE(wptr.use_count(), 1l); // "p" still holds a reference
    }

    QCOMPARE(wptr.use_count(), 0l);
}
