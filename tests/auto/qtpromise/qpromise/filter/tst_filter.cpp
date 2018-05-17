// Tests
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_filter : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void normalList();
    void promiseList();
    void filterChained();
    void filterOutOfOrder();
};

QTEST_MAIN(tst_qpromise_filter)
#include "tst_filter.moc"

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
            QPromise<int>::resolve(42),
            QPromise<int>::resolve(43),
            QPromise<int>::resolve(44)
        };

        auto p = QPromise<int>::all(promises)
                .filter([](int value, int){
                    return value >= 43;
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({43, 44}));
    }

    static void execPromise()
    {
        Sequence<QPromise<int>, Args...> promises {
            QPromise<int>::resolve(42),
            QPromise<int>::resolve(43),
            QPromise<int>::resolve(44)
        };

        auto p = QPromise<int>::all(promises)
                .filter([](int value, int) {
                    return QPromise<bool>::resolve(value >= 43);
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({43, 44}));
    }

    static void execChained()
    {
        Sequence<QPromise<int>, Args...> promises {
            QPromise<int>::resolve(42),
            QPromise<int>::resolve(43),
            QPromise<int>::resolve(44),
            QPromise<int>::resolve(45)
        };

        auto p = QPromise<int>::all(promises)
                .filter([](int value, int) {
                    return QPromise<bool>::resolve(value >= 43);
                })
                .filter([](int value, int) {
                    return value <= 44;
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({43, 44}));
    }

    static void execOutOfOrder()
    {
        Sequence<QPromise<int>, Args...> promises {
            QPromise<int>::resolve(42),
            QPromise<int>::resolve(43),
            QPromise<int>::resolve(44),
            QPromise<int>::resolve(45)
        };

        int length = promises.length();
        auto p = QPromise<int>::all(promises)
                .filter([length](int value, int index) {
                    return QPromise<bool>::resolve(value >= 43).delay(100 * (length-index));
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({43, 44, 45}));
    }
};

} // anonymous namespace

void tst_qpromise_filter::normalList()
{
    SequenceTester<QList<QPromise<int>>>::execNormal();
}

void tst_qpromise_filter::promiseList()
{
    SequenceTester<QList<QPromise<int>>>::execPromise();
}

void tst_qpromise_filter::filterChained()
{
    SequenceTester<QList<QPromise<int>>>::execChained();
}

void tst_qpromise_filter::filterOutOfOrder()
{
    SequenceTester<QList<QPromise<int>>>::execOutOfOrder();
}
