// Tests
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_reduce : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void normalList();
    void promiseList();
};

QTEST_MAIN(tst_qpromise_reduce)
#include "tst_reduce.moc"

namespace {

template <class Sequence>
struct SequenceTester
{
};

template <template <typename, typename...> class Sequence, typename ...Args>
struct SequenceTester<Sequence<QPromise<int>, Args...>>
{
    static void execNormal()
    {
        Sequence<QPromise<int>, Args...> promises {
            QPromise<int>::resolve(2),
            QPromise<int>::resolve(4),
            QPromise<int>::resolve(8)
        };

        auto p = QPromise<int>::all(promises)
                .reduce([](int value, int current, int index) {
                    Q_UNUSED(index);
                    return value + current;
                }, 0);

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
        QCOMPARE(waitForValue(p, int()), 14);
    }

    static void execPromise()
    {
        Sequence<QPromise<int>, Args...> promises {
            QPromise<int>::resolve(2),
            QPromise<int>::resolve(4),
            QPromise<int>::resolve(8)
        };

        auto p = QPromise<int>::all(promises)
                .reduce([](int value, int current, int index) {
                    Q_UNUSED(index);
                    return QPromise<int>::resolve(value + current);
                }, 1);

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
        QCOMPARE(waitForValue(p, int()), 15);
    }
};

} // anonymous namespace

void tst_qpromise_reduce::normalList()
{
    SequenceTester<QList<QPromise<int>>>::execNormal();
}

void tst_qpromise_reduce::promiseList()
{
    SequenceTester<QList<QPromise<int>>>::execPromise();
}

