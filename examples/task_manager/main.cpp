////////////////////////////////////////////////////////////////////////////
//
// Getting started with the Barq native (C++) SDK.
//
// A short tour of everyday use: define typed models, open a local database,
// then create / read / query / update / delete objects, follow a relationship,
// and react to changes live. No server is required — this is all local-first.
//
////////////////////////////////////////////////////////////////////////////

#include <barq_native/sdk.hpp>

#include <filesystem>
#include <iostream>
#include <string>

using namespace barq::native;

// --- Models -----------------------------------------------------------------
// A Task belongs to a Project. Declare the side that holds the link (Task)
// first, then the target (Project) adds a `linking_objects<>` back-reference
// that automatically lists every Task pointing at it.
struct Project;

struct Task {
    primary_key<int64_t> _id;
    std::string title;
    bool done = false;
    int64_t priority = 0; // higher shows first
    Project* project;     // the project this task belongs to (a to-one link)
};
BARQ_SCHEMA(Task, _id, title, done, priority, project)

struct Project {
    primary_key<int64_t> _id;
    std::string name;
    linking_objects<&Task::project> tasks; // every Task whose project is this
};
BARQ_SCHEMA(Project, _id, name, tasks)

namespace {

// `results` is a live, lazily-evaluated view; take it by forwarding reference
// so both a stored query and a temporary (e.g. from `.sort(...)`) print here.
template <typename Results>
void print_tasks(Results&& tasks) {
    if (tasks.size() == 0) {
        std::cout << "    (none)\n\n";
        return;
    }
    for (size_t i = 0; i < tasks.size(); ++i) {
        auto t = tasks[i];
        std::cout << "    [" << (static_cast<bool>(t.done) ? 'x' : ' ') << "] "
                  << 'P' << static_cast<int64_t>(t.priority) << "  "
                  << static_cast<std::string>(t.title) << "\n";
    }
    std::cout << "\n";
}

} // namespace

int main() {
    auto path = std::filesystem::temp_directory_path() / "barq_task_manager_demo.barq";
    std::error_code ec;
    std::filesystem::remove(path, ec); // fresh run each time

    db_config config;
    config.set_path(path.string());
    auto barq = db(config);

    // --- Create -------------------------------------------------------------
    // Every write runs in a transaction. Assigning the one unmanaged `launch`
    // to each task links them all to the same project (objects are matched by
    // primary key, so the project is stored once).
    std::cout << "== Create ==\n\n";
    Project launch;
    launch._id = 1;
    launch.name = "Launch v1";
    barq.write([&] {
        auto add_task = [&](int64_t id, const char* title, int64_t priority, bool done) {
            Task t;
            t._id = id;
            t.title = title;
            t.priority = priority;
            t.done = done;
            t.project = &launch;
            barq.add(std::move(t));
        };
        add_task(1, "Design the schema", 3, true);
        add_task(2, "Write the SDK", 3, false);
        add_task(3, "Add tests", 2, false);
        add_task(4, "Write the docs", 1, false);
    });
    std::cout << "  seeded " << barq.objects<Task>().size() << " tasks\n\n";

    // --- Read + query -------------------------------------------------------
    // `objects<T>()` returns everything; `.sort` and `.where` refine it. The
    // `where` predicate is plain C++ — the fields build the query for you.
    std::cout << "== All tasks, highest priority first ==\n\n";
    print_tasks(barq.objects<Task>().sort("priority", /*ascending=*/false));

    std::cout << "== Still to do ==\n\n";
    print_tasks(barq.objects<Task>().where([](auto& t) { return t.done == false; }));

    // --- Update -------------------------------------------------------------
    // Fetch the object, change a field inside a write — it persists on commit.
    std::cout << "== Update: finish \"Write the SDK\" ==\n\n";
    barq.write([&] {
        auto t = barq.objects<Task>().where([](auto& t) { return t._id == 2; })[0];
        t.done = true;
    });
    print_tasks(barq.objects<Task>().where([](auto& t) { return t.done == false; }));

    // --- Relationships ------------------------------------------------------
    // Read the project back and follow its backlink to count its tasks.
    std::cout << "== Relationships ==\n\n";
    auto project = barq.objects<Project>()[0];
    std::cout << "  project \"" << static_cast<std::string>(project.name) << "\" owns "
              << project.tasks.size() << " tasks\n\n";

    // --- Realtime -----------------------------------------------------------
    // A results view is live: register an observer, and any write that changes
    // the view delivers a change set (insertions / deletions / modifications).
    std::cout << "== Realtime ==\n\n";
    auto todo = barq.objects<Task>().where([](auto& t) { return t.done == false; });
    int inserts = 0;
    auto token = todo.observe([&](auto&& change) { inserts += change.insertions.size(); });
    barq.refresh(); // deliver the initial notification

    std::cout << "  a new task is filed...\n";
    barq.write([&] {
        Task t;
        t._id = 5;
        t.title = "Ship it";
        t.priority = 3;
        t.project = &launch;
        barq.add(std::move(t));
    });
    barq.refresh(); // deliver the change
    std::cout << "  observer saw " << inserts << " insertion(s)\n\n";

    // --- Delete -------------------------------------------------------------
    std::cout << "== Delete ==\n\n";
    std::cout << "  before: " << barq.objects<Task>().size() << " tasks\n";
    barq.write([&] {
        auto t = barq.objects<Task>().where([](auto& t) { return t._id == 4; })[0];
        barq.remove(t);
    });
    std::cout << "  after:  " << barq.objects<Task>().size() << " tasks (removed \"Write the docs\")\n\n";

    std::cout << "Done.\n";
    return 0;
}
