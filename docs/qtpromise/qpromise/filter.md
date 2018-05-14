## `QPromise<T>::filter`

```
QPromise<QVector<T>>::filter(Function filter) -> QPromise<QVector<T>>
```

Takes a `Promise` that contains a `QVector<T>` of values and applies a filter function to each promise.
If the filter function resolves to a `true` then the promise will be added to the result list.

> **Note**: These filters can be chained together.

```cpp
QPromise<QVector<int>> promises = {...};

auto p = promises.filter([](int value, int index) {
            return value > 10; // Return boolean a value
        })
        .filter([](int value, int index) {
            return QPromise<bool>::resolve(value < 15); // Return a boolean as a promise
        })
        .then([](const QVector<int>& values) {
            // ...
        });
```
