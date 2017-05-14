// QtPromise
#include <qtpromise/qpromise.h>

// Qt
#include <QtTest>

using namespace QtPromise;

// https://promisesaplus.com/#requirements
class tst_requirements: public QObject
{
    Q_OBJECT

private Q_SLOTS:

    // 2.1. Promise States
    void statePending();        // 2.1.1.
    void stateFulfilled();      // 2.1.2.
    void stateRejected();       // 2.1.3.

    // 2.2. The then Method
    void thenArguments();       // 2.2.1.
    void thenOnFulfilled();     // 2.2.2.
    void thenOnRejected();      // 2.2.3.
    void thenAsynchronous();    // 2.2.4.
                                // 2.2.5. NOT APPLICABLE
    void thenMultipleCalls();   // 2.2.6.
    void thenHandlers();        // 2.2.7.

    // 2.3. The Promise Resolution Procedure

}; // class tst_requirements

QTEST_MAIN(tst_requirements)
#include "tst_requirements.moc"

void tst_requirements::statePending()
{
    // 2.1.1. When pending, a promise:
    {
        QPromise<> p;
        QVERIFY(p.isPending());
        QVERIFY(!p.isFulfilled());
        QVERIFY(!p.isRejected());
    }

    // 2.1.1.1. may transition to either the fulfilled state
    {
        QPromise<> p;
        p.fulfill();
        QVERIFY(!p.isPending());
        QVERIFY(p.isFulfilled());
    }

    // 2.1.1.1. ... or the rejected state
    {
        QPromise<> p;
        p.reject("foo");
        QVERIFY(!p.isPending());
        QVERIFY(p.isRejected());
    }
}

void tst_requirements::stateFulfilled()
{
    QVector<int> values;
    auto log_value = [&values](int res) { values << res; };

    QPromise<int> p;
    QVERIFY(p.isPending());

    // 2.1.2. When fulfilled, a promise:
    p.fulfill(42).then(log_value).wait();
    QVERIFY(p.isFulfilled());
    QVERIFY(!p.isRejected());

    // 2.1.2.1. must not transition to any other state.
    p.reject("foo").then(log_value).wait();
    QVERIFY(p.isFulfilled());
    QVERIFY(!p.isRejected());

    // 2.1.2.2. must have a value, which must not change.
    p.fulfill(51).then(log_value).wait();
    QVERIFY(p.isFulfilled());
    QVERIFY(!p.isRejected());

    QCOMPARE(values, QVector<int>({42, 42, 42}));
}

void tst_requirements::stateRejected()
{
    QStringList errors;
    auto log_error = [&errors](const QString& err) { errors << err; return -1; };

    QPromise<int> p;
    QVERIFY(p.isPending());

    // 2.1.3 When rejected, a promise:
    p.reject(QString("foo")).then(nullptr, log_error).wait();
    QVERIFY(!p.isFulfilled());
    QVERIFY(p.isRejected());

    // 2.1.3.1. must not transition to any other state.
    p.fulfill(51).then(nullptr, log_error).wait();
    QVERIFY(!p.isFulfilled());
    QVERIFY(p.isRejected());

    // 2.1.3.2. must have a reason, which must not change.
    p.reject(QString("bar")).then(nullptr, log_error).wait();
    QVERIFY(!p.isFulfilled());
    QVERIFY(p.isRejected());

    QCOMPARE(errors, QStringList({"foo", "foo", "foo"}));
}

void tst_requirements::thenArguments()
{
    // Both onFulfilled and onRejected are given
    {
        int value = 0;
        QPromise<int> p;
        p.fulfill(42).then([&value](int res) { value = res; }, [](const QString&){}).wait();
        QVERIFY(p.isFulfilled());
        QCOMPARE(value, 42);
    }
    {
        QString error;
        QPromise<int> p;
        p.reject(QString("foo")).then([](int) {}, [&error](const QString& err){ error = err; }).wait();
        QVERIFY(p.isRejected());
        QCOMPARE(error, QString("foo"));
    }

    // 2.2.1. onFulfilled is an optional arguments:
    {
        QString error;
        QPromise<int> p;
        p.reject(QString("foo")).then(nullptr, [&error](const QString& err){ error = err; return 42; }).wait();
        QVERIFY(p.isRejected());
        QCOMPARE(error, QString("foo"));
    }

    // 2.2.1. onRejected is an optional arguments:
    {
        int value = 0;
        QPromise<int> p;
        p.fulfill(42).then([&value](int res) { value = res; }).wait();
        QVERIFY(p.isFulfilled());
        QCOMPARE(value, 42);
    }

    // 2.2.1.1. If onFulfilled is not a function, it must be ignored.
    // 2.2.1.2. If onRejected is not a function, it must be ignored.
    // -> compilation fails on type check.
}

void tst_requirements::thenOnFulfilled()
{
    // 2.2.2. If onFulfilled is a function:

    QPromise<int> p0;
    QVector<int> values;
    auto p1 = p0.then([&values](int res) { values << res; }); // @TODO .wait(10)

    // 2.2.2.2. it must not be called before promise is fulfilled.
    QVERIFY(p0.isPending());
    QVERIFY(p1.isPending());
    QVERIFY(values.isEmpty());

    // 2.2.2.1. it must be called after promise is fulfilled,
    // with promise’s value as its first argument.
    p0.fulfill(42);
    p1.wait();
    QVERIFY(p0.isFulfilled());
    QVERIFY(p1.isFulfilled());
    QCOMPARE(values, QVector<int>({42}));

    // 2.2.2.3. it must not be called more than once
    p0.fulfill(12);
    p1.wait();
    QCOMPARE(values, QVector<int>({42}));
}

