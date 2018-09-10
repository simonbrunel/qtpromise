#ifndef QTPROMISE_QPROMISESIGNALS_H
#define QTPROMISE_QPROMISESIGNALS_H

#include <QObject>
#include <memory>
#include "qpromise.h"


namespace QtPromise {

class QPromiseConnections : private std::shared_ptr<std::vector<QMetaObject::Connection>>
{
    using Connections = std::vector<QMetaObject::Connection>;
    using Base = std::shared_ptr<Connections>;

public:
    QPromiseConnections() : Base(new Connections, [](Connections* c) {
        if (!c->empty()) {
            qWarning("QPromiseConnections: destroyed with unhandled connections");
            disconnectConnections(c);
        }
        delete c;
    }) { }

    void operator<<(QMetaObject::Connection&& other) const
    {
        get()->emplace_back(std::move(other));
    }

    size_t count() const { return get()->size(); }

    void disconnect() const { disconnectConnections(get()); }

private:
    static void disconnectConnections(Connections* c) Q_DECL_NOEXCEPT
    {
        for (const auto& connection: *c) { QObject::disconnect(connection); }
        c->clear();
    }
};

} // namespace QtPromise

namespace QtPromisePrivate {

// Deduce promise types from Qt signals
// TODO: Suppress QPrivateSignal trailing private signal args
// TODO: Support deducing tuple from args (might require MSVC2017)
template <typename Signal>
using PromiseFromSignal = typename QtPromise::QPromise<Unqualified<typename ArgsOf<Signal>::first>>;

// Connect signal() to QPromiseResolve
template <typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count == 0)>::type
connectSignalToResolver(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseResolve<void>& resolve,
                        const Sender* sender, Signal signal)
{
    connections << QObject::connect(sender, signal, [=]() {
        resolve();
        connections.disconnect();
    });
}

// Connect signal() to QPromiseReject
template <typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count == 0)>::type
connectSignalToResolver(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseReject<T>& reject,
                        const Sender* sender, Signal signal)
{
    connections << QObject::connect(sender, signal, [=]() {
        reject(QtPromise::QPromiseSignalException<void>());
        connections.disconnect();
    });
}

// Connect signal(args...) to QPromiseResolve
template <typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count >= 1)>::type
connectSignalToResolver(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseResolve<T>& resolve,
                        const Sender* sender, Signal signal)
{
    connections << QObject::connect(sender, signal, [=](const T& value) {
        resolve(value);
        connections.disconnect();
    });
}

// Connect signal(args...) to QPromiseReject
template <typename T, typename Sender, typename Signal>
typename std::enable_if<(ArgsOf<Signal>::count >= 1)>::type
connectSignalToResolver(const QtPromise::QPromiseConnections& connections,
                        const QtPromise::QPromiseReject<T>& reject,
                        const Sender* sender, Signal signal)
{
    using V = Unqualified<typename ArgsOf<Signal>::first>;
    connections << QObject::connect(sender, signal, [=](const V& value) {
        reject(QtPromise::QPromiseSignalException<V>(value));
        connections.disconnect();
    });
}

// Connect QObject::destroyed signal to QPromiseReject
template <typename T, typename Sender>
void connectDestroyedToReject(const QtPromise::QPromiseConnections& connections,
                              const QtPromise::QPromiseReject<T>& reject,
                              const Sender* sender)
{
    connections << QObject::connect(sender, &QObject::destroyed, [=]() {
        reject(QtPromise::QPromiseContextException());
        connections.disconnect();
    });
}

} // namespace QtPromisePrivate

namespace QtPromise {

template <typename Sender, typename Signal>
static inline typename QtPromisePrivate::PromiseFromSignal<Signal>
connect(const Sender* sender, Signal signal)
{
    using T = typename QtPromisePrivate::PromiseFromSignal<Signal>::Type;
    using QtPromisePrivate::connectSignalToResolver;

    return QPromise<T>([&](const QPromiseResolve<T>& resolve,
                           const QPromiseReject<T>& reject) {
        QPromiseConnections connections;
        connectSignalToResolver(connections, resolve, sender, signal);
        QtPromisePrivate::connectDestroyedToReject(connections, reject, sender);
    });
}

template <typename FSender, typename FSignal, typename RSender, typename RSignal>
static inline typename QtPromisePrivate::PromiseFromSignal<FSignal>
connect(const FSender* fsender, FSignal fsignal, const RSender* rsender, RSignal rsignal)
{
    using T = typename QtPromisePrivate::PromiseFromSignal<FSignal>::Type;
    using QtPromisePrivate::connectSignalToResolver;

    return QPromise<T>([&](const QPromiseResolve<T>& resolve,
                           const QPromiseReject<T>& reject) {
        QPromiseConnections connections;
        connectSignalToResolver(connections, resolve, fsender, fsignal);
        connectSignalToResolver(connections, reject, rsender, rsignal);
        QtPromisePrivate::connectDestroyedToReject(connections, reject, fsender);
    });
}

template <typename Sender, typename FSignal, typename RSignal>
static inline typename QtPromisePrivate::PromiseFromSignal<FSignal>
connect(const Sender* sender, FSignal fsignal, RSignal rsignal)
{
    return connect(sender, fsignal, sender, rsignal);
}

} // namespace QtPromise

#endif // QTPROMISE_QPROMISESIGNALS_H
