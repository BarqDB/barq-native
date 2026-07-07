#include <barq_native/sync.hpp>

#include <barq/object-store/sync/generic_network_transport.hpp>
#include <barq/object-store/sync/sync_manager.hpp>
#include <barq/object-store/sync/sync_user.hpp>
#include <barq/sync/config.hpp>

#ifdef BARQ_NATIVE_HAVE_SYNC_SERVER_TESTS
#include <barq/sync/noinst/server/server.hpp>
#endif

#include "../main.hpp"
#include "test_objects.hpp"

#include <chrono>
#include <filesystem>
#include <future>
#include <thread>

namespace barq::native {
struct NativeOwnedOrder {
    primary_key<int64_t> _id;
    std::string owner_id;
    std::string item;
};
BARQ_SCHEMA(NativeOwnedOrder, _id, owner_id, item)
} // namespace barq::native

using namespace barq::native;

#ifdef BARQ_NATIVE_HAVE_SYNC_SERVER_TESTS
namespace {

constexpr const char* user_0_path_test_token =
    "ewogICAgImlkZW50aXR5IjogInVzZXJfMCIsCiAgICAiYWRtaW4iOiBmYWxzZSwKICAgICJ0aW1l"
    "c3RhbXAiOiAxNDU1NTMwNjE0LAogICAgImV4cGlyZXMiOiBudWxsLAogICAgImFwcF9pZCI6ICJp"
    "by5yZWFsbS5UZXN0IiwKICAgICJwYXRoIjogIi90ZXN0IiwKICAgICJhY2Nlc3MiOiBbImRvd25s"
    "b2FkIiwgInVwbG9hZCIsICJtYW5hZ2UiXQp9Cg==";

constexpr const char* user_1_path_test_token =
    "ewogICAgImlkZW50aXR5IjogInVzZXJfMSIsCiAgICAiYWRtaW4iOiBmYWxzZSwKICAgICJ0aW1l"
    "c3RhbXAiOiAxNDU1NTMwNjE0LAogICAgImV4cGlyZXMiOiBudWxsLAogICAgImFwcF9pZCI6ICJp"
    "by5yZWFsbS5UZXN0IiwKICAgICJwYXRoIjogIi90ZXN0IiwKICAgICJhY2Nlc3MiOiBbImRvd25s"
    "b2FkIiwgInVwbG9hZCIsICJtYW5hZ2UiXQp9Cg==";

std::string make_server_root(barq_path& base)
{
    std::string path = std::string(base) + "-server";
    std::filesystem::create_directories(path);
    return path;
}

barq::sync::Server::Config make_native_flx_server_config()
{
    barq::sync::Server::Config config;
    config.logger = barq::util::Logger::get_default_logger();
    config.enable_flx_sync = true;
    config.flx_rules.push_back(
        {"NativeOwnedOrder", barq::sync::Server::Config::FLXRule::Mode::Owner, "owner_id"});
    config.connection_reaper_timeout = 100000000;
    config.connection_reaper_interval = 100000000;
    return config;
}

struct NativeFLXServer {
    barq_path root_base;
    std::string root_dir;
    barq::sync::Server server;
    std::thread thread;
    std::exception_ptr server_error;

    NativeFLXServer()
        : root_dir(make_server_root(root_base))
        , server(root_dir, barq::util::none, make_native_flx_server_config())
    {
        server.start("localhost", "");
        thread = std::thread([this] {
            try {
                server.run();
            }
            catch (...) {
                server_error = std::current_exception();
            }
        });
    }

    ~NativeFLXServer()
    {
        server.stop();
        if (thread.joinable()) {
            thread.join();
        }
        std::filesystem::remove_all(root_dir);
    }

