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

#include <barq/object-store/sync/token_sync_user.hpp>

#include <utility>

// The tenant/token SyncUser and its config builders live in core
// (barq::TokenSyncUser) so that every Barq client shares one implementation.
// This file is a thin idiomatic C++ wrapper over it; the Kotlin/other SDKs wrap
// the same core type through the C API (barq_sync_user_* in core's c_api).

namespace barq::native {

struct sync_user::Impl {
    std::shared_ptr<::barq::TokenSyncUser> user;
};

sync_user::sync_user(std::string tenant_id, std::string user_id, std::string access_token)
    : m_impl(std::make_shared<Impl>())
{
    m_impl->user = ::barq::TokenSyncUser::create(std::move(tenant_id), std::move(user_id), std::move(access_token));
}

std::string sync_user::tenant_id() const
{
    return m_impl->user->tenant_id();
}

std::string sync_user::user_id() const
{
    return m_impl->user->user_id();
}

std::string sync_user::access_token() const
{
    return m_impl->user->access_token();
}

std::string sync_user::route() const
{
    return m_impl->user->route();
}

void sync_user::set_route(std::string route, bool verified)
{
    m_impl->user->set_route(std::move(route), verified);
}

void sync_user::set_access_token(std::string access_token)
{
    m_impl->user->set_access_token(std::move(access_token));
}

void sync_user::set_access_token_refresh_handler(access_token_refresh_handler handler)
{
    m_impl->user->set_access_token_refresh_handler(std::move(handler));
}

void sync_user::mark_access_token_refresh_required()
{
    m_impl->user->mark_access_token_refresh_required();
}

sync_config sync_user::make_sync_config(std::string partition) const
{
    return sync_config(m_impl->user->make_sync_config(std::move(partition)));
}

sync_config sync_user::make_flexible_sync_config() const
{
    return sync_config(m_impl->user->make_flexible_sync_config());
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
