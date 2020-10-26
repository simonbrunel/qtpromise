---
title: connect
---

# QtPromise::connect

*Since: 0.5.0*

```cpp
(1) QtPromise::connect(QObject* sender, Signal(T) resolver) -> QPromise<T>
(2) QtPromise::connect(QObject* sender, Signal(T) resolver, Signal(R) rejecter) -> QPromise<T>
(3) QtPromise::connect(QObject* sender, Signal(T) resolver, QObject* sender2, Signal(R) rejecter) -> QPromise<T>
```

Creates a `QPromise<T>` that will be fulfilled with the `resolver` signal's first argument, or a
`QPromise<void>` if `resolver` doesn't provide any argument.

The second `(2)` and third `(3)` variants of this method will reject the `output` promise when the
`rejecter` signal is emitted. The rejection reason is the value of the `rejecter` signal's first
argument or [`QPromiseUndefinedException`](../exceptions/undefined.md) if `rejected` doesn't provide
any argument.

Additionally, the `output` promise will be automatically rejected with [`QPromiseContextException`](../exceptions/context.md)
if `sender` is destroyed before the promise is resolved (that doesn't apply to `sender2`).

```cpp
class Sender : public QObject
{
Q_SIGNALS:
    void finished(const QByteArray&);
    void error(ErrorCode);
};

auto sender = new Sender{};
auto output = QtPromise::connect(sender, &Sender::finished, &Sender::error);

// 'output' resolves as soon as one of the following events happens:
// - the 'sender' object is destroyed, the promise is rejected
// - the 'finished' signal is emitted, the promise is fulfilled
// - the 'error' signal is emitted, the promise is rejected

// 'output' type: QPromise<QByteArray>
output.then([](const QByteArray& res) {
    // 'res' is the first argument of the 'finished' signal.
}).fail([](ErrorCode error) {
    // 'error' is the first argument of the 'error' signal.
}).fail([](const QPromiseContextException& error) {
    // the 'sender' object has been destroyed before any of
    // the 'finished' or 'error' signals have been emitted.
});
```

Optionally, if the signal's argument can be ignored, promises returned by `QtPromise::connect()`
can be converted to `QPromise<void>`:

```cpp
QPromise<void> output = QtPromise::connect(sender, &Sender::finished, &Sender::error);

output.then([]() {
    // the first argument of the 'finished' signal is ignored.
}).fail([](ErrorCode error) {
    // 'error' is the first argument of the 'error' signal.
}).fail([](const QPromiseContextException& error) {
    // the 'sender' object has been destroyed before any of
    // the 'finished' or 'error' signals have been emitted.
});
```

Such a conversion is performed implicitly when returning a promise from a function:

```cpp
class Sender : public QObject
{
public:
    QPromise<void> execute()
    {
        auto p = QtPromise::connect(this, &Sender::finished, &Sender::error);
        executeImpl();
        return p;
    }
    
Q_SIGNALS:
    void finished(const QByteArray&);
    void error(ErrorCode);

private:
    void executeImpl() { /* ... */ }
};
```

::: warning IMPORTANT
The conversion to `QtPromise<void>` is not supported on MSVC 2013. 
:::


See also the [`Qt Signals`](../qtsignals.md) section for more examples.
