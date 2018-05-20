// Tests
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_spread : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void props();
    void propsMixed();
    void propsDelayed();
    void spread();
};

QTEST_MAIN(tst_qpromise_spread)
#include "tst_spread.moc"

struct Result {
    int arg1;
    QString arg2;
    QByteArray arg3;
};

bool operator==(const Result& l, const Result& r)
{
    return l.arg1 == r.arg1 &&
            l.arg2 == r.arg2 &&
            l.arg3 == r.arg3;
}

void tst_qpromise_spread::props()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QPromise<QByteArray>::resolve("42")
    );

    auto p = QPromise<Result>::props(in);

    Result expected = {42, QLatin1String("42"), "42"};

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Result>>::value));
    QCOMPARE(waitForValue(p, Result()), expected);
}

void tst_qpromise_spread::propsMixed()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QByteArray("42")
    );

    auto p = QPromise<Result>::props(in);

    Result expected = {42, QLatin1String("42"), "42"};

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Result>>::value));
    QCOMPARE(waitForValue(p, Result()), expected);
}

void tst_qpromise_spread::propsDelayed()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42).delay(300),
                QPromise<QString>::resolve(QLatin1String("42")).delay(200),
                QPromise<QByteArray>::resolve("42").delay(100)
    );

    auto p = QPromise<Result>::props(in);

    Result expected = {42, QLatin1String("42"), "42"};

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Result>>::value));
    QCOMPARE(waitForValue(p, Result()), expected);
}

void tst_qpromise_spread::spread()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QPromise<QByteArray>::resolve("42")
    );

    auto p = QPromise<QString>::spread(in, [=](int arg1, const QString &arg2, const QByteArray &arg3) {
                return QString("test %1 %2 %3")
                                          .arg(arg1)
                                          .arg(arg2)
                                          .arg(arg3.constData());
            });

    auto expected = QString("test 42 42 42");

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(waitForValue(p, QString()), expected);
}

