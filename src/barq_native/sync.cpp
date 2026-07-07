////////////////////////////////////////////////////////////////////////////
//
// Copyright 2024 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#include <barq_native/sync.hpp>

#include <barq/object-store/sync/generic_network_transport.hpp>
#include <barq/object-store/sync/sync_manager.hpp>
#include <barq/object-store/sync/sync_user.hpp>
#include <barq/sync/config.hpp>

#include <barq/error_codes.hpp>

#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

namespace barq::native {
namespace {

void validate_non_empty(const std::string& value, const char* name)
{
    if (value.empty()) {
        throw std::invalid_argument(std::string{name} + " must not be empty");
    }
}

void validate_partition(const std::string& partition)
{
    validate_non_empty(partition, "partition");
    if (partition.front() == '/') {
        throw std::invalid_argument("partition must be relative to the tenant");
    }
}

} // unnamed namespace

struct sync_user::Impl final : public ::barq::SyncUser {
    Impl(std::string tenant_id, std::string user_id, std::string access_token)
        : m_tenant_id(std::move(tenant_id))
        , m_user_id(std::move(user_id))
        , m_access_token(std::move(access_token))
        , m_manager(::barq::SyncManager::create(::barq::SyncClientConfig{}))
    {
        validate_non_empty(m_tenant_id, "tenant_id");
        validate_non_empty(m_user_id, "user_id");
        validate_non_empty(m_access_token, "access_token");
    }

    std::string user_id() const noexcept override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_user_id;
    }

    std::string app_id() const noexcept override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_tenant_id;
    }

    std::string access_token() const override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_access_token;
    }

    std::string refresh_token() const override
    {
        return {};
    }

    State state() const override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_state;
    }

    bool access_token_refresh_required() const override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_access_token_refresh_required;
    }

    ::barq::SyncManager* sync_manager() override
    {
        return m_manager.get();
    }

    void request_log_out() override
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_state = State::LoggedOut;
        }
        m_manager->update_sessions_for(*this, State::LoggedIn, State::LoggedOut, {});
    }

    void request_refresh_location(CompletionHandler&& completion) override
    {
        completion(std::nullopt);
    }

    void request_access_token(CompletionHandler&& completion) override
    {
        access_token_refresh_handler handler;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            handler = m_refresh_handler;
        }

        if (!handler) {
            completion(std::nullopt);
            return;
        }

        try {
            set_access_token(handler());
            completion(std::nullopt);
        }
        catch (const std::exception& e) {
            completion(::barq::networking::NetworkError(::barq::ErrorCodes::RuntimeError, e.what()));
        }
    }

    void track_barq(std::string_view path) override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tracked_barqs.emplace_back(path);
    }

    std::string create_file_action(::barq::SyncFileAction, std::string_view, std::optional<std::string>) override
    {
        return {};
    }

    std::string tenant_id() const
    {
        return app_id();
    }

    std::string route() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_route;
    }

    void set_route(std::string route, bool verified)
    {
        validate_non_empty(route, "route");
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_route = route;
        }
        m_manager->set_sync_route(std::move(route), verified);
    }

    void set_access_token(std::string access_token)
    {
        validate_non_empty(access_token, "access_token");
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_access_token = access_token;
            m_access_token_refresh_required = false;
            m_state = State::LoggedIn;
        }
        m_manager->update_sessions_for(*this, State::LoggedIn, State::LoggedIn, access_token);
    }

    void set_access_token_refresh_handler(access_token_refresh_handler handler)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_refresh_handler = std::move(handler);
    }

    void mark_access_token_refresh_required()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_access_token_refresh_required = true;
    }

    bool has_route() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return !m_route.empty();
    }

private:
    mutable std::mutex m_mutex;
    std::string m_tenant_id;
    std::string m_user_id;
    std::string m_access_token;
    std::string m_route;
    bool m_access_token_refresh_required = false;
    State m_state = State::LoggedIn;
    access_token_refresh_handler m_refresh_handler;
    std::vector<std::string> m_tracked_barqs;
    std::shared_ptr<::barq::SyncManager> m_manager;
};

sync_user::sync_user(std::string tenant_id, std::string user_id, std::string access_token)
    : m_impl(std::make_shared<Impl>(std::move(tenant_id), std::move(user_id), std::move(access_token)))
{
}

std::string sync_user::tenant_id() const
{
    return m_impl->tenant_id();
}

std::string sync_user::user_id() const
{
    return m_impl->user_id();
}

std::string sync_user::access_token() const
{
    return m_impl->access_token();
}

std::string sync_user::route() const
{
    return m_impl->route();
}

void sync_user::set_route(std::string route, bool verified)
{
    m_impl->set_route(std::move(route), verified);
}

void sync_user::set_access_token(std::string access_token)
{
    m_impl->set_access_token(std::move(access_token));
}

void sync_user::set_access_token_refresh_handler(access_token_refresh_handler handler)
{
    m_impl->set_access_token_refresh_handler(std::move(handler));
}

void sync_user::mark_access_token_refresh_required()
{
    m_impl->mark_access_token_refresh_required();
}

sync_config sync_user::make_sync_config(std::string partition) const
{
    validate_partition(partition);
    if (!m_impl->has_route()) {
        throw std::logic_error("sync route must be set before making a sync config");
    }
    auto config = std::make_shared<::barq::SyncConfig>(m_impl, std::move(partition));
    return sync_config(config);
}

sync_config sync_user::make_flexible_sync_config() const
{
    if (!m_impl->has_route()) {
        throw std::logic_error("sync route must be set before making a sync config");
    }
    auto config = std::make_shared<::barq::SyncConfig>(m_impl, ::barq::SyncConfig::FLXSyncEnabled{});
    return sync_config(config);
}

sync_config make_sync_config(const tenant_sync_config& config)
{
    sync_user user(config.tenant_id, config.user_id, config.access_token);
    user.set_route(config.route, config.route_verified);
    return user.make_sync_config(config.partition);
}

sync_config make_flexible_sync_config(const flexible_sync_tenant_config& config)
{
    sync_user user(config.tenant_id, config.user_id, config.access_token);
    user.set_route(config.route, config.route_verified);
    return user.make_flexible_sync_config();
}

} // namespace barq::native
