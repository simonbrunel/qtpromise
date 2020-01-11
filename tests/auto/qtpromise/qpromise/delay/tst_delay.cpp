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

#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_delay : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fulfilled();
    void rejected();
};

QTEST_MAIN(tst_qpromise_delay)
#include "tst_delay.moc"

void tst_qpromise_delay::fulfilled()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;

    timer.start();

    auto p = QPromise<int>::resolve(42).delay(1000).finally([&]() {
        elapsed = timer.elapsed();
    });

    QCOMPARE(waitForValue(p, -1), 42);
    QCOMPARE(p.isFulfilled(), true);

    // Qt::CoarseTimer (default) Coarse timers try to
    // keep accuracy within 5% of the desired interval.
    // Require accuracy within 6% for passing the test.
    QVERIFY(elapsed >= static_cast<qint64>(1000 * 0.94));
    QVERIFY(elapsed <= static_cast<qint64>(1000 * 1.06));
}

void tst_qpromise_delay::rejected()
{
    QElapsedTimer timer;
    qint64 elapsed = -1;

    timer.start();

    auto p = QPromise<int>::reject(QString("foo")).delay(1000).finally([&]() {
        elapsed = timer.elapsed();
    });

    QCOMPARE(waitForError(p, QString()), QString("foo"));
    QCOMPARE(p.isRejected(), true);
    QVERIFY(elapsed <= 10);
}
