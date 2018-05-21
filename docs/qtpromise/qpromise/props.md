## `[static] QPromise<T>::props`

```
[static] QPromise<T>::props(std::tuple<QPromise<X>,...> promises) ->; QPromise<T>
```

Returns a `QPromise<T>` that fulfills when **all** `promises` of input tuples have been fulfilled. The `input` tuple types do not have to be the same resulting type. The `output` value can be any object that will fit the resulting types of the input `promises`.

If any of the given `promises` fail, `output` immediately rejects with the error of the promise that rejected, whether or not any of the other promises have resolved.

The main usecase is to use a `struct` that matches all of the types resolved by the input `std::tuple`.  It is important to be sure that the order of the fields in the `struct` match the order of the resolved types of the input `std::tuple`.

``` cpp
struct MyObject {
    int property1;
    QString property2;
};

auto promises = std::make_tuple(
        QPromise<int>(...),
        QPromise<QString>(...)
    );

auto p = QPromise<MyObject>::props(promises)
        .then([](const MyObject& obj) {...});
```

In cases where the input doesn't require all data fields to be a `QPromise<...>` the direct value can be used.

``` cpp
struct MyObject {
    int property1;
    QString property2;
};

auto promises = std::make_tuple(
        10,
        QPromise<QString>(...)
    );

auto p = QPromise<MyObject>::props(promises)
        .then([](const MyObject& obj) {...});
```

In C++14 any object that has a compatible initilizer list can be used.

``` cpp
auto promises = std::make_tuple(
        QPromise<int>(...),
        QPromise<QString>(...)
    );

auto p = QPromise<std::tuple<int,QString>>::props(promises))
         .then([](const std::tuple<int,QString>& obj) {...});
```
