// Tests
#include "../../shared/utils.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QtTest>

using namespace QtPromise;

class tst_qpromise_cast : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void typeToVoid();


    //void castToVoidOperator();
    //void typeToVariant();   // QPromise<T>.cast<QVariant>()
    //void variantToType();
    //void voidToVariant();   // QPromise<void>.cast<QVariant>()
    //void variantToVoid();

    //void jsonValueToJsonObject();
};

QTEST_MAIN(tst_qpromise_cast)
#include "tst_cast.moc"

void tst_qpromise_cast::typeToVoid()
{
    QPromise<void> p0 = QPromise<int>::resolve(42);
    QPromise<void> p1 = QPromise<QString>::resolve(QString("foo"));
    QVERIFY(p0.isFulfilled());
    QVERIFY(p1.isFulfilled());
}

//void tst_qpromise_cast::castToVoidOperator()
//{
    //auto p0 = QPromise<int>::resolve(42);
    //QPromise<double> p1(p0);
    //QPromise<void> p2(p0);
    //auto p4 = QPromise<QString>::resolve("foo");
    //
    //p0.then([](int res) { qDebug() << res; });
    //p1.then([](double res) { qDebug() << res; });
    //p2.then([]() { qDebug() << "done"; });
    //
    //foo().then([]() {
    //    qDebug() << "done";
    //}).wait();
    //
    //QPromise<QVariant>::all({p0, p1, p4}).then([](const QVector<QVariant>& res) {
    //    qDebug() << "all done!" << res;
    //}).wait();
//}

/*
namespace {

template <typename T, typename U>
void test_qpromise_cast(const T& t, const U& u)
{
    auto p = QPromise<T>::resolve(t).cast<U>();
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<U> >::value));
    QCOMPARE(waitForValue(p, U()), u);
}

template <typename U>
void test_qpromise_cast(const U& u)
{
    auto p = QPromise<void>::resolve().cast<U>();
    Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<U> >::value));
    QCOMPARE(waitForValue(p, U()), u);
}

} // anonymous namespace


void tst_qpromise_cast::typeToVariant()
{
    test_qpromise_cast(42, QVariant(42));
    test_qpromise_cast(4.2, QVariant(4.2));
    test_qpromise_cast(true, QVariant(true));
    test_qpromise_cast(QString("foo"), QVariant(QString("foo")));
    test_qpromise_cast(QUrl("http://x.y.z"), QVariant(QUrl("http://x.y.z")));
    test_qpromise_cast(QSize(128, 256), QVariant(QSize(128, 256)));
    test_qpromise_cast(QDate(2018, 1, 1), QVariant(QDate(2018, 1, 1)));
    test_qpromise_cast(QJsonValue("foo"), QVariant(QJsonValue("foo")));
    test_qpromise_cast(QStringList{"foo", "bar"}, QVariant(QStringList{"foo", "bar"}));
    test_qpromise_cast(QList<QVariant>{"foo", 42}, QVariant(QList<QVariant>{"foo", 42}));
    test_qpromise_cast(QMap<QString, QVariant>{{"foo", 42}}, QVariant(QVariantMap{{"foo", 42}}));
}

void tst_qpromise_cast::voidToVariant()
{
    test_qpromise_cast(QVariant());
}

void tst_qpromise_cast::variantToType()
{
    // invalid
    // int
    // QString
    // QColor
    // QList
}


/*
void tst_qpromise_cast::jsonValueToJsonObject()
{
    {   // QJsonValue(Null) -> QJsonObject
        auto p = QPromise<QJsonValue>::resolve(QJsonValue()).cast<QJsonObject>();
        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QJsonObject> >::value));
        const QJsonObject res = waitForValue(p, QJsonObject());
        QVERIFY(res.isEmpty());
    }
    {   // QJsonValue(int) -> QJsonObject
        auto p = QPromise<QJsonValue>::resolve(QJsonValue(42)).cast<QJsonObject>();
        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QJsonObject> >::value));
        const QJsonObject res = waitForValue(p, QJsonObject());
        QVERIFY(res.isEmpty());
    }
    {   // QJsonValue(QJsonObject) -> QJsonObject
        const QJsonObject object{{"magic", 42}};
        auto p = QPromise<QJsonValue>::resolve(QJsonValue(object)).cast<QJsonObject>();
        Q_STATIC_ASSERT((std::is_same<decltype(p), QPromise<QJsonObject> >::value));
        const QJsonObject res = waitForValue(p, QJsonObject());
        QCOMPARE(res.value("magic"), 42);
    }
}
*/
