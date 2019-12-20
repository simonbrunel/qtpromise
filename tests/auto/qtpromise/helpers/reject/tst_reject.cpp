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

// STL
#include <memory>

using namespace QtPromise;

class tst_helpers_reject : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void rejectWithValue();
    void rejectWithQSharedPtr();
    void rejectWithStdSharedPtr();
};

QTEST_MAIN(tst_helpers_reject)
#include "tst_reject.moc"

void tst_helpers_reject::rejectWithValue()
{
    auto p = QPromise<int>::reject(42);

    QCOMPARE(p.isRejected(), true);
    QCOMPARE(waitForError(p, -1), 42);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_helpers_reject::rejectWithQSharedPtr()
{
    QWeakPointer<int> wptr;

    {
        QSharedPointer<int> sptr(new int(42));
        auto p = QPromise<int>::reject(sptr);

        QCOMPARE(waitForError(p, QSharedPointer<int>()), sptr);

        wptr = sptr;
        sptr.reset();

        QCOMPARE(wptr.isNull(), false); // "p" still holds a reference
    }

    QCOMPARE(wptr.isNull(), true);
}

// https://github.com/simonbrunel/qtpromise/issues/6
void tst_helpers_reject::rejectWithStdSharedPtr()
{
    std::weak_ptr<int> wptr;

    {
        std::shared_ptr<int> sptr(new int(42));
        auto p = QPromise<int>::reject(sptr);

        QCOMPARE(waitForError(p, std::shared_ptr<int>()), sptr);

        wptr = sptr;
        sptr.reset();

        QCOMPARE(wptr.use_count(), 1l); // "p" still holds a reference
    }

    QCOMPARE(wptr.use_count(), 0l);
}
