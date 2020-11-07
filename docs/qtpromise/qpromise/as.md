---
title: .as
---

# QPromise::as

*Since: 0.7.0*

```cpp
QPromise<T>::as<U>() -> QPromise<U>
```

This method converts the resolved value of `QPromise<T>` to the type `U`. For any `T` and `U` other
than `QVariant`, the result of the conversion is the same as calling `static_cast<U>` for type `T`,
including support for 
[converting constructors](https://en.cppreference.com/w/cpp/language/converting_constructor). 

```cpp
QtPromise::resolve(42)
    .as<double>()
    .then([] (double value) {
        // value is 42.0
    });

QtPromise::resolve(QByteArray{"foo"})
    .as<QString>()
    .then([](const QString& value) {
        // value has been constructed using QString(const QByteArray&)
    });
```

If `U` is `void`, the resolved value of `QPromise<T>` is dropped. 

Calling this method for `QPromise<QVariant>` tries to convert the resolved `QVariant` to type `U`
using the `QVariant` [conversion algorithm](https://doc.qt.io/qt-5/qvariant.html#using-canconvert-and-convert-consecutively). 
For example, this allows to convert a string contained in `QVariant` to number. If such a 
conversion fails, the promise is rejected with 
[`QPromiseConversionException`](../exceptions/conversion.md).

```cpp
QVariant fourtyTwo{"42"};
QVariant foo{"foo"};
QVariant input = ... ; // foutyTwo or foo

QtPromise::resolve(input)
    .as<int>()
    .then([] (int value) {
        // value is 42
    })
    .fail(const QPromiseConversionException& e) {
        // input was "foo"
    });
```

Conversion to `QVariant` using this method effectively calls `QVariant::fromValue<T>()`. All custom
types should be registered with 
[`Q_DECLARE_METATYPE`](https://doc.qt.io/qt-5/qmetatype.html#Q_DECLARE_METATYPE):

```cpp
struct Foo {};
Q_DECLARE_METATYPE(Foo);

QtPromise::resolve(Foo{})
    .as<QVariant>()
    .then([] (const QVariant& value) {
        // value contains an instance of Foo
    });
```

Calling this method for `QPromise<void>` is not supported.
