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
    void all();
    void allspread();
    void chain();
};

QTEST_MAIN(tst_qpromise_spread)
#include "tst_spread.moc"

void tst_qpromise_spread::all()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QPromise<QByteArray>::resolve("42")
    );

    auto p = QPromise<std::tuple<int,QString,QByteArray>>::all(in);
    auto expected = std::make_tuple(42, QString("42"), QByteArray("42"));

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<std::tuple<int,QString,QByteArray>>>::value));
    QCOMPARE(waitForValue(p, std::tuple<int,QString,QByteArray>()), expected);
}

void tst_qpromise_spread::allspread()
{

    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QPromise<QByteArray>::resolve("42")
    );

    auto p = QPromise<std::tuple<int,QString,QByteArray>>::all(in)
            .spread<QString>([=](int arg1, const QString &arg2, const QByteArray &arg3) {
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

    auto p = QPromise<std::tuple<int,QString,QByteArray>>::all(in)
            .spread<std::tuple<int,QString,QByteArray>>([=](int arg1, const QString &arg2, const QByteArray &arg3) {
                auto in2 = std::make_tuple(
                            QPromise<int>::resolve(arg1 + 1),
                            QPromise<QString>::resolve(arg2 + QLatin1Char('2')),
                            QPromise<QByteArray>::resolve(arg3)
                        );
                return QPromise<std::tuple<int,QString,QByteArray>>::all(in2);
            })
            .spread<QString>([=](int arg1, const QString &arg2, const QByteArray &arg3) {
                return QString("test %1 %2 %3")
                                          .arg(arg1)
                                          .arg(arg2)
                                          .arg(arg3.constData());
            });

    auto expected = QString("test 43 422 42");

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(waitForValue(p, QString()), expected);
}

