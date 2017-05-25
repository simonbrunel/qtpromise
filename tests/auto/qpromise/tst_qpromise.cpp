// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;
using namespace QtPromisePrivate;

class tst_qpromise: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void finallyReturns();
    void finallyThrows();
    void finallyDelayedFulfilled();
    void finallyDelayedRejected();

}; // class tst_qpromise

QTEST_MAIN(tst_qpromise)
#include "tst_qpromise.moc"

void tst_qpromise::finallyReturns()
{
    {   // fulfilled
        QVector<int> values;
        auto next = QPromise<int>::resolve(42).finally([&]() {
            values << 8;
            return 16; // ignored!
        });

        next.then([&](int r) {
            values << r;
        }).wait();

        QVERIFY(next.isFulfilled());
        QCOMPARE(values, QVector<int>({8, 42}));
    }
    {   // rejected
        QString error;
        int value = -1;
        auto next = QPromise<int>([](const QPromiseResolve<int>) {
            throw QString("foo");
        }).finally([&]() {
            value = 8;
            return 16; // ignored!
        });

        next.fail([&](const QString& err) {
            error = err;
            return 42;
        }).wait();

        QVERIFY(next.isRejected());
        QCOMPARE(error, QString("foo"));
        QCOMPARE(value, 8);
    }
}

void tst_qpromise::finallyThrows()
{
    {   // fulfilled
        QString error;
        auto next = QPromise<int>::resolve(42).finally([&]() {
            throw QString("bar");
        });

        next.fail([&](const QString& err) {
            error = err;
            return 0;
        }).wait();

        QVERIFY(next.isRejected());
        QCOMPARE(error, QString("bar"));
    }
    {   // rejected
        QString error;
        auto next = QPromise<int>::reject(QString("foo")).finally([&]() {
            throw QString("bar");
        });

        next.fail([&](const QString& err) {
            error = err;
            return 0;
        }).wait();

        QVERIFY(next.isRejected());
        QCOMPARE(error, QString("bar"));
    }
}

void tst_qpromise::finallyDelayedFulfilled()
{
    {   // fulfilled
        QVector<int> values;
        auto next = QPromise<int>::resolve(42).finally([&]() {
            QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
                qtpromise_defer([=, &values]() {
                    values << 64;
                    resolve(16); // ignored!
                });
            });

            values << 8;
            return p;
        });

        next.then([&](int r) {
            values << r;
        }).wait();

        QVERIFY(next.isFulfilled());
        QCOMPARE(values, QVector<int>({8, 64, 42}));
    }
    {   // rejected
        QString error;
        QVector<int> values;
        auto next = QPromise<int>::reject(QString("foo")).finally([&]() {
            QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
                qtpromise_defer([=, &values]() {
                    values << 64;
                    resolve(16); // ignored!
                });
            });

            values << 8;
            return p;
        });

        next.then([&](int r) {
            values << r;
        }, [&](const QString& err) {
            error = err;
        }).wait();

        QVERIFY(next.isRejected());
        QCOMPARE(error, QString("foo"));
        QCOMPARE(values, QVector<int>({8, 64}));
    }
}

void tst_qpromise::finallyDelayedRejected()
{
    {   // fulfilled
        QString error;
        auto next = QPromise<int>::resolve(42).finally([]() {
            return QPromise<int>([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
                qtpromise_defer([=]() {
                    reject(QString("bar"));
                });
            });
        });

        next.fail([&](const QString& err) {
            error = err;
            return 0;
        }).wait();

        QVERIFY(next.isRejected());
        QCOMPARE(error, QString("bar"));
    }
    {   // rejected
        QString error;
        auto next = QPromise<int>::reject(QString("foo")).finally([]() {
            return QPromise<int>([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
                qtpromise_defer([=]() {
                    reject(QString("bar"));
                });
            });
        });

        next.fail([&](const QString& err) {
            error = err;
            return 0;
        }).wait();

        QVERIFY(next.isRejected());
        QCOMPARE(error, QString("bar"));
    }
}
