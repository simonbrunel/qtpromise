// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;
using namespace QtPromisePrivate;

class tst_qpromise : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void resolveSync();
    void resolveSync_void();
    void resolveDelayed();
    void rejectSync();
    void rejectDelayed();
    void rejectThrows();

    void thenReturns();
    void thenThrows();
    void thenNullPtr();
    void thenSkipResult();
    void thenDelayedResolved();
    void thenDelayedRejected();

    void failSameType();
    void failBaseClass();
    void failCatchAll();

    void finallyReturns();
    void finallyReturns_void();
    void finallyThrows();
    void finallyThrows_void();
    void finallyDelayedResolved();
    void finallyDelayedRejected();

}; // class tst_qpromise

QTEST_MAIN(tst_qpromise)
#include "tst_qpromise.moc"

template <typename T>
T waitForValue(const QPromise<T>& promise, const T& initial)
{
    T value(initial);
    promise.then([&](const T& res) {
        value = res;
    }).wait();
    return value;
}

template <typename T>
T waitForValue(const QPromise<void>& promise, const T& initial, const T& expected)
{
    T value(initial);
    promise.then([&]() {
        value = expected;
    }).wait();
    return value;
}

template <typename T, typename E>
E waitForError(const QPromise<T>& promise, const E& initial)
{
    E error(initial);
    promise.fail([&](const E& err) {
        error = err;
        return T();
    }).wait();
    return error;
}

template <typename E>
E waitForError(const QPromise<void>& promise, const E& initial)
{
    E error(initial);
    promise.fail([&](const E& err) {
        error = err;
    }).wait();
    return error;
}

void tst_qpromise::resolveSync()
{
    {   // resolver(resolve)
        QPromise<int> p([](const QPromiseResolve<int>& resolve) {
            resolve(42);
        });

        QCOMPARE(p.isFulfilled(), true);
        QCOMPARE(waitForError(p, QString()), QString());
        QCOMPARE(waitForValue(p, -1), 42);
    }
    {   // resolver(resolve, reject)
        QPromise<int> p([](const QPromiseResolve<int>& resolve, const QPromiseReject<int>&) {
            resolve(42);
        });

        QCOMPARE(p.isFulfilled(), true);
        QCOMPARE(waitForError(p, QString()), QString());
        QCOMPARE(waitForValue(p, -1), 42);
    }
}

void tst_qpromise::resolveSync_void()
{
    {   // resolver(resolve)
        QPromise<void> p([](const QPromiseResolve<void>& resolve) {
            resolve();
        });

        QCOMPARE(p.isFulfilled(), true);
        QCOMPARE(waitForError(p, QString()), QString());
        QCOMPARE(waitForValue(p, -1, 42), 42);
    }
    {   // resolver(resolve, reject)
        QPromise<void> p([](const QPromiseResolve<void>& resolve, const QPromiseReject<void>&) {
            resolve();
        });

        QCOMPARE(p.isFulfilled(), true);
        QCOMPARE(waitForError(p, QString()), QString());
        QCOMPARE(waitForValue(p, -1, 42), 42);
    }
}

void tst_qpromise::resolveDelayed()
{
    {   // resolver(resolve)
        QPromise<int> p([](const QPromiseResolve<int>& resolve) {
            QtPromisePrivate::qtpromise_defer([=]() {
                resolve(42);
            });
        });

        QCOMPARE(p.isPending(), true);
        QCOMPARE(waitForError(p, QString()), QString());
        QCOMPARE(waitForValue(p, -1), 42);
        QCOMPARE(p.isFulfilled(), true);
    }
    {   // resolver(resolve, reject)
        QPromise<int> p([](const QPromiseResolve<int>& resolve, const QPromiseReject<int>&) {
            QtPromisePrivate::qtpromise_defer([=]() {
                resolve(42);
            });
        });

        QCOMPARE(p.isPending(), true);
        QCOMPARE(waitForError(p, QString()), QString());
        QCOMPARE(waitForValue(p, -1), 42);
        QCOMPARE(p.isFulfilled(), true);
    }
}

void tst_qpromise::rejectSync()
{
    QPromise<int> p([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        reject(QString("foo"));
    });

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(waitForValue(p, -1), -1);
    QCOMPARE(waitForError(p, QString()), QString("foo"));
}

void tst_qpromise::rejectDelayed()
{
    QPromise<int> p([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        QtPromisePrivate::qtpromise_defer([=]() {
            reject(QString("foo"));
        });
    });

    QCOMPARE(p.isPending(), true);
    QCOMPARE(waitForValue(p, -1), -1);
    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
}

