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
    void join();
    void joinMixed();
    void joinspread();
    void chain();
    void delayed();
    void unpack();
};

QTEST_MAIN(tst_qpromise_spread)
#include "tst_spread.moc"

void tst_qpromise_spread::join()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QPromise<QByteArray>::resolve("42")
    );

    auto p = QPromise<std::tuple<int,QString,QByteArray>>::join(in);
    auto expected = std::make_tuple(42, QString("42"), QByteArray("42"));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<std::tuple<int,QString,QByteArray>>>::value));
    QCOMPARE(waitForValue(p, std::tuple<int,QString,QByteArray>()), expected);
}

void tst_qpromise_spread::joinMixed()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                "42"
    );

    auto p = QPromise<std::tuple<int,QString,QByteArray>>::join(in);
    auto expected = std::make_tuple(42, QString("42"), QByteArray("42"));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<std::tuple<int,QString,QByteArray>>>::value));
    QCOMPARE(waitForValue(p, std::tuple<int,QString,QByteArray>()), expected);
}

void tst_qpromise_spread::joinspread()
{

    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QPromise<QByteArray>::resolve("42")
    );

    auto p = QPromise<std::tuple<int,QString,QByteArray>>::join(in)
            .spread([=](int arg1, const QString &arg2, const QByteArray &arg3) {
                return QString("test %1 %2 %3")
                                          .arg(arg1)
                                          .arg(arg2)
                                          .arg(arg3.constData());
            });

    auto expected = QString("test 42 42 42");

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(waitForValue(p, QString()), expected);
}

void tst_qpromise_spread::chain()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QPromise<QByteArray>::resolve("42")
    );

    auto p = QPromise<std::tuple<int,QString,QByteArray>>::join(in)
            .spread([=](int arg1, const QString &arg2, const QByteArray &arg3) {
                auto in2 = std::make_tuple(
                            QPromise<int>::resolve(arg1 + 1),
                            QPromise<QString>::resolve(arg2 + QLatin1Char('2')),
                            QPromise<QByteArray>::resolve(arg3)
                        );
                return QPromise<std::tuple<int,QString,QByteArray>>::join(in2);
            })
            .spread([=](int arg1, const QString &arg2, const QByteArray &arg3) {
                return QString("test %1 %2 %3")
                                          .arg(arg1)
                                          .arg(arg2)
                                          .arg(arg3.constData());
            });

    auto expected = QString("test 43 422 42");

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(waitForValue(p, QString()), expected);
}

void tst_qpromise_spread::delayed()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QPromise<QByteArray>::resolve("42")
    );

    auto p = QPromise<std::tuple<int,QString,QByteArray>>::join(in)
            .spread([=](int arg1, const QString &arg2, const QByteArray &arg3) {
                auto in2 = std::make_tuple(
                            QPromise<int>::resolve(arg1 + 1).delay(100),
                            QPromise<QString>::resolve(arg2 + QLatin1Char('2')).delay(150),
                            QPromise<QByteArray>::resolve(arg3).delay(200)
                        );
                return QPromise<std::tuple<int,QString,QByteArray>>::join(in2);
            })
            .spread([=](int arg1, const QString &arg2, const QByteArray &arg3) {
                return QString("test %1 %2 %3")
                                          .arg(arg1)
                                          .arg(arg2)
                                          .arg(arg3.constData());
            });

    auto expected = QString("test 43 422 42");

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(waitForValue(p, QString()), expected);
}

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

void tst_qpromise_spread::unpack()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QPromise<QByteArray>::resolve("42")
    );

    auto p = QPromise<std::tuple<int,QString,QByteArray>>::join(in)
            .unpack<Result>();

    Result expected = {42, QLatin1String("42"), "42"};

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Result>>::value));
    QCOMPARE(waitForValue(p, Result()), expected);
}

