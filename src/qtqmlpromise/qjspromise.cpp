// QtQmlPromise
#include "qjspromise.h"
#include "qjspromise_p.h"

// Qt
#include <QJSValueIterator>
#include <QJSEngine>

using namespace QtPromise;

namespace {

QJSValue toJSArray(QJSEngine* engine, const QVector<QJSValue>& values)
{
    Q_ASSERT(engine);

    const int count = values.count();
    QJSValue array = engine->newArray(count);

    for (int i = 0; i < count; ++i) {
        array.setProperty(i, values[i]);
    }

    return array;
}

template <typename T>
QJSValue newError(const QJSValue& source, const T& value)
{
    QJSEngine* engine = source.engine();
    QJSValue error = engine->evaluate("throw new Error('')");
    error.setProperty("message", engine->toScriptValue(value));
    return error;
}

} // anonymous namespace

QJSPromise::QJSPromise()
    : m_promise(QPromise<QJSValue>::resolve(QJSValue()))
{ }

QJSPromise::QJSPromise(QPromise<QJSValue>&& promise)
    : m_promise(std::move(promise))
{
}

QJSPromise::QJSPromise(QJSEngine* engine, QJSValue resolver)
    : m_promise([=](
        const QPromiseResolve<QJSValue>& resolve,
        const QPromiseReject<QJSValue>& reject) mutable {

            // resolver is part of the Promise wrapper in qtqmlpromise.js
            Q_ASSERT(resolver.isCallable());
            Q_ASSERT(engine);

            auto proxy = QtPromisePrivate::JSPromiseResolver(resolve, reject);
            auto ret = resolver.call(QJSValueList() <<  engine->toScriptValue(proxy));
            if (ret.isError()) {
                throw ret;
            }
        })
{ }

QJSPromise QJSPromise::then(QJSValue fulfilled, QJSValue rejected) const
{
    const bool fulfillable = fulfilled.isCallable();
    const bool rejectable = rejected.isCallable();

    if (!fulfillable && !rejectable) {
        return *this;
    }

    auto _rejected = [=]() mutable {
        QJSValue error;

        try {
            throw;
        } catch (const QJSValue& err) {
            error = err;
        } catch (const QVariant& err) {
            error = newError(rejected, err);
        } catch (const std::exception& e) {
            error = newError(rejected, QString(e.what()));
        } catch (...) {
            error = newError(rejected, QString("Unknown error"));
        }

        return rejected.call({error});
    };

    if (!fulfillable) {
        return m_promise.then(nullptr, _rejected);
    }

    auto _fulfilled = [=](const QJSValue& res) mutable {
        return fulfilled.call(QJSValueList() << res);
    };

    if (!rejectable) {
        return m_promise.then(_fulfilled);
    }

    return m_promise.then(_fulfilled, _rejected);
}

QJSPromise QJSPromise::fail(QJSValue handler) const
{
    return then(QJSValue(), handler);
}

QJSPromise QJSPromise::finally(QJSValue handler) const
{
    return m_promise.finally([=]() mutable {
        return handler.call();
    });
}

QJSPromise QJSPromise::tap(QJSValue handler) const
{
    return m_promise.tap([=](const QJSValue& res) mutable {
        return handler.call(QJSValueList() << res);
    });
}

QJSPromise QJSPromise::delay(int msec) const
{
    return m_promise.delay(msec);
}

QJSPromise QJSPromise::wait() const
{
    return m_promise.wait();
}

QJSPromise QJSPromise::resolve(QJSValue&& value)
{
    return QPromise<QJSValue>::resolve(std::forward<QJSValue>(value));
}

QJSPromise QJSPromise::reject(QJSValue&& error)
{
    return QPromise<QJSValue>::reject(std::forward<QJSValue>(error));
}

QJSPromise QJSPromise::all(QJSEngine* engine, QJSValue&& input)
{
    Q_ASSERT(engine);

    if (!input.isArray()) {
        // TODO TYPEERROR!
        return QPromise<QJSValue>::reject("foobar");
    }

    Q_ASSERT(input.hasProperty("length"));
    const int count = input.property("length").toInt();
    if (!count) {
        return QPromise<QJSValue>::resolve(QJSValue(input));
    }

    QList<QPromise<QJSValue> > promises;
    for (int i = 0; i < count; ++i) {
        QJSValue value = input.property(i);
        const QVariant variant = value.toVariant();
        if (variant.userType() == qMetaTypeId<QJSPromise>()) {
            promises << variant.value<QJSPromise>().m_promise;
        } else {
            promises << QPromise<QJSValue>::resolve(std::move(value));
        }
    }

    return QPromise<QJSValue>::all(promises)
        .then([engine](const QVector<QJSValue>& results) {
            return toJSArray(engine, results);
        });
}
