////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2026 the Barq authors
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

#ifndef BARQ_NATIVE_MANAGED_VECTOR_HPP
#define BARQ_NATIVE_MANAGED_VECTOR_HPP

#include <barq_native/macros.hpp>
#include <barq_native/vector.hpp>
#include <barq_native/internal/bridge/list.hpp>

#include <cstddef>
#include <vector>

namespace barq::native {
    template <typename, typename>
    struct managed;

    // MARK: - vector_indexed managed accessor
    //
    // A `vector_indexed<Dims, Metric, Encoding>` property stores a list of
    // floats and is read and written as a whole vector. Unlike
    // `managed<std::vector<T>>` it deliberately exposes no element-level
    // mutators (no push_back / set / erase): an embedding is replaced
    // wholesale, never partially edited. Reads and the query-time knn are the
    // supported operations.
    template <size_t Dims,
              internal::bridge::vector_metric Metric,
              internal::bridge::vector_encoding Encoding>
    struct managed<vector_indexed<Dims, Metric, Encoding>> : managed_base {
        using Wrapped = vector_indexed<Dims, Metric, Encoding>;

        // Whole-vector assignment. A `Wrapped` converts to `std::vector<float>`,
        // so this single overload also accepts the wrapper and braced lists.
        managed& operator=(const std::vector<float>& v) {
            m_obj->set_list_values(m_key, v);
            return *this;
        }

        [[nodiscard]] std::vector<float> value() const {
            auto list = internal::bridge::list(*m_barq, *m_obj, m_key);
            size_t count = list.size();
            std::vector<float> ret;
            ret.reserve(count);
            for (size_t i = 0; i < count; i++) {
                ret.push_back(internal::bridge::get<float>(list, i));
            }
            return ret;
        }

        [[nodiscard]] Wrapped detach() const {
            return Wrapped(value());
        }
        [[nodiscard]] operator std::vector<float>() const // NOLINT(google-explicit-constructor)
        {
            return value();
        }

        [[nodiscard]] size_t size() const {
            return internal::bridge::list(*m_barq, *m_obj, m_key).size();
        }
        [[nodiscard]] float operator[](size_t idx) const {
            auto list = internal::bridge::list(*m_barq, *m_obj, m_key);
            return internal::bridge::get<float>(list, idx);
        }

    private:
        managed() = default;
        managed(const managed&) = delete;
        managed(managed&&) = delete;
        managed& operator=(const managed&) = delete;
        managed& operator=(managed&&) = delete;
        template <typename, typename>
        friend struct managed;
    };
}

#endif // BARQ_NATIVE_MANAGED_VECTOR_HPP
