## `QPromise<T>::each`

```
QPromise<T>::each(Function function) -> QPromise<QVector<R>>
```

Takes a `Promise` that contains a `QVector<T>` of values and runs a function on each element in the sequence.
The resulting `QVector<R>` is identical to the input `QVector<T>`.

> **Note**: These functions can be chained together.

```cpp
QPromise<QVector<int>> promises = {...};

auto p = promises.each([](int value, int index) {
            {...}  // Do some synchronus action
        })
        .each([](int value, int index) {
            return QPromise<int>(...); // Do some asynchronus action
        })
        .then([](const QVector<int>& values) {
            // ...  original vector back
        });
```
