# QPromiseConversionException

*Since: 0.7.0*

This exception is thrown whenever a promise conversion using `QPromise<T>::as<U>()` fails, for 
example: 

```cpp
QVariant input = ...;

QtPromise::resolve(input)
    .as<int>()
    .then([](int value) {

    })
    .fail([](const QPromiseconversionException& e) {
        // conversion may file because input could not be converted to number
    });
```
