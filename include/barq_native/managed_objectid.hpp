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

#ifndef BARQ_NATIVE_MANAGED_OBJECTID_HPP
#define BARQ_NATIVE_MANAGED_OBJECTID_HPP

#include <barq_native/macros.hpp>
#include <barq_native/types.hpp>

#include <barq_native/internal/bridge/object_id.hpp>

namespace barq::native {
    class rbool;
}

namespace barq::native {
    template<>
    struct managed<barq::native::object_id> : managed_base {
        using managed<barq::native::object_id>::managed_base::operator=;
        [[nodiscard]] barq::native::object_id detach() const {
            return m_obj->template get<barq::native::internal::bridge::object_id>(m_key).operator ::barq::native::object_id();
        }

        [[nodiscard]] barq::native::object_id operator *() const {
            return detach();
        }

        [[nodiscard]] operator barq::native::object_id() const {
            return detach();
        }

        //MARK: -   comparison operators
        rbool operator==(const barq::native::object_id& rhs) const noexcept;
        rbool operator!=(const barq::native::object_id& rhs) const noexcept;

    private:
        managed() = default;
        managed(const managed&) = delete;
        managed(managed &&) = delete;
        managed& operator=(const managed&) = delete;
        managed& operator=(managed&&) = delete;
        template<typename, typename>
        friend struct managed;
    };

    template<>
    struct managed<std::optional<barq::native::object_id>> : managed_base {
        using managed<std::optional<barq::native::object_id>>::managed_base::operator=;

        [[nodiscard]] std::optional<barq::native::object_id> detach() const {
            auto v = m_obj->template get_optional<barq::native::internal::bridge::object_id>(m_key);
            if (v) {
                return v.value().operator ::barq::native::object_id();
            } else {
                return std::nullopt;
            }
        }

        [[nodiscard]] std::optional<barq::native::object_id> operator *() const {
            return detach();
        }

        [[nodiscard]] operator std::optional<barq::native::object_id>() const {
            return detach();
        }

        //MARK: -   comparison operators
        rbool operator==(const std::optional<barq::native::object_id>& rhs) const noexcept;
        rbool operator!=(const std::optional<barq::native::object_id>& rhs) const noexcept;

    private:
        managed() = default;
        managed(const managed&) = delete;
        managed(managed &&) = delete;
        managed& operator=(const managed&) = delete;
        managed& operator=(managed&&) = delete;
        template<typename, typename>
        friend struct managed;
    };

} // namespace barq

#endif//BARQ_NATIVE_MANAGED_OBJECTID_HPP
