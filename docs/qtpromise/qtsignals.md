## Creating `QPromise<T>` from signals

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
