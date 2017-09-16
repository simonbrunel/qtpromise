#ifndef QTQMLPROMISE_QJSPROMISE_H
#define QTQMLPROMISE_QJSPROMISE_H

// QtQmlPromise
#include "qtqmlpromiseglobal.h"

// QtPromise
#include <QtPromise>

// Qt
#include <QJSValue>

namespace QtPromise {

class QTQMLPROMISE_EXPORT QJSPromise
{
    Q_GADGET

public:
    QJSPromise();
    QJSPromise(QJSEngine* engine, QJSValue resolver);
    QJSPromise(QPromise<QJSValue>&& promise);

    Q_INVOKABLE bool isFulfilled() const { return m_promise.isFulfilled(); }
    Q_INVOKABLE bool isRejected() const { return m_promise.isRejected(); }
    Q_INVOKABLE bool isPending() const{ return m_promise.isPending(); }

    Q_INVOKABLE QtPromise::QJSPromise then(QJSValue fulfilled, QJSValue rejected = QJSValue()) const;
    Q_INVOKABLE QtPromise::QJSPromise fail(QJSValue handler) const;
    Q_INVOKABLE QtPromise::QJSPromise finally(QJSValue handler) const;
    Q_INVOKABLE QtPromise::QJSPromise tap(QJSValue handler) const;
    Q_INVOKABLE QtPromise::QJSPromise delay(int msec) const;
    Q_INVOKABLE QtPromise::QJSPromise wait() const;

public: // STATICS
    static QJSPromise resolve(QJSValue&& value);
    static QJSPromise reject(QJSValue&& error);
    static QJSPromise all(QJSEngine* engine, QJSValue&& input);

private:
    friend struct QtPromisePrivate::PromiseFulfill<QJSValue>;

    QPromise<QJSValue> m_promise;

}; // class QJSPromise

} // namespace QtPromise

Q_DECLARE_TYPEINFO(QtPromise::QJSPromise, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(QtPromise::QJSPromise)

#endif // ifndef QTQMLPROMISE_QJSPROMISE_H
