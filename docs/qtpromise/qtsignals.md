## Qt Signals

QtPromise supports creating promises that are resolved or rejected by regular Qt signals using the `QtPromise::connect()` helper:

```cpp
// single resolve signal
::connect(sender, resolveSignal)
// resolve and reject signal
::connect(sender, resolveSignal, rejectSignal)
// resolve and reject from different objects
::connect(sender1, resolveSignal, sender2, rejectSignal)
```

`QtPromise::connect()` creates a `QPromise<T>` that resolves with the signal's argument:
```cpp
// void stringSignal(const QString&)
QPromise<QString> p = QtPromise::connect(
    obj, &Object::stringSignal
);
p.then([](const QString& value) {
    // resolved with `value` from stringSignal
});
```
`QPromise<void>` is created for signals with no arguments.

A rejection is added to the promise by providing a second signal:
```cpp
// void intRejectSignal(int)
QPromise<void> p = QtPromise::connect(
    obj, &Object::voidResolveSignal, &Object::intRejectSignal,
);
p.fail([](int value) {
    // rejected with `value` from intRejectSignal
});
```

Rejections from signals with no arguments are caught by `QPromiseSignalException`:
```cpp
// void voidRejectSignal()
QPromise<void> p = QtPromise::connect(
    obj, &Object::voidResolveSignal, &Object::voidRejectSignal
);
p.fail([](const QPromiseSignalException&) {
    // rejected by voidRejectSignal
});
```

Promises created by `QtPromise::connect()` are automatically rejected with `QPromiseContextException` if the sender is destroyed before fulfilling the promise:
```cpp
// void voidRejectSignal()
QPromise<void> p = QtPromise::connect(
    obj, &Object::voidResolveSignal
);
p.then([]() {
    // promise fulfilled
}).fail([](const QPromiseContextException&) {
    // rejected by obj destruction
});
```

## Manually creating `QPromise<T>` from signals

When creating promises from from signals, care must be taken to guarantee that all signals involved in the promise creation are disconnected after fulfillment.

The easiest option is to simply delete the signal source after the promise is handled:
```cpp
// in promise returning method
return QPromise<void>([&](const auto& resolve, const auto& reject) {
    QObject::connect(obj, &Object::signal, [=]() {
        // {... resolve/reject ...}
        object->deleteLater();
    });
});
```

Sometimes the deletion of the signal source may not be possible. In this case, all signal connections must be tracked and disconnected manually. QtPromise provides the `QPromiseConnections` helper to handle this conveniently:
```cpp
// in promise returning method
QPromiseConnections connections;
return QPromise<void>([&](const auto& resolve, const auto& reject) {
    connections << QObject::connect(obj1, &Object::signal1, [=]() {
        // {... resolve/reject ...}
    });
    connections << QObject::connect(obj2, &Object::signal2, [=]() {
        // {... resolve/reject ...}
    });
})
// timeout in case no signals are emitted at all
.timeout(2000)
// clean up signal connections when finished
.finally([=]() { connections.disconnect(); });
```
