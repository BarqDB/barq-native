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

## License And Attribution

Barq Native is a modified fork of the
[Realm C++ SDK](https://github.com/realm/realm-cpp). The source files have been
changed from their original form as part of the Barq project; see the Git
history for details. The original Realm Inc. copyright headers are retained.

The code is licensed under the Apache License 2.0. See `LICENSE` and `NOTICE`.

Barq is an independent project and is not affiliated with, sponsored by, or
endorsed by Realm, MongoDB, Inc., or MongoDB Atlas. "Realm", "MongoDB", and
"MongoDB Atlas" are trademarks of MongoDB, Inc.