void tst_requirements::thenOnRejected()
{
    // 2.2.3. If onRejected is a function:

    QPromise<> p0;
    QStringList errors;
    auto p1 = p0.then(nullptr, [&errors](const QString& err) { errors << err; }); // @TODO .wait(10)

    // 2.2.3.2. it must not be called before promise is rejected.
    QVERIFY(p0.isPending());
    QVERIFY(p1.isPending());
    QVERIFY(errors.isEmpty());

    // 2.2.3.1. it must be called after promise is rejected,
    // with promise’s reason as its first argument.
    p0.reject(QString("foo"));
    p1.wait();
    QVERIFY(p0.isRejected());
    QVERIFY(p1.isFulfilled());
    QCOMPARE(errors, QStringList({"foo"}));

    // 2.2.2.3. it must not be called more than once.
    p0.reject(12);
    p1.wait();
    QCOMPARE(errors, QStringList({"foo"}));
}

void tst_requirements::thenAsynchronous()
{
    // 2.2.4. onFulfilled or onRejected must not be called until the execution context
    // stack contains only platform code (ie. executed asynchronously, after the event
    // loop turn in which then is called, and with a fresh stack).

    int value = 0;
    QPromise<int> p0;
    p0.fulfill(42);
    QVERIFY(p0.isFulfilled());

    auto p1 = p0.then([&value](int res){ value = res; });
    QVERIFY(p1.isPending());
    QCOMPARE(value, 0);

    p1.wait();
    QVERIFY(p1.isFulfilled());
    QCOMPARE(value, 42);
}

void tst_requirements::thenMultipleCalls()
{
    // 2.2.6. then may be called multiple times on the same promise:

    // 2.2.6.1. If/when promise is fulfilled, all respective onFulfilled callbacks
    // must execute in the order of their originating calls to then:
    {
        QPromise<int> p;
        QVector<int> values;
        auto all = qPromiseAll(QVector<QPromise<void> >{
            p.then([&values](int r) { values << r + 1; }),
            p.then([&values](int r) { values << r + 2; }),
            p.then([&values](int r) { values << r + 3; })
        });

        p.fulfill(42);
        all.wait();
        QCOMPARE(values, QVector<int>({43, 44, 45}));
    }

    // 2.2.6.2. If/when promise is rejected, all respective onRejected callbacks
    // must execute in the order of their originating calls to then:
    {
        QPromise<int> p;
        QVector<int> values;
        auto all = qPromiseAll(QVector<QPromise<int> >{
            p.then(nullptr, [&values](int r) { values << r + 1; return r + 1; }),
            p.then(nullptr, [&values](int r) { values << r + 2; return r + 2; }),
            p.then(nullptr, [&values](int r) { values << r + 3; return r + 3; })
        });

        p.reject(8);
        all.wait();
        QCOMPARE(values, QVector<int>({9, 10, 11}));
    }
}

void tst_requirements::thenHandlers()
{
    // 2.2.7. then must return a promise: p2 = p1.then(onFulfilled, onRejected);
    {
        QPromise<int> p1;
        auto handler = [](){ return 42; };
        Q_STATIC_ASSERT((std::is_same<decltype(p1.then(handler, nullptr)), QPromise<int> >::value));
        Q_STATIC_ASSERT((std::is_same<decltype(p1.then(nullptr, handler)), QPromise<int> >::value));
        Q_STATIC_ASSERT((std::is_same<decltype(p1.then(handler, handler)), QPromise<int> >::value));
    }

    // 2.2.7.1. If either onFulfilled or onRejected returns a value x, run the
    // Promise Resolution Procedure [[Resolve]](p2, x) -> See 2.3.

    // 2.2.7.2. If either onFulfilled or onRejected throws an exception e,
    // p2 must be rejected with e as the reason.
    {
        QString reason;
        QPromise<int> p1;
        auto p2 = p1.fulfill(42).then([](){ throw QString("foo"); });
        p2.then(nullptr, [&reason](const QString& e) { reason = e; }).wait();

        QVERIFY(p2.isRejected());
        QCOMPARE(reason, QString("foo"));
    }
    {
        QString reason;
        QPromise<int> p1;
        auto p2 = p1.reject(QString("foo")).then(nullptr, [](){ throw QString("bar"); return 42; });
        p2.then(nullptr, [&reason](const QString& e) { reason = e; return 0; }).wait();

        QVERIFY(p2.isRejected());
        QCOMPARE(reason, QString("bar"));
    }

    // 2.2.7.3. If onFulfilled is not a function and promise1 is fulfilled,
    // p2 must be fulfilled with the same value as promise1
    {
        QString value;
        QPromise<QString> p1;
        auto p2 = p1.fulfill("42").then(nullptr, [](){ return QString(); });
        Q_STATIC_ASSERT((std::is_same<decltype(p2), QPromise<QString> >::value));
        p2.then([&value](const QString& e) { value = e; }).wait();

        QVERIFY(p2.isFulfilled());
        QCOMPARE(value, QString("42"));
    }
}
