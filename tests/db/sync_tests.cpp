#include <barq_native/sync.hpp>

#include <barq/object-store/sync/generic_network_transport.hpp>
#include <barq/object-store/sync/sync_manager.hpp>
#include <barq/object-store/sync/sync_user.hpp>
#include <barq/sync/config.hpp>

#include "../main.hpp"

using namespace barq::native;

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
