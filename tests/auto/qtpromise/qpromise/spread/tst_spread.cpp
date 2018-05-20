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
    void propsReject();
    void propsDelayedReject();
    void spread();
    void spreadMixed();
    void spreadDelayed();
    void spreadDelayedReject();
    void chained();
};

QTEST_MAIN(tst_qpromise_spread)
#include "tst_spread.moc"


template <typename T>
QPromise<T> delayResolve( const T& value, int ms)
{
    return QPromise<T>([&](
                const QPromiseResolve<T>& resolve,
                const QPromiseReject<T>&) {
                    QtPromisePrivate::qtpromise_defer([=]() {
                        QPromise<void>::resolve().delay(ms).then([=](){
                            resolve(value);
                        });
                    });
                });
}

template <typename T>
QPromise<T> delayReject( const T& value, int ms)
{
    return QPromise<T>([&](
                const QPromiseResolve<T>&,
                const QPromiseReject<T>& reject) {
                    QtPromisePrivate::qtpromise_defer([=]() {
                        QPromise<void>::resolve().delay(ms).then([=](){
                            reject(value);
                        });
                    });
                });
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
                delayResolve<int>(42, 200),
                delayResolve<QString>(QString("42"), 100),
                delayResolve<QByteArray>("42", 300)
    );

    auto p = QPromise<Result>::props(in);

    Result expected = {42, QLatin1String("42"), "42"};

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Result>>::value));
    QCOMPARE(waitForValue(p, Result()), expected);
}

void tst_qpromise_spread::propsReject()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::reject(QString("This is an error")),
                QPromise<QByteArray>::resolve("42")
    );

    auto p = QPromise<Result>::props(in);

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Result>>::value));
    QCOMPARE(waitForError(p, QString()), QString("This is an error"));
}

void tst_qpromise_spread::propsDelayedReject()
{
    auto in = std::make_tuple(
                delayResolve<int>(42, 200),
                delayReject<QString>(QString("This is an error"), 100),
                delayResolve<QByteArray>("42", 300)
    );

    auto p = QPromise<Result>::props(in);

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<Result>>::value));
    QCOMPARE(waitForError(p, QString()), QString("This is an error"));
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

void tst_qpromise_spread::spreadMixed()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QByteArray("42")
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

void tst_qpromise_spread::spreadDelayed()
{
    auto in = std::make_tuple(
                delayResolve<int>(42, 200),
                delayResolve<QString>(QString("42"), 100),
                delayResolve<QByteArray>("42", 300)
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

void tst_qpromise_spread::spreadDelayedReject()
{
    auto in = std::make_tuple(
                delayResolve<int>(42, 200),
                delayReject<QString>(QString("This is an error"), 100),
                delayResolve<QByteArray>("42", 300)
    );

    auto p = QPromise<QString>::spread(in, [=](int arg1, const QString &arg2, const QByteArray &arg3) {
                return QString("test %1 %2 %3")
                                          .arg(arg1)
                                          .arg(arg2)
                                          .arg(arg3.constData());
            });

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(waitForError(p, QString()), QString("This is an error"));
}

void tst_qpromise_spread::chained()
{
    auto in = std::make_tuple(
                QPromise<int>::resolve(42),
                QPromise<QString>::resolve(QLatin1String("42")),
                QPromise<QByteArray>::resolve("42")
    );

    auto p = QPromise<Result>::props(in)
            .then([]( const Result& res) {
                auto in1 = std::make_tuple(
                    QPromise<int>::resolve(res.arg1 + 1),
                    QPromise<QString>::resolve(res.arg2 + QString("2")),
                    QPromise<QByteArray>::resolve(res.arg3 + QByteArray("2"))
                );

                return QPromise<QString>::spread(in1, [](int arg1, const QString &arg2, const QByteArray &arg3) {
                    return QString("test %1 %2 %3")
                                              .arg(arg1)
                                              .arg(arg2)
                                              .arg(arg3.constData());
                });
            });

    auto expected = QString("test 43 422 422");

    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QString>>::value));
    QCOMPARE(waitForValue(p, QString()), expected);
}

