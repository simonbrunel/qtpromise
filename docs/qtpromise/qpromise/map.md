## `QPromise<T>::map`

```
QPromise<T>::map(Function mapper) -> QPromise<QVector<R>>
```

Takes a `Promise` that contains a `QVector<T>` of values and applies a mapping function to each promise
and returns a new `QVector<R>` of results.

> **Note**: These mappers can be chained together as well as change the return type of the promise.

```cpp
QPromise<QVector<int>> promises = {...};

auto p = promises.map([](int value, int index) {
            return value + 1; // Return a value
        })
        .map([](int value, int index) {
            return QPromise<int>::resolve(value + 1); // Return as a promise
        })
        .map([](int value, int index) {
            return QString::number(value); // Change type
        })
        .then([](const QVector<QString>& values) {
            // ...
        });
```
