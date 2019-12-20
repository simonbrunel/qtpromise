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

// QtPromise
#include <QtPromise>

// Qt
#include <QtConcurrent>
#include <QtTest>

using namespace QtPromise;

class tst_thread : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void resolve();
    void resolve_void();
    void reject();
    void then();
    void then_void();
    void fail();
    void finally();

}; // class tst_thread

QTEST_MAIN(tst_thread)
#include "tst_thread.moc"

void tst_thread::resolve()
{
    int value = -1;
    QThread* target = nullptr;
    QThread* source = nullptr;

    QPromise<int>([&](const QPromiseResolve<int>& resolve) {
        QtConcurrent::run([=, &source]() {
            source = QThread::currentThread();
            resolve(42);
        });
    }).then([&](int res) {
        target = QThread::currentThread();
        value = res;
    }).wait();

    QVERIFY(source != nullptr);
    QVERIFY(source != target);
    QCOMPARE(target, QThread::currentThread());
    QCOMPARE(value, 42);
}

void tst_thread::resolve_void()
{
    int value = -1;
    QThread* target = nullptr;
    QThread* source = nullptr;

    QPromise<void>([&](const QPromiseResolve<void>& resolve) {
        QtConcurrent::run([=, &source]() {
            source = QThread::currentThread();
            resolve();
        });
    }).then([&]() {
        target = QThread::currentThread();
        value = 43;
    }).wait();

    QVERIFY(source != nullptr);
    QVERIFY(source != target);
    QCOMPARE(target, QThread::currentThread());
    QCOMPARE(value, 43);
}

void tst_thread::reject()
{
    QString error;
    QThread* target = nullptr;
    QThread* source = nullptr;

    QPromise<int>([&](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        QtConcurrent::run([=, &source]() {
            source = QThread::currentThread();
            reject(QString("foo"));
        });
    }).fail([&](const QString& err) {
        target = QThread::currentThread();
        error = err;
        return -1;
    }).wait();

    QVERIFY(source != nullptr);
    QVERIFY(source != target);
    QCOMPARE(target, QThread::currentThread());
    QCOMPARE(error, QString("foo"));
}

void tst_thread::then()
{
    QThread* source = nullptr;
    QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
        source = QThread::currentThread();
        resolve(42);
    });

    int value = -1;
    QThread* target = nullptr;
    QtPromise::resolve(QtConcurrent::run([&](const QPromise<int>& p) {
        p.then([&](int res) {
            target = QThread::currentThread();
            value = res;
        }).wait();
    }, p)).wait();

    QVERIFY(target != nullptr);
    QVERIFY(source != target);
    QCOMPARE(source, QThread::currentThread());
    QCOMPARE(value, 42);
}

void tst_thread::then_void()
{
    QThread* source = nullptr;
    QPromise<void> p([&](const QPromiseResolve<void>& resolve) {
        source = QThread::currentThread();
        resolve();
    });

    int value = -1;
    QThread* target = nullptr;
    QtPromise::resolve(QtConcurrent::run([&](const QPromise<void>& p) {
        p.then([&]() {
            target = QThread::currentThread();
            value = 43;
        }).wait();
    }, p)).wait();

    QVERIFY(target != nullptr);
    QVERIFY(source != target);
    QCOMPARE(source, QThread::currentThread());
    QCOMPARE(value, 43);
}

void tst_thread::fail()
{
    QThread* source = nullptr;
    QPromise<int> p([&](const QPromiseResolve<int>&, const QPromiseReject<int>& reject) {
        source = QThread::currentThread();
        reject(QString("foo"));
    });

    QString error;
    QThread* target = nullptr;
    QtPromise::resolve(QtConcurrent::run([&](const QPromise<int>& p) {
        p.fail([&](const QString& err) {
            target = QThread::currentThread();
            error = err;
            return -1;
        }).wait();
    }, p)).wait();

    QVERIFY(target != nullptr);
    QVERIFY(source != target);
    QCOMPARE(source, QThread::currentThread());
    QCOMPARE(error, QString("foo"));
}

void tst_thread::finally()
{
    QThread* source = nullptr;
    QPromise<int> p([&](const QPromiseResolve<int>& resolve) {
        source = QThread::currentThread();
        resolve(42);
    });

    int value = -1;
    QThread* target = nullptr;
    QtPromise::resolve(QtConcurrent::run([&](const QPromise<int>& p) {
        p.finally([&]() {
            target = QThread::currentThread();
            value = 43;
        }).wait();
    }, p)).wait();

    QVERIFY(target != nullptr);
    QVERIFY(source != target);
    QCOMPARE(source, QThread::currentThread());
    QCOMPARE(value, 43);
}
