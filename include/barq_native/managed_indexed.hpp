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

#ifndef BARQ_NATIVE_MANAGED_INDEXED_HPP
#define BARQ_NATIVE_MANAGED_INDEXED_HPP

#include <barq_native/macros.hpp>

#include <barq_native/managed_string.hpp>
#include <barq_native/managed_numeric.hpp>
#include <barq_native/managed_timestamp.hpp>
#include <barq_native/managed_uuid.hpp>
#include <barq_native/managed_objectid.hpp>
#include <barq_native/managed_mixed.hpp>

#include <string>
#include <type_traits>

namespace barq::native {
    template <typename, typename>
    struct managed;

    // MARK: - index property wrappers
    //
    // Declaring a struct field as `indexed<T>` or `fulltext<T>` marks the
    // matching column for a search index in the schema. At runtime an indexed
    // column reads, writes, and queries exactly like a plain `T` column, so the
    // wrappers are transparent value holders and the managed accessors below
    // just reuse the plain `managed<T>` behaviour.
    //
    // Note: index wrappers are for non-optional scalar properties. Core rejects
    // an index on a non-indexable type when the schema is applied, and
    // `fulltext<>` is constrained to `std::string` at compile time.

    // A property with a general search index, e.g. `indexed<int64_t> score;`.
    template <typename T>
    struct indexed {
        using internal_type = T;
        indexed() = default;
        indexed(const T& v) // NOLINT(google-explicit-constructor)
            : value(v)
        {
        }
        indexed& operator=(const T& v)
        {
            value = v;
            return *this;
        }
        T value{};
        operator T() const // NOLINT(google-explicit-constructor)
        {
            return value;
        }
    };

    // String specialisation adds the usual `const char*` conveniences.
    template <>
    struct indexed<std::string> {
        using internal_type = std::string;
        indexed() = default;
        indexed(const std::string& v) // NOLINT(google-explicit-constructor)
            : value(v)
        {
        }
        indexed(const char* v) // NOLINT(google-explicit-constructor)
            : value(v)
        {
        }
        indexed& operator=(const std::string& v)
        {
            value = v;
            return *this;
        }
        indexed& operator=(const char* v)
        {
            value = v;
            return *this;
        }
        std::string value;
        operator std::string() const // NOLINT(google-explicit-constructor)
        {
            return value;
        }
    };

    // A string property with a full-text index, e.g. `fulltext<std::string> bio;`.
    template <typename T>
    struct fulltext {
        static_assert(std::is_same_v<T, std::string>,
                      "fulltext<> indexes are only supported on std::string properties");
        using internal_type = std::string;
        fulltext() = default;
        fulltext(const std::string& v) // NOLINT(google-explicit-constructor)
            : value(v)
        {
        }
        fulltext(const char* v) // NOLINT(google-explicit-constructor)
            : value(v)
        {
        }
        fulltext& operator=(const std::string& v)
        {
            value = v;
            return *this;
        }
        fulltext& operator=(const char* v)
        {
            value = v;
            return *this;
        }
        std::string value;
        operator std::string() const // NOLINT(google-explicit-constructor)
        {
            return value;
        }
    };

    // MARK: - managed accessors
    //
    // `managed<T>` is a dependent base here, so it only needs to be a complete
    // type when a concrete `managed<indexed<T>>` is instantiated (which is when
    // a user's schema is compiled, with every `managed<T>` already visible).

    template <typename T>
    struct managed<indexed<T>> : managed<T> {
        using managed<T>::operator=;

    private:
        managed() = default;
        managed(const managed&) = delete;
        managed(managed&&) = delete;
        managed& operator=(const managed&) = delete;
        managed& operator=(managed&&) = delete;
        template <typename, typename>
        friend struct managed;
    };

    template <typename T>
    struct managed<fulltext<T>> : managed<T> {
        using managed<T>::operator=;

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

#endif // BARQ_NATIVE_MANAGED_INDEXED_HPP
