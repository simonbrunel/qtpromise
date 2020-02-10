---
title: .delay
---

# QPromise::delay

*Since: 0.2.0*

```cpp
QPromise<T>::delay(int msec) -> QPromise<T>
```

This method returns a promise that will be fulfilled with the same value as the `input` promise
and after at least `msec` milliseconds. If the `input` promise is rejected, the `output` promise
is immediately rejected with the same reason.

```cpp
QPromise<int> input = {...}
auto output = input.delay(2000).then([](int res) {
    // called 2 seconds after `input` is fulfilled
});
```

*Since: 0.6.0*

```cpp
QPromise<T>::delay(std::chrono::milliseconds msec) -> QPromise<T>
```

This is a convenience overload accepting durations from the the standard C++ library. It is available only if your compiler supports the `<chrono>` header. 
This method returns a promise that will be fulfilled with the same value as the `input` promise
and after at least `msec` milliseconds. If the `input` promise is rejected, the `output` promise
is immediately rejected with the same reason.

```cpp
QPromise<int> input = {...}
auto output = input.delay(std::chrono::seconds{2}).then([](int res) {
    // called 2 seconds after `input` is fulfilled
});
```

C++14 alternative:

```cpp
using namespace std::chrono_literals;

QPromise<int> input = {...}
auto output = input.delay(2s).then([](int res) {
    // called 2 seconds after `input` is fulfilled
});
```
