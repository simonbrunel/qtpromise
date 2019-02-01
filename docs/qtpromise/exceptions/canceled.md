---
title: QPromiseCanceledException
---

# QPromiseCanceledException

*Since: 0.1.0*

This exception is thrown for promise created from a [`QFuture`](../qtconcurrent.md)
which has been canceled (e.g. using [`QFuture::cancel()`](http://doc.qt.io/qt-5/qfuture.html#cancel)).
Note that QtPromise doesn't support promise cancelation yet. For example:

```cpp
auto output = qPromise(future)
    .fail([](const QPromiseCanceledException&) {
        // `future` has been canceled!
    });
```

