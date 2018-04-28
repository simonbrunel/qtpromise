// Tests
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

// STL
#include <memory>

using namespace QtPromise;

class tst_qpromise_resolve : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void resolveWithValue();
    void resolveWithNoValue();
    void resolveWithQSharedPtr();
    void resolveWithStdSharedPtr();
};

QTEST_MAIN(tst_qpromise_resolve)
#include "tst_resolve.moc"

void tst_qpromise_resolve::resolveWithValue()
{
    const int value = 42;
    auto p0 = QPromise<int>::resolve(value);
    auto p1 = QPromise<int>::resolve(43);

    QCOMPARE(p0.isFulfilled(), true);
    QCOMPARE(p1.isFulfilled(), true);
    QCOMPARE(waitForValue(p0, -1), 42);
    QCOMPARE(waitForValue(p1, -1), 43);
}

void tst_qpromise_resolve::resolveWithNoValue()
{
    auto p = QPromise<void>::resolve();

    QCOMPARE(p.isFulfilled(), true);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_qpromise_resolve::resolveWithQSharedPtr()
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
void tst_qpromise_resolve::resolveWithStdSharedPtr()
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
