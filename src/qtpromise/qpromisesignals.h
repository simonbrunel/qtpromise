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

#endif // QTPROMISE_QPROMISESIGNALS_H
