## `QPromise<T>::wait`

```
QPromise<T>::wait() -> QPromise<T>
```

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
