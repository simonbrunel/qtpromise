<a href="https://promisesaplus.com/" title="Promises/A+ 1.1"><img src="http://promisesaplus.com/assets/logo-small.png" alt="Promises/A+" align="right"/></a>

# QtPromise
[![qpm](https://img.shields.io/github/release/simonbrunel/qtpromise.svg?style=flat-square&label=qpm&colorB=4CAF50)](http://www.qpm.io/packages/com.github.simonbrunel.qtpromise/index.html) [![Travis](https://img.shields.io/travis/simonbrunel/qtpromise.svg?style=flat-square)](https://travis-ci.org/simonbrunel/qtpromise) [![coverage](https://img.shields.io/codecov/c/github/simonbrunel/qtpromise.svg?style=flat-square)](https://codecov.io/gh/simonbrunel/qtpromise)

[Promises/A+](https://promisesaplus.com/) implementation for [Qt/C++](https://www.qt.io/).

Requires [Qt 5.4](https://www.qt.io/download/) (or later) with [C++11 support enabled](https://wiki.qt.io/How_to_use_C++11_in_your_Qt_Projects).

## Getting Started
### Installation
QtPromise is a [header-only](https://en.wikipedia.org/wiki/Header-only) library, simply download the [latest release](https://github.com/simonbrunel/qtpromise/releases/latest) (or [`git submodule`](https://git-scm.com/docs/git-submodule])) and include `qtpromise.pri` from your project `.pro`.

### qpm
Alternatively and **only** if your project relies on [qpm](http://www.qpm.io/), you can install QtPromise as follow:

```bash
qpm install com.github.simonbrunel.qtpromise
```

### Usage
The recommended way to use QtPromise is to include the single module header:

```cpp
#include <QtPromise>
```

### Example
Let's first make the code more readable by using the library namespace:

```cpp
using namespace QtPromise;
```

This `download` function creates a [promise from callbacks](#qpromise-qpromise) which will be resolved when the network request is finished:

```cpp
QPromise<QByteArray> download(const QUrl& url)
{
    return QPromise<QByteArray>([&](
        const QPromiseResolve<QByteArray>& resolve,
        const QPromiseReject<QByteArray>& reject) {

        QNetworkReply* reply = manager->get(QNetworkRequest(url));
        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            if (reply->error() == QNetworkReply::NoError) {
                resolve(reply->readAll());
            } else {
                reject(reply->error());
            }

            reply->deleteLater();
        });
    });
}
```

The following method `uncompress` data in a separate thread and returns a [promise from QFuture](#qtconcurrent):

```cpp
QPromise<Entries> uncompress(const QByteArray& data)
{
    return qPromise(QtConcurrent::run([](const QByteArray& data) {
        Entries entries;

        // {...} uncompress data and parse content.

        if (error) {
            throw MalformedException();
        }

        return entries;
    }, data));
}
```

It's then easy to chain the whole asynchronous process using promises:
- initiate the promise chain by downloading a specific URL,
- [`then`](#qpromise-then) *and only if download succeeded*, uncompress received data,
- [`then`](#qpromise-then) validate and process the uncompressed entries,
- [`finally`](#qpromise-finally) perform operations whatever the process succeeded or failed,
- and handle specific errors using [`fail`](#qpromise-fail).

```cpp
download(url).then(&uncompress).then([](const Entries& entries) {
    if (entries.isEmpty()) {
        throw UpdateException("No entries");
    }
    // {...} process entries
}).finally([]() {
    // {...} cleanup
}).fail([](QNetworkReply::NetworkError err) {
    // {...} handle network error
}).fail([](const UpdateException& err) {
    // {...} handle update error
}).fail([]() {
    // {...} catch all
});
```

## QtConcurrent
QtPromise integrates with [QtConcurrent](http://doc.qt.io/qt-5/qtconcurrent-index.html) to make easy chaining QFuture with QPromise.

### <a name="qtconcurrent-convert"></a> Convert
Converting `QFuture<T>` to `QPromise<T>` is done using the [`qPromise`](#helpers-qpromise) helper:

```cpp
QFuture<int> future = QtConcurrent::run([]() {
    // {...}
    return 42;
});

QPromise<int> promise = qPromise(future);
```

or simply:

```cpp
auto promise = qPromise(QtConcurrent::run([]() {
    // {...}
}));
```

### Chain
Returning a `QFuture<T>` in [`then`](#qpromise-then)  or [`fail`](#qpromise-fail) automatically translate to `QPromise<T>`:

```cpp
QPromise<int> input = ...
auto output = input.then([](int res) {
    return QtConcurrent::run([]() {
        // {...}
        return QString("42");
    });
});

// output type: QPromise<QString>
output.then([](const QString& res) {
    // {...}
});
```

The `output` promise is resolved when the `QFuture` is [finished](http://doc.qt.io/qt-5/qfuture.html#isFinished).

### Error
Exceptions thrown from a QtConcurrent thread reject the associated promise with the exception as the reason. Note that if you throw an exception that is not a subclass of `QException`, the promise with be rejected with [`QUnhandledException`](http://doc.qt.io/qt-5/qunhandledexception.html#details) (this restriction only applies to exceptions thrown from a QtConcurrent thread, [read more](http://doc.qt.io/qt-5/qexception.html#details)).

```cpp
QPromise<int> promise = ...
promise.then([](int res) {
    return QtConcurrent::run([]() {
        // {...}

        if (!success) {
            throw CustomException();
        }

        return QString("42");
    });
}).fail(const CustomException& err) {
    // {...}
});
```

## Thread-Safety
 QPromise is thread-safe and can be copied and accessed across different threads. QPromise relies on [explicitly data sharing](http://doc.qt.io/qt-5/qexplicitlyshareddatapointer.html#details) and thus `auto p2 = p1` represents the same promise: when `p1` resolves, handlers registered on `p1` and `p2` are called, the fulfilled value being shared between both instances.

> **Note:** while it's safe to access the resolved value from different threads using [`then`](#qpromise-then), QPromise provides no guarantee about the object being pointed to. Thread-safety and reentrancy rules for that object still apply.

## QPromise
### <a name="qpromise-qpromise"></a> `QPromise<T>::QPromise(resolver)`
Creates a new promise that will be fulfilled or rejected by the given `resolver` lambda:

```cpp
QPromise<int> promise([](const QPromiseResolve<int>& resolve, const QPromiseReject<int>& reject) {
    async_method([=](bool success, int result) {
        if (success) {
            resolve(result);
        } else {
            reject(customException());
        }
    });
});
```

> **Note:** `QPromise<void>` is specialized to not contain any value, meaning that the `resolve` callback takes no argument.

**C++14**

```cpp
QPromise<int> promise([](const auto& resolve, const auto& reject) {
    // {...}
});
```

### <a name="qpromise-then"></a> `QPromise<T>::then(onFulfilled, onRejected) -> QPromise<R>`
See [Promises/A+ `.then`](https://promisesaplus.com/#the-then-method) for details.

```cpp
QPromise<int> input = ...
auto output = input.then([](int res) {
    // called with the 'input' result if fulfilled
}, [](const ReasonType& reason) {
    // called with the 'input' reason if rejected
    // see QPromise<T>::fail for details
});
```

> **Note**: `onRejected` handler is optional, `output` will be rejected with the same reason as `input`.

> **Note**: it's recommended to use the [`fail`](#qpromise-fail) shorthand to handle errors.

The type `<R>` of the `output` promise depends on the return type of the `onFulfilled` handler:

```cpp
QPromise<int> input = {...}
auto output = input.then([](int res) {
    return QString::number(res);    // -> QPromise<QString>
});

// output type: QPromise<QString>
output.then([](const QString& res) {
    // {...}
});
```

> **Note**: only `onFulfilled` can change the promise type, `onRejected` **must** return the same type as `onFulfilled`. That also means if `onFulfilled` is `nullptr`, `onRejected` must return the same type as the `input` promise.

```cpp
QPromise<int> input = ...
auto output = input.then([](int res) {
    return res + 4;
}, [](const ReasonType& reason) {
    return -1;
});
```

If `onFulfilled` doesn't return any value, the `output` type is `QPromise<void>`:

```cpp
QPromise<int> input = ...
auto output = input.then([](int res) {
    // {...}
});

// output type: QPromise<void>
output.then([]() {
    // `QPromise<void>` `onFulfilled` handler has no argument
});
```

You can also decide to skip the promise result by omitting the handler argument:

```cpp
QPromise<int> input = {...}
auto output = input.then([]( /* skip int result */ ) {
    // {...}
});
```

The `output` promise can be *rejected* by throwing an exception in either `onFulfilled` or `onRejected`:

```cpp
QPromise<int> input = {...}
auto output = input.then([](int res) {
    if (res == -1) {
        throw ReasonType();
    } else {
        return res;
    }
});

// output.isRejected() is true
```

If an handler returns a promise (or QFuture), the `output` promise is delayed and will be resolved by the returned promise.

### <a name="qpromise-fail"></a> `QPromise<T>::fail(onRejected) -> QPromise<T>`
Shorthand to `promise.then(nullptr, onRejected)`, similar to the [`catch` statement](http://en.cppreference.com/w/cpp/language/try_catch):

```cpp
promise.fail([](const MyException&) {
    // {...}
}).fail(const QException&) {
    // {...}
}).fail(const std::exception&) {
    // {...}
}).fail() {
    // {...} catch-all
});
```

### <a name="qpromise-finally"></a> `QPromise<T>::finally(handler) -> QPromise<T>`
This `handler` is **always** called, without any argument and whatever the `input` promise state (fulfilled or rejected). The `output` promise has the same type as the `input` one but also the same value or error. The finally `handler` **can not modify the fulfilled value** (the returned value is ignored), however, if `handler` throws, `output` is rejected with the new exception.

```cpp
auto output = input.finally([]() {
    // {...}
});
```

If `handler` returns a promise (or QFuture), the `output` promise is delayed until the returned promise is resolved and under the same conditions: the delayed value is ignored, the error transmitted to the `output` promise.

### <a name="qpromise-wait"></a> `QPromise<T>::wait() -> QPromise<T>`
This method holds the execution of the remaining code **without** blocking the event loop of the current thread:

```cpp
int result = -1;
QPromise<int> input = qPromise(QtConcurrent::run([]() { return 42; }));
auto output = input.then([&](int res) {
    result = res;
});

// output.isPending() is true && result is -1

output.wait();

// output.isPending() is false && result is 42
```

### `QPromise<T>::isPending() -> bool`
Returns `true` if the promise is pending (not fulfilled or rejected), otherwise returns `false`.

### `QPromise<T>::isFulfilled() -> bool`
Returns `true` if the promise is fulfilled, otherwise returns `false`.

### `QPromise<T>::isRejected() -> bool`
Returns `true` if the promise is rejected, otherwise returns `false`.

## QPromise (statics)
### <a name="qpromise-resolve"></a> `[static] QPromise<T>::resolve(value) -> QPromise<T>`
Creates a `QPromise<T>` that is fulfilled with the given `value` of type `T`:

```cpp
QPromise<int> compute(const QString& type)
{
    if (type == "magic") {
        return QPromise<int>::resolve(42);
    }

    return QPromise<int>([](const QPromiseResolve<int>& resolve) {
        // {...}
    });
}
```

See also: [`qPromise`](#helpers-qpromise)

### <a name="qpromise-reject"></a> `[static] QPromise<T>::reject(reason) -> QPromise<T>`
Creates a `QPromise<T>` that is rejected with the given `reason` of *whatever type*:

```cpp
QPromise<int> compute(const QString& type)
{
    if (type == "foobar") {
        return QPromise<int>::reject(QString("Unknown type: %1").arg(type));
    }

    return QPromise<int>([](const QPromiseResolve<int>& resolve) {
        // {...}
    });
}
```

### <a name="qpromise-all"></a> `[static] QPromise<T>::all(QVector<QPromise<T>>) -> QPromise<QVector<T>>`
Returns a `QPromise<QVector<T>>` that fulfills when **all** `promises` of (the same) type `T` have been fulfilled. The `output` value is a vector containing **all** the values of `promises`, in the same order. If any of the given `promises` fail, `output` immediately rejects with the error of the promise that rejected, whether or not the other promises are resolved.

```cpp
QVector<QPromise<QByteArray> > promises{
    download(QUrl("http://a...")),
    download(QUrl("http://b...")),
    download(QUrl("http://c..."))
};

auto output = QPromise<QByteArray>::all(promises);

// output type: QPromise<QVector<QByteArray>>
output.then([](const QVector<QByteArray>& res) {
    // {...}
});
```

See also: [`qPromiseAll`](#helpers-qpromiseall)

## Helpers
### <a name="helpers-qpromise"></a> `qPromise(T value) -> QPromise<R>`
Similar to the `QPromise<T>::resolve` static method, creates a promise resolved from a given `value` without the extra typing:

```cpp
auto promise = qPromise();                // QPromise<void>
auto promise = qPromise(42);              // QPromise<int>
auto promise = qPromise(QString("foo"));  // QPromise<QString>
```

This method also allows to convert `QFuture<T>` to `QPromise<T>` delayed until the `QFuture` is finished ([read more](#qtconcurrent-convert)).

### <a name="helpers-qpromiseall"></a> `qPromiseAll(QVector<QPromise<T> promises) -> QPromise<QVector<T>>`
This method simply calls the appropriated [`QPromise<T>::all`](#qpromise-all) static method based on the given `QVector` type. In some cases, this method is more convenient than the static one since it avoid some extra typing:

```cpp
QVector<QPromise<QByteArray> > promises{...}

auto output = qPromiseAll(promises);
// eq. QPromise<QByteArray>::all(promises)
```

## License
QtPromise is available under the [MIT license](LICENSE).
