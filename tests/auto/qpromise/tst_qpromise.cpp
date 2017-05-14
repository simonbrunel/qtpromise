// QtPromise
#include <qtpromise/qpromise.h>

// Qt
#include <QtTest>

using namespace QtPromise;

static const int ASYNC_DELAY = 256;

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
    {
        QPromise<int> p;
        QVector<int> values;
        auto next = p.finally([&values]() {
            values << 8;
            return 16;
        });

        p.fulfill(42);
        next.then([&values](int r) {
            values << r;
        }).wait();

        QVERIFY(p.isFulfilled());
        QVERIFY(next.isFulfilled());
        QCOMPARE(values, QVector<int>({8, 42}));
    }
    {
        QPromise<int> p;
        QVector<int> values;
        auto next = p.finally([&values]() {
            values << 8;
            return 16;
        });

        p.reject(QString("foo"));
        next.then([&values](int r) {
            values << r;
        }).wait();

        QVERIFY(p.isRejected());
        QVERIFY(next.isRejected());
        QCOMPARE(values, QVector<int>({8}));
    }
}

void tst_qpromise::finallyThrows()
{
    {
        QPromise<int> p;
        QString error;
        auto next = p.finally([]() {
            throw QString("bar");
        });

        p.fulfill(42);
        next.fail([&error](const QString& err) {
            error = err;
            return 0;
        }).wait();

        QVERIFY(p.isFulfilled());
        QVERIFY(next.isRejected());
        QCOMPARE(error, QString("bar"));
    }
    {
        QPromise<int> p;
        QString error;
        auto next = p.finally([]() {
            throw QString("bar");
        });

        p.reject(QString("foo"));
        next.fail([&error](const QString& err) {
            error = err;
            return 0;
        }).wait();

        QVERIFY(p.isRejected());
        QVERIFY(next.isRejected());
        QCOMPARE(error, QString("bar"));
    }
}

void tst_qpromise::finallyDelayedFulfilled()
{
    {
        QPromise<int> p0;
        QVector<int> values;
        auto next = p0.finally([&values]() {
            QPromise<int> p1;
            QTimer::singleShot(ASYNC_DELAY, [p1, &values]() mutable {
                values << 64;
                p1.fulfill(16);
            });

            values << 8;
            return p1;
        });

        p0.fulfill(42);
        next.then([&values](int r) {
            values << r;
        }).wait();

        QVERIFY(p0.isFulfilled());
        QVERIFY(next.isFulfilled());
        QCOMPARE(values, QVector<int>({8, 64, 42}));
    }
    {
        QPromise<int> p0;
        QVector<int> values;
        auto next = p0.finally([&values]() {
            QPromise<int> p1;
            QTimer::singleShot(ASYNC_DELAY, [p1, &values]() mutable {
                values << 64;
                p1.fulfill(16);
            });

            values << 8;
            return p1;
        });

        p0.reject(QString("foo"));
        next.then([&values](int r) {
            values << r;
        }).wait();

        QVERIFY(p0.isRejected());
        QVERIFY(next.isRejected());
        QCOMPARE(values, QVector<int>({8, 64}));
    }
}

void tst_qpromise::finallyDelayedRejected()
{
    {
        QPromise<int> p0;
        QString error;
        auto next = p0.finally([]() {
            QPromise<int> p1;
            QTimer::singleShot(ASYNC_DELAY, [p1]() mutable {
                p1.reject(QString("bar"));
            });

            return p1;
        });

        p0.fulfill(42);
        next.fail([&error](const QString& err) {
            error = err;
            return 0;
        }).wait();

        QVERIFY(p0.isFulfilled());
        QVERIFY(next.isRejected());
        QCOMPARE(error, QString("bar"));
    }
    {
        QPromise<int> p0;
        QString error;
        auto next = p0.finally([]() {
            QPromise<int> p1;
            QTimer::singleShot(ASYNC_DELAY, [p1]() mutable {
                p1.reject(QString("bar"));
            });

            return p1;
        });

        p0.reject(QString("foo"));
        next.fail([&error](const QString& err) {
            error = err;
            return 0;
        }).wait();

        QVERIFY(p0.isRejected());
        QVERIFY(next.isRejected());
        QCOMPARE(error, QString("bar"));
    }
}