void tst_qpromise::rejectThrows()
{
    {   // resolver(resolve)
        QPromise<int> p([](const QPromiseResolve<int>&) {
            throw QString("foo");
        });

        QCOMPARE(p.isRejected(), true);
        QCOMPARE(waitForValue(p, -1), -1);
        QCOMPARE(waitForError(p, QString()), QString("foo"));
    }
    {   // resolver(resolve, reject)
        QPromise<int> p([](const QPromiseResolve<int>&, const QPromiseReject<int>&) {
            throw QString("foo");
        });

        QCOMPARE(p.isRejected(), true);
        QCOMPARE(waitForValue(p, -1), -1);
        QCOMPARE(waitForError(p, QString()), QString("foo"));
    }
}

void tst_qpromise::thenReturns()
{
    auto p = QPromise<int>::resolve(42);

    QVariantList values;
    p.then([&](int res) {
        values << res;
        return QString::number(res+1);
    }).then([&](const QString& res) {
        values << res;
    }).then([&]() {
        values << 44;
    }).wait();

    QCOMPARE(values, QVariantList({42, QString("43"), 44}));
}

void tst_qpromise::thenThrows()
{
    auto input = QPromise<int>::resolve(42);
    auto output = input.then([](int res) {
        throw QString("foo%1").arg(res);
        return 42;
    });

    QString error;
    output.then([&](int res) {
        error += "bar" + QString::number(res);
    }).fail([&](const QString& err) {
        error += err;
    }).wait();

    QCOMPARE(input.isFulfilled(), true);
    QCOMPARE(output.isRejected(), true);
    QCOMPARE(error, QString("foo42"));
}

void tst_qpromise::thenNullPtr()
{
    {   // resolved
        auto p = QPromise<int>::resolve(42).then(nullptr);

        QCOMPARE(waitForValue(p, -1), 42);
        QCOMPARE(p.isFulfilled(), true);
    }
    {   // rejected
        auto p = QPromise<int>::reject(QString("foo")).then(nullptr);

        QCOMPARE(waitForError(p, QString()), QString("foo"));
        QCOMPARE(p.isRejected(), true);
    }
}

void tst_qpromise::thenSkipResult()
{
    auto p = QPromise<int>::resolve(42);

    int value = -1;
    p.then([&]() {
        value = 43;
    }).wait();

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int> >::value));
    QCOMPARE(value, 43);
}

void tst_qpromise::thenDelayedResolved()
{
    auto p = QPromise<int>::resolve(42).then([](int res) {
        return QPromise<QString>([=](const QPromiseResolve<QString>& resolve) {
            QtPromisePrivate::qtpromise_defer([=]() {
                resolve(QString("foo%1").arg(res));
            });
        });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString> >::value));
    QCOMPARE(waitForValue(p, QString()), QString("foo42"));
}

void tst_qpromise::thenDelayedRejected()
{
    auto p = QPromise<int>::resolve(42).then([](int res) {
        return QPromise<void>([=](const QPromiseResolve<void>&, const QPromiseReject<void>& reject) {
            QtPromisePrivate::qtpromise_defer([=]() {
                reject(QString("foo%1").arg(res));
            });
        });
    });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void> >::value));
    QCOMPARE(waitForError(p, QString()), QString("foo42"));
}

void tst_qpromise::failSameType()
{
    // http://en.cppreference.com/w/cpp/error/exception
    auto p = QPromise<int>::reject(std::out_of_range("foo"));

    QString error;
    p.fail([&](const std::domain_error& e) {
        error += QString(e.what()) + "0";
        return -1;
    }).fail([&](const std::out_of_range& e) {
        error += QString(e.what()) + "1";
        return -1;
    }).fail([&](const std::exception& e) {
        error += QString(e.what()) + "2";
        return -1;
    }).wait();

    QCOMPARE(error, QString("foo1"));
}

void tst_qpromise::failBaseClass()
{
    // http://en.cppreference.com/w/cpp/error/exception
    auto p = QPromise<int>::reject(std::out_of_range("foo"));

    QString error;
    p.fail([&](const std::runtime_error& e) {
        error += QString(e.what()) + "0";
        return -1;
    }).fail([&](const std::logic_error& e) {
        error += QString(e.what()) + "1";
        return -1;
    }).fail([&](const std::exception& e) {
        error += QString(e.what()) + "2";
        return -1;
    }).wait();

    QCOMPARE(error, QString("foo1"));
}

void tst_qpromise::failCatchAll()
{
    auto p = QPromise<int>::reject(std::out_of_range("foo"));

    QString error;
    p.fail([&](const std::runtime_error& e) {
        error += QString(e.what()) + "0";
        return -1;
    }).fail([&]() {
        error += "bar";
        return -1;
    }).fail([&](const std::exception& e) {
        error += QString(e.what()) + "2";
        return -1;
    }).wait();

    QCOMPARE(error, QString("bar"));
}

