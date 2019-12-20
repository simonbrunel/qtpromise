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

#include "../shared/object.h"
#include "../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromiseconnections : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void connections();
    void destruction();
    void senderDestroyed();

}; // class tst_qpromiseconnections

QTEST_MAIN(tst_qpromiseconnections)
#include "tst_qpromiseconnections.moc"

void tst_qpromiseconnections::connections()
{
    Object sender;

    QPromiseConnections connections;
    QCOMPARE(sender.hasConnections(), false);
    QCOMPARE(connections.count(), 0);

    connections << connect(&sender, &Object::noArgSignal, [=]() {});
    QCOMPARE(sender.hasConnections(), true);
    QCOMPARE(connections.count(), 1);

    connections << connect(&sender, &Object::twoArgsSignal, [=]() {});
    QCOMPARE(sender.hasConnections(), true);
    QCOMPARE(connections.count(), 2);

    connections.disconnect();
    QCOMPARE(sender.hasConnections(), false);
    QCOMPARE(connections.count(), 0);
}

void tst_qpromiseconnections::destruction()
{
    Object sender;

    {
        QPromiseConnections connections;
        QCOMPARE(sender.hasConnections(), false);
        QCOMPARE(connections.count(), 0);

        connections << connect(&sender, &Object::noArgSignal, [=]() {});
        QCOMPARE(sender.hasConnections(), true);
        QCOMPARE(connections.count(), 1);
    }

    QCOMPARE(sender.hasConnections(), false);
}

void tst_qpromiseconnections::senderDestroyed()
{
    QPromiseConnections connections;
    QCOMPARE(connections.count(), 0);

    {
        Object sender;
        QCOMPARE(sender.hasConnections(), false);

        connections << connect(&sender, &Object::noArgSignal, [=]() {});
        QCOMPARE(sender.hasConnections(), true);
        QCOMPARE(connections.count(), 1);
    }

    // should not throw
    connections.disconnect();
    QCOMPARE(connections.count(), 0);
}
