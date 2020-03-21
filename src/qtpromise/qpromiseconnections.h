/*
 * Copyright (c) Simon Brunel, https://github.com/simonbrunel
 *
 * This source code is licensed under the MIT license found in
 * the LICENSE file in the root directory of this source tree.
 */

#ifndef QTPROMISE_QPROMISECONNECTIONS_H
#define QTPROMISE_QPROMISECONNECTIONS_H

#include <QtCore/QSharedPointer>

namespace QtPromise {

class QPromiseConnections
{
public:
    QPromiseConnections() : m_d(QSharedPointer<Data>::create()) { }

    int count() const { return m_d->connections.count(); }

    void disconnect() const { m_d->disconnect(); }

    void operator<<(QMetaObject::Connection&& other) const
    {
        m_d->connections.append(std::move(other));
    }

private:
    struct Data
    {
        QVector<QMetaObject::Connection> connections;

        ~Data()
        {
            if (!connections.empty()) {
                qWarning("QPromiseConnections: destroyed with unhandled connections.");
                disconnect();
            }
        }

        void disconnect()
        {
            for (const auto& connection : connections) {
                QObject::disconnect(connection);
            }
            connections.clear();
        }
    };

    QSharedPointer<Data> m_d;
};

} // namespace QtPromise

#endif // QTPROMISE_QPROMISECONNECTIONS_H
