## `QPromise<T>::reduce`

```
QPromise<T>::reduce(Function reducer, R initial) -> QPromise<R>
```

Takes a `Promise` that contains a `QVector<T>` of values and applies a reducing function to it
and returns a new `QPromise<R>` with the reduced result.

> **Note**: Promises are processed in the order that they are in the input vector.

```cpp
QPromise<QVector<int>> promises = {...};

auto p = promises.reduce([](int value, int current, int index) {
            return value + current;  // Return as value or promise
        }, 0);
        .then([](int value) {
            // ...
        });
```