    std::string route() const
    {
        return "ws://localhost:" + std::to_string(server.listen_endpoint().port()) + "/barq-sync";
    }
};

void require_ready(std::future<void>& future)
{
    REQUIRE(future.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    future.get();
}

bool require_ready(std::future<bool>& future)
{
    REQUIRE(future.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return future.get();
}

db open_native_flx_db(const std::string& route, const char* token, barq_path& path)
{
    sync_user user("io.realm.Test", "device", token);
    user.set_route(route);

    db_config config;
    config.set_path(path);
    config.set_sync_config(user.make_flexible_sync_config());

    return open<NativeOwnedOrder>(config);
}

void subscribe_to_orders(db& barq)
{
    auto subscriptions = barq.subscriptions();
    auto complete = subscriptions.update([](mutable_sync_subscription_set& mutable_subscriptions) {
        mutable_subscriptions.add<NativeOwnedOrder>("orders");
    });
    CHECK(require_ready(complete));
}

void wait_for_sync(db& barq)
{
    auto session = barq.get_sync_session();
    REQUIRE(session);

    auto upload = session->wait_for_upload_completion();
    require_ready(upload);

    auto download = session->wait_for_download_completion();
    require_ready(download);

    barq.refresh();
}

void add_order(db& barq, int64_t id, const std::string& owner_id, const std::string& item)
{
    barq.write([&] {
        NativeOwnedOrder order;
        order._id = id;
        order.owner_id = owner_id;
        order.item = item;
        barq.add(std::move(order));
    });
}

void check_one_order(db& barq, const std::string& owner_id, const std::string& item)
{
    auto orders = barq.objects<NativeOwnedOrder>();
    REQUIRE(orders.size() == 1);
    CHECK(orders[0].owner_id == owner_id);
    CHECK(orders[0].item == item);
}

} // unnamed namespace
#endif

TEST_CASE("tenant sync config wires route token tenant and partition", "[sync]") {
    sync_user user("tenant-a", "device-1", "token-1");
    user.set_route("ws://127.0.0.1:9090/barq-sync");

    auto sync = user.make_sync_config("shared");
    auto core = static_cast<std::shared_ptr<::barq::SyncConfig>>(sync);

    REQUIRE(core);
    REQUIRE(core->user);
    CHECK(core->partition_value == "shared");
    CHECK(core->user->app_id() == "tenant-a");
    CHECK(core->user->user_id() == "device-1");
    CHECK(core->user->access_token() == "token-1");

    auto route = core->user->sync_manager()->sync_route();
    CHECK(route.first == "ws://127.0.0.1:9090/barq-sync");
    CHECK(route.second);

    user.set_access_token("token-2");
    CHECK(core->user->access_token() == "token-2");

    int refresh_count = 0;
    user.set_access_token_refresh_handler([&] {
        ++refresh_count;
        return std::string("token-3");
    });
    user.mark_access_token_refresh_required();
    CHECK(core->user->access_token_refresh_required());

    bool completed = false;
    core->user->request_access_token([&](std::optional<::barq::networking::NetworkError> error) {
        CHECK(!error);
        completed = true;
    });

    CHECK(completed);
    CHECK(refresh_count == 1);
    CHECK(core->user->access_token() == "token-3");
    CHECK_FALSE(core->user->access_token_refresh_required());
}

TEST_CASE("flexible sync config wires route token tenant without partition", "[sync]") {
    sync_user user("tenant-a", "device-1", "token-1");
    CHECK_THROWS_AS(user.make_flexible_sync_config(), std::logic_error);

    user.set_route("ws://127.0.0.1:9090/barq-sync");
    auto sync = user.make_flexible_sync_config();
    auto core = static_cast<std::shared_ptr<::barq::SyncConfig>>(sync);

    REQUIRE(core);
    REQUIRE(core->user);
    CHECK(core->flx_sync_requested);
    CHECK(core->partition_value.empty());
    CHECK(core->user->app_id() == "tenant-a");
    CHECK(core->user->user_id() == "device-1");
    CHECK(core->user->access_token() == "token-1");

    flexible_sync_tenant_config config;
    config.tenant_id = "tenant-b";
    config.user_id = "device-2";
    config.access_token = "token-2";
    config.route = "ws://127.0.0.1:9090/barq-sync";
    auto sync_from_config = make_flexible_sync_config(config);
    auto core_from_config = static_cast<std::shared_ptr<::barq::SyncConfig>>(sync_from_config);
    REQUIRE(core_from_config);
    CHECK(core_from_config->flx_sync_requested);
    CHECK(core_from_config->partition_value.empty());
    CHECK(core_from_config->user->app_id() == "tenant-b");
}

TEST_CASE("flexible sync subscriptions update commits named queries", "[sync]") {
    barq_path path;

    sync_user user("tenant-a", "device-1", "token-1");
    user.set_route("ws://127.0.0.1:9090/barq-sync");

    db_config config;
    config.set_path(path);
    config.set_sync_config(user.make_flexible_sync_config());

    auto barq = open<Person, Dog>(config);
    auto subscriptions = barq.subscriptions();
    CHECK(subscriptions.size() == 0);

    auto add_complete = subscriptions.update([](mutable_sync_subscription_set& mutable_subscriptions) {
        mutable_subscriptions.add<Person>("adults", [](managed<Person>& person) {
            return person.age > 18;
        });
    });
    (void)add_complete;

    REQUIRE(subscriptions.size() == 1);
    auto adults = subscriptions.find("adults");
    REQUIRE(adults);
    REQUIRE(adults->name);
    CHECK(*adults->name == "adults");
    CHECK(adults->object_class_name == "Person");
    CHECK(adults->query_string.find("age") != std::string::npos);
    CHECK(adults->query_string.find("18") != std::string::npos);

    auto update_complete = subscriptions.update([](mutable_sync_subscription_set& mutable_subscriptions) {
        mutable_subscriptions.update_subscription<Person>("adults", [](managed<Person>& person) {
            return person.age > 21;
        });
    });
    (void)update_complete;

    auto updated_adults = subscriptions.find("adults");
    REQUIRE(updated_adults);
    CHECK(updated_adults->query_string.find("21") != std::string::npos);

    auto remove_complete = subscriptions.update([](mutable_sync_subscription_set& mutable_subscriptions) {
        mutable_subscriptions.remove("adults");
    });
    (void)remove_complete;

    CHECK(subscriptions.size() == 0);
    CHECK_FALSE(subscriptions.find("adults"));
}

#ifdef BARQ_NATIVE_HAVE_SYNC_SERVER_TESTS
TEST_CASE("flexible sync two native users share one server file with filtered views", "[sync]") {
    NativeFLXServer server;
    barq_path user_0_path;
    barq_path user_1_path;

    auto user_0_db = open_native_flx_db(server.route(), user_0_path_test_token, user_0_path);
    auto user_1_db = open_native_flx_db(server.route(), user_1_path_test_token, user_1_path);

    subscribe_to_orders(user_0_db);
    subscribe_to_orders(user_1_db);
    wait_for_sync(user_0_db);
    wait_for_sync(user_1_db);

    add_order(user_0_db, 1, "user_0", "apples");
    add_order(user_1_db, 2, "user_1", "oranges");

    wait_for_sync(user_0_db);
    wait_for_sync(user_1_db);
    wait_for_sync(user_0_db);
    wait_for_sync(user_1_db);

    check_one_order(user_0_db, "user_0", "apples");
    check_one_order(user_1_db, "user_1", "oranges");

    CHECK_FALSE(server.server_error);
    CHECK(server.server.errors_seen() == 0);
}
#endif

TEST_CASE("tenant sync config rejects invalid inputs", "[sync]") {
    CHECK_THROWS_AS(sync_user("", "device-1", "token"), std::invalid_argument);
    CHECK_THROWS_AS(sync_user("tenant-a", "", "token"), std::invalid_argument);
    CHECK_THROWS_AS(sync_user("tenant-a", "device-1", ""), std::invalid_argument);

    sync_user user("tenant-a", "device-1", "token");
    CHECK_THROWS_AS(user.make_sync_config("shared"), std::logic_error);

    user.set_route("ws://127.0.0.1:9090/barq-sync");
    CHECK_THROWS_AS(user.make_sync_config(""), std::invalid_argument);
    CHECK_THROWS_AS(user.make_sync_config("/tenant-a/shared"), std::invalid_argument);
}