void tst_qpromise::finallyReturns()
{
    {   // fulfilled
        int value = -1;
        auto p = QPromise<int>::resolve(42).finally([&]() {
            value = 8;
            return 16; // ignored!
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int> >::value));
        QCOMPARE(waitForValue(p, -1), 42);
        QCOMPARE(p.isFulfilled(), true);
        QCOMPARE(value, 8);
    }
    {   // rejected
        int value = -1;
        auto p = QPromise<int>::reject(QString("foo")).finally([&]() {
            value = 8;
            return 16; // ignored!
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int> >::value));
        QCOMPARE(waitForError(p, QString()), QString("foo"));
        QCOMPARE(p.isRejected(), true);
        QCOMPARE(value, 8);
    }
}

void tst_qpromise::finallyReturns_void()
{
    {   // fulfilled
        int value = -1;
        auto p = QPromise<void>::resolve().finally([&]() {
            value = 8;
            return 16; // ignored!
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void> >::value));
        QCOMPARE(waitForValue(p, -1, 42), 42);
        QCOMPARE(p.isFulfilled(), true);
        QCOMPARE(value, 8);
    }
    {   // rejected
        int value = -1;
        auto p = QPromise<void>::reject(QString("foo")).finally([&]() {
            value = 8;
            return 16; // ignored!
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void> >::value));
        QCOMPARE(waitForError(p, QString()), QString("foo"));
        QCOMPARE(p.isRejected(), true);
        QCOMPARE(value, 8);
    }
}

void tst_qpromise::finallyThrows()
{
    {   // fulfilled
        auto p = QPromise<int>::resolve(42).finally([&]() {
            throw QString("bar");
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int> >::value));
        QCOMPARE(waitForError(p, QString()), QString("bar"));
        QCOMPARE(p.isRejected(), true);
    }
    {   // rejected
        auto p = QPromise<int>::reject(QString("foo")).finally([&]() {
            throw QString("bar");
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int> >::value));
        QCOMPARE(waitForError(p, QString()), QString("bar"));
        QCOMPARE(p.isRejected(), true);
    }
}

void tst_qpromise::finallyThrows_void()
{
    {   // fulfilled
        auto p = QPromise<void>::resolve().finally([&]() {
            throw QString("bar");
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void> >::value));
        QCOMPARE(waitForError(p, QString()), QString("bar"));
        QCOMPARE(p.isRejected(), true);
    }
    {   // rejected
        auto p = QPromise<void>::reject(QString("foo")).finally([&]() {
            throw QString("bar");
        });

        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void> >::value));
        QCOMPARE(waitForError(p, QString()), QString("bar"));
        QCOMPARE(p.isRejected(), true);
    }
}

void tst_qpromise::finallyDelayedResolved()
{
    {   // fulfilled
        QVector<int> values;
        auto p = QPromise<int>::resolve(42).finally([&]() {
            QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
                qtpromise_defer([=, &values]() {
                    values << 64;
                    resolve(16); // ignored!
                });
            });

            values << 8;
            return p;
        });

        QCOMPARE(waitForValue(p, -1), 42);
        QCOMPARE(p.isFulfilled(), true);
        QCOMPARE(values, QVector<int>({8, 64}));
    }
    {   // rejected
        QVector<int> values;
        auto p = QPromise<int>::reject(QString("foo")).finally([&]() {
            QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
                qtpromise_defer([=, &values]() {
                    values << 64;
                    resolve(16); // ignored!
                });
            });

            values << 8;
            return p;
        });

        p.then([&](int r) {
            values << r;
        }).wait();

        QCOMPARE(waitForError(p, QString()), QString("foo"));
        QCOMPARE(p.isRejected(), true);
        QCOMPARE(values, QVector<int>({8, 64}));
    }
}

void tst_qpromise::finallyDelayedRejected()
{
    {   // fulfilled
        auto p = QPromise<int>::resolve(42).finally([]() {
            return QPromise<int>([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
                qtpromise_defer([=]() {
                    reject(QString("bar"));
                });
            });
        });

        QCOMPARE(waitForError(p, QString()), QString("bar"));
        QCOMPARE(p.isRejected(), true);
    }
    {   // rejected
        auto p = QPromise<int>::reject(QString("foo")).finally([]() {
            return QPromise<int>([](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
                qtpromise_defer([=]() {
                    reject(QString("bar"));
                });
            });
        });

        QCOMPARE(waitForError(p, QString()), QString("bar"));
        QCOMPARE(p.isRejected(), true);
    }
}
