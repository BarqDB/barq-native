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

#ifndef BARQ_NATIVE_BRIDGE_SYNC_MANAGER_HPP
#define BARQ_NATIVE_BRIDGE_SYNC_MANAGER_HPP

#include <memory>
#include <string>
#include <barq_native/logger.hpp>
#include <barq_native/internal/bridge/barq.hpp>

namespace barq {
    class SyncManager;
}

namespace barq::native {
    struct user;
}

namespace barq::native::internal::bridge {
        struct sync_manager {
            void set_log_level(logger::level);
        private:
            friend struct ::barq::native::user;
            sync_manager(const std::shared_ptr<SyncManager> &);
            std::shared_ptr<SyncManager> m_manager;
        };
}

#endif //BARQ_NATIVE_BRIDGE_SYNC_MANAGER_HPP
