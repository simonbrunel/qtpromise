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

protected:
    int m_connections = 0;
    void connectNotify(const QMetaMethod&) override { ++m_connections; }
    void disconnectNotify(const QMetaMethod&) override { --m_connections; }
};

QTEST_MAIN(tst_signals)
#include "tst_signals.moc"


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
