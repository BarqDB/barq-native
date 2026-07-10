# Barq Native

[![License](https://img.shields.io/github/license/BarqDB/barq-native)](./LICENSE)
![Status](https://img.shields.io/badge/status-alpha-f7c948)
![C++](https://img.shields.io/badge/C%2B%2B-native-00599c)

The C++ SDK for [BarqDB](https://github.com/BarqDB) — a local-first database with real-time sync. Barq Native gives C++ apps a typed object database backed by [barq-core](https://github.com/BarqDB/barq-core).

## Features

- Typed C++ models via schema macros
- Local reads and writes backed by barq-core
- Sync-facing types bridged to the core sync client

## Build

```sh
cmake -S . -B build -DBARQ_NATIVE_NO_TESTS=ON
cmake --build build --target barq_native
```

By default CMake fetches barq-core from `https://github.com/BarqDB/barq-core.git`. To build against a local checkout or a pinned revision instead:

```sh
cmake -S . -B build -DBARQ_CORE_SOURCE_DIR=/path/to/barq-core   # local source
cmake -S . -B build -DBARQ_CORE_GIT_TAG=v20.1.6                 # pinned core release
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

## License and attribution

Barq Native is a modified fork of the [Realm C++ SDK](https://github.com/realm/realm-cpp), licensed under the [Apache License 2.0](./LICENSE). The original Realm Inc. copyright headers are retained in every file they came from; see the Git history for what changed. Fork and third-party notices are in [`NOTICE`](./NOTICE).

Barq is an independent project. It is not affiliated with, sponsored by, or endorsed by Realm, MongoDB, Inc., or MongoDB Atlas. "Realm", "MongoDB", and "MongoDB Atlas" are trademarks of MongoDB, Inc.
