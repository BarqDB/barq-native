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

#ifndef BARQ_NATIVE_BRIDGE_RESULTS_HPP
#define BARQ_NATIVE_BRIDGE_RESULTS_HPP

#include <cstddef>
#include <barq_native/internal/bridge/obj.hpp>
#include <barq_native/internal/bridge/utils.hpp>

namespace barq {
    class Results;
}
namespace barq::native::internal::bridge {
    struct barq;
    struct table;
    struct query;
    struct notification_token;
    struct obj;
    struct collection_change_set;

    struct sort_descriptor {
        std::string key_path;
        bool ascending;
    private:
        friend struct results;
        friend struct list;
        operator std::pair<std::string, bool>() const;
    };

    struct results {
        results();
        results(const results& other) ;
        results& operator=(const results& other) ;
        results(results&& other);
        results& operator=(results&& other);
        ~results();

        results(const Results&); //NOLINT(google-explicit-constructor)
        results(const barq&, const table_view&);
        size_t size();
        [[nodiscard]] barq get_barq() const;
        [[nodiscard]] table get_table() const;
        results(const barq&, const query&);
        results sort(const std::vector<sort_descriptor>&);
        notification_token add_notification_callback(std::shared_ptr<collection_change_callback>&&);
    private:
        template <typename T>
        friend T get(results&, size_t);
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        storage::Results m_results[1];
#else
        std::shared_ptr<Results> m_results;
#endif
    };

    template <typename T>
    T get(results&, size_t);
    template <>
    obj get(results&, size_t);
    template <>
    int64_t get(results&, size_t);
    template <>
    bool get(results&, size_t);
    template <>
    double get(results&, size_t);
    template <>
    float get(results&, size_t);
    template <>
    std::string get(results&, size_t);
    template <>
    ::barq::native::uuid get(results&, size_t);
    template <>
    ::barq::native::object_id get(results&, size_t);
    template <>
    ::barq::native::decimal128 get(results&, size_t);
    template <>
    std::vector<uint8_t> get(results&, size_t);
    template <>
    std::chrono::time_point<std::chrono::system_clock> get(results&, size_t);
    template <>
    mixed get(results&, size_t);
}

#endif //BARQ_NATIVE_BRIDGE_RESULTS_HPP
