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

class tst_qpromise_fail : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void sameType();
    void baseClass();
    void catchAll();
    // TODO: sync / async
};

QTEST_MAIN(tst_qpromise_fail)
#include "tst_fail.moc"

void tst_qpromise_fail::sameType()
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

void tst_qpromise_fail::baseClass()
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

void tst_qpromise_fail::catchAll()
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
