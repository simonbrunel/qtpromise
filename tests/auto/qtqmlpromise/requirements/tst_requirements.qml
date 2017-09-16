import QtQuick 2.3
import QtPromise 1.0
import QtTest 1.0

// https://promisesaplus.com/#requirements
TestCase {
    name: "Requirements"

    // 2.1. Promise States

    // 2.1.1 When pending, a promise:
    function test_pendingFulfilled() {
        // 2.1.1.1. may transition to either the fulfilled state
        var p = new Promise(function(resolve) {
            Qt.callLater(function() {
                resolve(42);
            });
        });

        compare(p.isPending(), true);
        compare(p.isFulfilled(), false);
        compare(p.isRejected(), false);

        p.wait();

        compare(p.isPending(), false);
        compare(p.isFulfilled(), true);
        compare(p.isRejected(), false);
    }

    // 2.1.1 When pending, a promise:
    function test_pendingRejected() {
        // 2.1.1.1. ... or the rejected state
        var p = new Promise(function(resolve, reject) {
            Qt.callLater(function() {
                reject(new Error(42));
            });
        });

        compare(p.isPending(), true);
        compare(p.isFulfilled(), false);
        compare(p.isRejected(), false);

        p.wait();

        compare(p.isPending(), false);
        compare(p.isFulfilled(), false);
        compare(p.isRejected(), true);
    }

    // 2.1.2. When fulfilled, a promise:
    function test_fulfilled() {
        var p = new Promise(function(resolve, reject) {
            Qt.callLater(function() {
                // 2.1.2.2. must have a value, which must not change.
                resolve(42);
                resolve(43);

                // 2.1.2.1. must not transition to any other state.
                reject(new Error("foo"));
            });
        });

        compare(p.isPending(), true);

        var value = -1;
        var error = null;

        p.then(function(res) {
            value = res;
        }, function(err) {
            error = err;
        }).wait();

        compare(p.isFulfilled(), true);
        compare(p.isRejected(), false);
        compare(error, null);
        compare(value, 42);
    }

    // 2.1.3 When rejected, a promise:
    function test_rejected() {
        var p = new Promise(function(resolve, reject) {
            Qt.callLater(function() {
                // 2.1.3.2. must have a reason, which must not change.
                reject(new Error("foo"));
                reject(new Error("bar"));

                // 2.1.3.1. must not transition to any other state.
                resolve(42);
            });
        });

        compare(p.isPending(), true);

        var value = -1;
        var error = null;

        p.then(function(res) {
            value = res;
        }, function(err) {
            error = err;
        }).wait();

        compare(p.isFulfilled(), false);
        compare(p.isRejected(), true);
        compare(error instanceof Error, true);
        compare(error.message, 'foo');
        compare(value, -1);
    }

    // 2.2. The then Method

    // 2.2.1. Both onFulfilled and onRejected are given (resolve)
    function test_thenArgsBothResolve() {
        var error = null;
        var value = -1;

        Promise.resolve(42).then(
            function(res) { value = res; },
            function(err) { error = err; }
        ).wait();

        compare(error, null);
        compare(value, 42);
    }

    // 2.2.1. Both onFulfilled and onRejected are given (reject)
    function test_thenArgsBothReject() {
        var error = null;
        var value = -1;

        Promise.reject(new Error('foo')).then(
            function(res) { value = res; },
            function(err) { error = err; }
        ).wait();

        compare(error instanceof Error, true);
        compare(error.message, 'foo');
        compare(value, -1);
    }

    // 2.2.1. onFulfilled is an optional arguments:
    function test_thenArgsSecond() {
        var error = null;

        Promise.reject(new Error('foo')).then(
            null,
            function(err) { error = err; }
        ).wait();

        compare(error instanceof Error, true);
        compare(error.message, 'foo');
    }

    // 2.2.1. onRejected is an optional arguments:
    function test_thenArgsFirst() {
        var value = -1;

        Promise.resolve(42).then(
            function(res) { value = res; }
        ).wait();

        compare(value, 42);
    }

    // 2.2.1.1. If onFulfilled is not a function, it must be ignored.
    function test_thenArgsFirstInvalid() {
        var value = -1;

        Promise.resolve(42).then('invalid').then(
            function(res) { value = res; }
        ).wait();

        compare(value, 42);
    }

    // 2.2.1.2. If onRejected is not a function, it must be ignored.
    function test_thenArgsSecondInvalid() {
        var error = -1;

        Promise.reject(new Error('foo')).then(
            null,
            'invalid'
        ).then(
            null,
            function(err) { error = err; }
        ).wait();

        compare(error instanceof Error, true);
        compare(error.message, 'foo');
    }

    // 2.2.2. If onFulfilled is a function:
    function test_thenOnFulfilled() {
        var p0 = new Promise(function(resolve) {
            Qt.callLater(function() {
                // 2.2.2.3. it must not be called more than once
                resolve(42);
                resolve(43);
            });
        });

        var values = [];
        var p1 = p0.then(function(res) {
            values.push(res);
        });

        // 2.2.2.2. it must not be called before promise is fulfilled.
        compare(p0.isPending(), true);
        compare(p1.isPending(), true);
        compare(values.length, 0);

        p1.wait();

        // 2.2.2.1. it must be called after promise is fulfilled,
        // with promise’s value as its first argument.
        compare(p0.isFulfilled(), true);
        compare(p1.isFulfilled(), true);
        compare(values, [42]);
    }

    // 2.2.3. If onRejected is a function:
    function test_thenOnRejected() {
        var p0 = new Promise(function(resolve, reject) {
            Qt.callLater(function() {
                // 2.2.2.3. it must not be called more than once
                reject(new Error('foo'));
                reject(new Error('bar'));
            });
        });

        var errors = [];
        var p1 = p0.then(null, function(res) {
            errors.push(res);
        });

        // 2.2.3.2. it must not be called before promise is rejected.
        compare(p0.isPending(), true);
        compare(p1.isPending(), true);
        compare(errors.length, 0);

        p1.wait();

        // 2.2.3.1. it must be called after promise is rejected,
        // with promise’s reason as its first argument.
        compare(p0.isRejected(), true);
        compare(p1.isFulfilled(), true);
        compare(errors.length, 1);
        compare(errors[0] instanceof Error, true);
        compare(errors[0].message, 'foo');
    }

    // 2.2.4. onFulfilled or onRejected must not be called until the execution context
    // stack contains only platform code (ie. executed asynchronously, after the event
    // loop turn in which then is called, and with a fresh stack).
    function test_thenAsynchronous()
    {
        var value = -1;
        var p0 = Promise.resolve(42);
        var p1 = p0.then(function(res){ value = res; });

        compare(p0.isFulfilled(), true);
        compare(p1.isPending(), true);
        compare(value, -1);

        p1.wait();

        compare(p1.isFulfilled(), true);
        compare(value, 42);
    }

    // 2.2.5 onFulfilled and onRejected must be called as functions (i.e. with no this value).
    function test_thenThisArg() {
        var scopes = [];

        Promise.resolve(42).then(function() { scopes.push(this); }).wait();
        Promise.reject(new Error('foo')).then(null, function() { scopes.push(this); }).wait();

        // Qt doesn't allow to call JS function with undefined "this"
        // Let's adopt the sloppy mode (this === the global object).
        var global = (function() { return this; })();
        compare(scopes, [global, global]);
    }

    // 2.2.6. then may be called multiple times on the same promise:

    // 2.2.6.1. If/when promise is fulfilled, all respective onFulfilled callbacks
    // must execute in the order of their originating calls to then:
    function test_thenMultipleCalls() {
        var values = [];
        var p = new Promise(function(resolve) {
            Qt.callLater(function() {
                resolve(42);
            });
        });

        Promise.all([
            p.then(function(res) { return res + 1; }),
            p.then(function(res) { return res + 2; }),
            p.then(function(res) { return res + 3; })
        ]).then(function(res) {
            values = res;
        }).wait();

        compare(values, [43, 44, 45]);
    }
}

