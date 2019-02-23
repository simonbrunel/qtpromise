#include "../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtConcurrent>
#include <QtTest>

using namespace QtPromise;

class tst_exceptions : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void canceled();
    void timeout();
    void undefined();

}; // class tst_exceptions

QTEST_MAIN(tst_exceptions)
#include "tst_exceptions.moc"

namespace {

template <class E>
void verify()
{
    auto p = qPromise(QtConcurrent::run([]() { throw E(); }));
    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForRejected<E>(p), true);
    QCOMPARE(p.isRejected(), true);
}

} // anonymous namespace

void tst_exceptions::canceled()
{
    verify<QPromiseCanceledException>();
}

void tst_exceptions::timeout()
{
    verify<QPromiseTimeoutException>();
}

void tst_exceptions::undefined()
{
    verify<QPromiseUndefinedException>();
}
