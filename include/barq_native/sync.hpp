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

#ifndef BARQ_NATIVE_SYNC_HPP
#define BARQ_NATIVE_SYNC_HPP

#include <barq_native/db.hpp>

#include <functional>
#include <memory>
#include <string>

namespace barq::native {

    struct tenant_sync_config {
        std::string tenant_id;
        std::string user_id;
        std::string route;
        std::string access_token;
        std::string partition;
        bool route_verified = true;
    };

    struct flexible_sync_tenant_config {
        std::string tenant_id;
        std::string user_id;
        std::string route;
        std::string access_token;
        bool route_verified = true;
    };

    class sync_user {
    public:
        using access_token_refresh_handler = std::function<std::string()>;

        sync_user(std::string tenant_id, std::string user_id, std::string access_token);

        [[nodiscard]] std::string tenant_id() const;
        [[nodiscard]] std::string user_id() const;
        [[nodiscard]] std::string access_token() const;
        [[nodiscard]] std::string route() const;

        void set_route(std::string route, bool verified = true);
        void set_access_token(std::string access_token);
        void set_access_token_refresh_handler(access_token_refresh_handler handler);
        void mark_access_token_refresh_required();

        [[nodiscard]] sync_config make_sync_config(std::string partition) const;
        [[nodiscard]] sync_config make_flexible_sync_config() const;

    private:
        struct Impl;
        std::shared_ptr<Impl> m_impl;
    };

    [[nodiscard]] sync_config make_sync_config(const tenant_sync_config& config);
    [[nodiscard]] sync_config make_flexible_sync_config(const flexible_sync_tenant_config& config);

} // namespace barq::native

#endif // BARQ_NATIVE_SYNC_HPP
