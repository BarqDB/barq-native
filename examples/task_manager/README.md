# Task manager

A getting-started tour of the Barq native C++ SDK — the everyday flow you reach
for in almost any app.

It defines two typed models (a `Task` and the `Project` it belongs to), opens a
local database, and then walks through:

- **Create** — write objects inside a transaction with `barq.write([&]{ ... })`.
- **Read & query** — `barq.objects<Task>()`, refined with `.sort(...)` and a
  plain-C++ `.where([](auto& t){ return t.done == false; })` predicate.
- **Update** — fetch an object and assign a field inside a write.
- **Relationships** — a to-one link (`Project* project`) plus a
  `linking_objects<&Task::project>` back-reference that lists a project's tasks.
- **Realtime** — `results.observe(...)` delivers a change set whenever a write
  affects the query, no polling.
- **Delete** — `barq.remove(obj)` inside a write.

```cpp
struct Task {
    barq::native::primary_key<int64_t> _id;
    std::string title;
    bool done = false;
    int64_t priority = 0;
    Project* project;
};
BARQ_SCHEMA(Task, _id, title, done, priority, project)

auto todo = db.objects<Task>().where([](auto& t) { return t.done == false; });
auto token = todo.observe([&](auto&& change) { /* react to inserts/edits */ });
```

## Build & run

The example is built as part of the SDK. From the SDK root:

```sh
cmake -G Ninja -S . -B build -DBARQ_CORE_SOURCE_DIR=/path/to/barq-core
cmake --build build --target task_manager
./build/examples/task_manager/task_manager
```

Set `-DBARQ_NATIVE_NO_EXAMPLES=ON` to skip building the examples.
