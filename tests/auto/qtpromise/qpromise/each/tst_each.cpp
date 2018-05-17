// Tests
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_each : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void normalList();
    void promiseList();
    void mapChained();
    void mapOutOfOrder();
};

QTEST_MAIN(tst_qpromise_each)
#include "tst_each.moc"

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

        QSharedPointer<QVector<int>> hits(new QVector<int>());
        auto p = QPromise<int>::all(promises)
                .each([hits](int value, int index){
                    hits->push_back(value + index);
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
        QCOMPARE(*hits, QVector<int>({42, 44, 46}));
    }

    static void execPromise()
    {
        Sequence<QPromise<int>, Args...> promises {
            QPromise<int>::resolve(42),
            QPromise<int>::resolve(43),
            QPromise<int>::resolve(44)
        };

        QSharedPointer<QVector<int>> hits(new QVector<int>());
        auto p = QPromise<int>::all(promises)
                .each([hits](int value, int index) {
                    QPromise<int>::resolve(value).then([hits,index](int value){
                        hits->push_back(value + index);
                    });
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
        QCOMPARE(*hits, QVector<int>({42, 44, 46}));
    }

    static void execChained()
    {
        Sequence<QPromise<int>, Args...> promises {
            QPromise<int>::resolve(42),
            QPromise<int>::resolve(43),
            QPromise<int>::resolve(44)
        };

        QSharedPointer<QVector<int>> hits1(new QVector<int>());
        QSharedPointer<QVector<int>> hits2(new QVector<int>());
        auto p = QPromise<int>::all(promises)
                .each([hits1](int value, int index) {
                    return QPromise<int>::resolve(value)
                            .then([hits1,index](int value){
                        hits1->push_back(value + index);
                    });
                })
                .each([hits2](int value, int index) {
                    hits2->push_back(value + index);
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
        QCOMPARE(*hits1, QVector<int>({42, 44, 46}));
        QCOMPARE(*hits2, QVector<int>({42, 44, 46}));
    }

    static void execOutOfOrder()
    {
        Sequence<QPromise<int>, Args...> promises {
            QPromise<int>::resolve(42),
            QPromise<int>::resolve(43),
            QPromise<int>::resolve(44)
        };

        QSharedPointer<QVector<int>> hits(new QVector<int>());
        int length = promises.length();
        auto p = QPromise<int>::all(promises)
                .each([length, hits](int value, int index) {
                    return QPromise<int>::resolve(value + 1)
                    .delay(100 * (length-index))
                    .then([hits,index](int value){
                        hits->push_back(value + index);
                    });
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({42, 43, 44}));
    }
};

} // anonymous namespace

void tst_qpromise_each::normalList()
{
    SequenceTester<QList<QPromise<int>>>::execNormal();
}

void tst_qpromise_each::promiseList()
{
    SequenceTester<QList<QPromise<int>>>::execPromise();
}

void tst_qpromise_each::mapChained()
{
    SequenceTester<QList<QPromise<int>>>::execChained();
}

void tst_qpromise_each::mapOutOfOrder()
{
    SequenceTester<QList<QPromise<int>>>::execOutOfOrder();
}
