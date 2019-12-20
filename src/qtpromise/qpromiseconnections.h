/*
 * Copyright (c) 2019 Simon Brunel, https://github.com/simonbrunel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef QTPROMISE_QPROMISECONNECTIONS_H
#define QTPROMISE_QPROMISECONNECTIONS_H

// Qt
#include <QSharedPointer>

namespace QtPromise {

class QPromiseConnections
{
public:
    QPromiseConnections() : m_d(new Data()) { }

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

        ~Data() {
            if (!connections.empty()) {
                qWarning("QPromiseConnections: destroyed with unhandled connections.");
                disconnect();
            }
        }

        void disconnect() {
            for (const auto& connection: connections) {
                QObject::disconnect(connection);
            }
            connections.clear();
        }
    };

    QSharedPointer<Data> m_d;
};

} // namespace QtPromise

#endif // QTPROMISE_QPROMISECONNECTIONS_H