/*
void tst_requirements::thenMultipleCalls()
{
    // 2.2.6.2. If/when promise is rejected, all respective onRejected callbacks
    // must execute in the order of their originating calls to then:
    {
        QVector<int> values;
        QPromise<int> p([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
            qtpromise_defer([=]() {
                reject(8);
            });
        });

        qPromiseAll(QVector<QPromise<int> >{
            p.then(nullptr, [&](int r) { values << r + 1; return r + 1; }),
            p.then(nullptr, [&](int r) { values << r + 2; return r + 2; }),
            p.then(nullptr, [&](int r) { values << r + 3; return r + 3; })
        }).wait();

        QCOMPARE(values, QVector<int>({9, 10, 11}));
    }
}

void tst_requirements::thenHandlers()
{
    // 2.2.7. then must return a promise: p2 = p1.then(onFulfilled, onRejected);
    {
        auto handler = [](){ return 42; };
        auto p1 = QPromise<int>::resolve(42);
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
        auto p1 = QPromise<int>::resolve(42);
        auto p2 = p1.then([](){ throw QString("foo"); });
        p2.then(nullptr, [&](const QString& e) { reason = e; }).wait();

        QVERIFY(p1.isFulfilled());
        QVERIFY(p2.isRejected());
        QCOMPARE(reason, QString("foo"));
    }
    {
        QString reason;
        auto p1 = QPromise<int>::reject(QString("foo"));
        auto p2 = p1.then(nullptr, [](){ throw QString("bar"); return 42; });
        p2.then(nullptr, [&](const QString& e) { reason = e; return 0; }).wait();

        QVERIFY(p1.isRejected());
        QVERIFY(p2.isRejected());
        QCOMPARE(reason, QString("bar"));
    }

    // 2.2.7.3. If onFulfilled is not a function and promise1 is fulfilled,
    // p2 must be fulfilled with the same value as promise1
    {
        QString value;
        auto p1 = QPromise<QString>::resolve("42");
        auto p2 = p1.then(nullptr, [](){ return QString(); });
        Q_STATIC_ASSERT((std::is_same<decltype(p2), QPromise<QString> >::value));
        p2.then([&](const QString& e) { value = e; }).wait();

        QVERIFY(p1.isFulfilled());
        QVERIFY(p2.isFulfilled());
        QCOMPARE(value, QString("42"));
    }
}
*/
