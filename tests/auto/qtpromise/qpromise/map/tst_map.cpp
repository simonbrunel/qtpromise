// Tests
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_map : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void normalList();
    void promiseList();
    void changeType();
    void mapChained();
};

QTEST_MAIN(tst_qpromise_map)
#include "tst_map.moc"

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
                .map([](int value, int){
                    return value + 1;
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({43, 44, 45}));
    }

    static void execPromise()
    {
        Sequence<QPromise<int>, Args...> promises {
            QPromise<int>::resolve(42),
            QPromise<int>::resolve(43),
            QPromise<int>::resolve(44)
        };

        auto p = QPromise<int>::all(promises)
                .map([](int value, int) {
                    return QPromise<int>::resolve(value + 1);
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({43, 44, 45}));
    }

    static void execChangeType()
    {
        /*
        Sequence<QPromise<int>, Args...> promises {
            QPromise<int>::resolve(42),
            QPromise<int>::resolve(43)
        };

        auto p = QPromise<int>::all(promises)
                .map([](int value, int) {
                    return QString::number(value);
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<QString>>>::value));
        QCOMPARE(waitForValue(p, QVector<QString>()), QVector<QString>({QLatin1String("42"), QLatin1String("43")}));
        */
    }

    static void execChained()
    {
        Sequence<QPromise<int>, Args...> promises {
            QPromise<int>::resolve(42),
            QPromise<int>::resolve(43),
            QPromise<int>::resolve(44)
        };

        auto p = QPromise<int>::all(promises)
                .map([](int value, int) {
                    return QPromise<int>::resolve(value + 1);
                })
                .map([](int value, int) {

                    return value + 2;
                });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QVector<int>>>::value));
        QCOMPARE(waitForValue(p, QVector<int>()), QVector<int>({45, 46, 47}));
    }
};

} // anonymous namespace

void tst_qpromise_map::normalList()
{
    SequenceTester<QList<QPromise<int>>>::execNormal();
}

void tst_qpromise_map::promiseList()
{
    SequenceTester<QList<QPromise<int>>>::execPromise();
}

void tst_qpromise_map::changeType()
{
    SequenceTester<QList<QPromise<int>>>::execChangeType();
}

void tst_qpromise_map::mapChained()
{
    SequenceTester<QList<QPromise<int>>>::execChained();
}
