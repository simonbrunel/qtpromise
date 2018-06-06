#include <QString>
#include <QtTest>

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_signals : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void resolve();
    void resolve_void();
    void reject();
    void reject_void();
    void reject_deleted();

    void signalSource();
    void promiseConnections();

}; // class tst_signals


// Test class for emitting signals and checking connections
class SignalSource : public QObject
{
    Q_OBJECT

public:
    // Return true if listeners are connected to signals
    bool hasConnectedSignals() const { return m_connections > 0; }

signals:
    void voidSignal();
    void intSignal(int value);
    void strSignal(const QString& value);

protected:
    int m_connections = 0;
    void connectNotify(const QMetaMethod&) override { ++m_connections; }
    void disconnectNotify(const QMetaMethod&) override { --m_connections; }
};

QTEST_MAIN(tst_signals)
#include "tst_signals.moc"


void tst_signals::resolve()
{
    // Test creating promise from resolve signal with value
    SignalSource signalSource;
    auto p = QtPromise::connect(&signalSource, &SignalSource::strSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QVERIFY(p.isPending());
    QVERIFY(signalSource.hasConnectedSignals());

    const QString testValue("42");
    QString result;
    emit signalSource.strSignal(testValue);
    p.then([&](const QString& r) { result = r; }).wait();

    QVERIFY(p.isFulfilled());
    QVERIFY(result == testValue);
    QVERIFY(!signalSource.hasConnectedSignals());
}

void tst_signals::resolve_void()
{
    // Test creating promise from void resolve signal
    SignalSource signalSource;
    auto p = QtPromise::connect(&signalSource, &SignalSource::voidSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QVERIFY(p.isPending());
    QVERIFY(signalSource.hasConnectedSignals());

    emit signalSource.voidSignal();
    p.wait();

    QVERIFY(p.isFulfilled());
    QVERIFY(!signalSource.hasConnectedSignals());
}

void tst_signals::reject()
{
    // Test creating promise from resolve and reject signal with value
    SignalSource signalSource;
    auto p = QtPromise::connect(&signalSource, &SignalSource::voidSignal, &SignalSource::strSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<void>>::value));
    QVERIFY(p.isPending());
    QVERIFY(signalSource.hasConnectedSignals());

    const QString testValue("42");
    QString reason;
    emit signalSource.strSignal(testValue);
    p.fail([&](const QString& r) { reason = r; }).wait();

    QVERIFY(p.isRejected());
    QVERIFY(reason == testValue);
    QVERIFY(!signalSource.hasConnectedSignals());
}

void tst_signals::reject_void()
{
    // Test creating promise from void resolve signal
    SignalSource signalSource;
    auto p = QtPromise::connect(&signalSource, &SignalSource::intSignal, &SignalSource::voidSignal);
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<int>>::value));
    QVERIFY(p.isPending());
    QVERIFY(signalSource.hasConnectedSignals());

    emit signalSource.voidSignal();
    bool result = false;
    p.fail([&](const QPromiseSignalException&) { result = true; return 0; }).wait();

    QVERIFY(p.isRejected());
    QVERIFY(result);
    QVERIFY(!signalSource.hasConnectedSignals());
}

void tst_signals::reject_deleted()
{
    // Test creating promise from a signal source that is being deleted
    QPromise<void> p = qPromise();
    {
        SignalSource signalSource;
        p = QtPromise::connect(&signalSource, &SignalSource::voidSignal);
        QVERIFY(p.isPending());
    }
    bool result = false;
    p.fail([&](const QPromiseContextException&) { result = true; }).wait();

    QVERIFY(p.isRejected());
    QVERIFY(result);
}

void tst_signals::signalSource()
{
    // Check if SignalSource is aware of connected signal handlers
    SignalSource signalSource;
    QVERIFY(!signalSource.hasConnectedSignals());
    auto c = connect(&signalSource, &SignalSource::voidSignal, [](){});
    QVERIFY(signalSource.hasConnectedSignals());
    disconnect(c);
    QVERIFY(!signalSource.hasConnectedSignals());
}

void tst_signals::promiseConnections()
{
    SignalSource source;
    // Track and disconnect signals with QPromiseConnections
    {
        QPromiseConnections connections;
        connections << connect(&source, &SignalSource::intSignal, [=]() {
            connections.disconnect();
        });
        connections << connect(&source, &SignalSource::voidSignal, [=]() {
            connections.disconnect();
        });
        QVERIFY(connections.count() == 2);
        QVERIFY(source.hasConnectedSignals());

        emit source.voidSignal();
        QVERIFY(connections.count() == 0);
        QVERIFY(!source.hasConnectedSignals());
    }
    // Disconnect on destruction
    {
        QPromiseConnections connections;
        connections << connect(&source, &SignalSource::intSignal, []() {});
        QVERIFY(source.hasConnectedSignals());
    }
    QVERIFY(!source.hasConnectedSignals());
}
