# Barq Native

Barq Native is the C++ SDK for BarqDB. It builds on top of
[barq-core](https://github.com/BarqDB/barq-core).

## Build

```sh
cmake -S . -B build -DBARQ_NATIVE_NO_TESTS=ON
cmake --build build --target barq_native
```

By default, CMake fetches `https://github.com/BarqDB/barq-core.git`.

To force a specific core source:

```sh
cmake -S . -B build -DBARQ_CORE_SOURCE_DIR=/path/to/barq-core
```

To force a git revision:

```sh
cmake -S . -B build -DBARQ_CORE_GIT_TAG=main
```

## Use

```cpp
#include <barq_native/sdk.hpp>

using namespace barq;

struct Dog {
    std::string name;
    int64_t age;
};

BARQ_SCHEMA(Dog, name, age)

int main() {
    db_config config;
    auto barq = db(std::move(config));

    auto dog = Dog{.name = "Rex", .age = 1};
    auto managed_dog = barq.write([&] {
        return barq.add(std::move(dog));
    });
}
```
