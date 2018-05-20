`[static] QPromise<T>::spread`
------------------------------

    [static] QPromise<T>::spread(std::tuple<QPromise<T>...> promises, Function fn) -> QPromise<T>

``` cpp
auto p = QPromise<QString>::spread(
             std::make_tuple(
                 QPromise<int>(...),
                 QPromise<QString>(...)
             ),
             [](int arg1, const QString &arg2) {
                 return QString();
             })
         ).then([](const QString &result) {...});
```
