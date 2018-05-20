`[static] QPromise<T>::props`
-----------------------------

    [static] QPromise<T>::props(std::tuple<QPromise<T>...> promises) ->; QPromise<T>

``` cpp
struct MyObject {
    int property1;
    QString property2;
};

QPromise<void> getObject() {
    return QPromise<MyObject>::props(
        std::make_tuple(
            QPromise<int>(...),
            QPromise<QString>(...)
        )
    ).then([](const MyObject& obj) {...});
}
```

``` cpp
auto p = QPromise<std::tuple<int,QString>>::props(
      std::make_tuple(
          QPromise<int>(...),
          QPromise<QString>(...)
      )).then([](const std::tuple<int,QString>& obj) {...});
```
