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
    void spread();
    void spreadMixed();
    void spreadDelayed();
    void spreadDelayedReject();
};

QTEST_MAIN(tst_qpromise_spread)
#include "tst_spread.moc"


template <typename T>
QPromise<T> delayResolve( const T& value, int ms)
{
    Q_UNUSED(ms);
    return QPromise<void>::resolve().delay(ms).then([=](){
        return QPromise<T>::resolve(value);
    });
}

template <typename T>
QPromise<T> delayReject( const T& value, int ms)
{
    Q_UNUSED(ms);
    return QPromise<void>::resolve().delay(ms).then([=](){
        return QPromise<T>::reject(value);
    });
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
