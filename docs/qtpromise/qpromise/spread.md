## `[static] QPromise<T>::spread`

```
[static] QPromise<T>::spread(std::tuple<QPromise<X>,...> promises, Function fn) -> QPromise<T>
```

Returns a `QPromise<T>` from a `Function` that is executed when **all** `promises` of input tuples have been fulfilled. The `input` tuple types do not have to be the same resulting type. The `Function` can be anything that accepts the resulting types of the input `promises`.

If any of the given `promises` fail, `output` immediately rejects with the error of the promise that rejected, whether or not any of the other promises have resolved.  In this case the `Function` will not be executed.

The main usecase is to use a `Function` whos signature matches all of the types resolved by the input `std::tuple`.  It is important to be sure that the order of the arguments in the `Function` match the order of the resolved types of the input `std::tuple`.

``` cpp
auto promises = std::make_tuple(
        QPromise<int>(...),
        QPromise<QString>(...)
    );

auto p = QPromise<QString>::spread(promises,
            [](int arg1, const QString &arg2) {
                // return a value or a promise
            })
         ).then([](const QString &result) {...});
```

In cases where the input doesn't require all values in the `std::tuple` to be a `QPromise<...>` the direct value can be used.

``` cpp
auto promises = std::make_tuple(
        10,
        QPromise<QString>(...)
    );

auto p = QPromise<QString>::spread(promises,
            [](int arg1, const QString &arg2) {
                // return a value or a promise
            })
         ).then([](const QString &result) {...});
```
