////////////////////////////////////////////////////////////////////////////
//
// Copyright 2022 Realm Inc.
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

#ifndef BARQ_NATIVE_FLEXIBLE_SYNC_HPP
#define BARQ_NATIVE_FLEXIBLE_SYNC_HPP

#include <future>
#include <type_traits>

#include <barq_native/internal/bridge/obj.hpp>
#include <barq_native/internal/bridge/query.hpp>
#include <barq_native/internal/bridge/schema.hpp>
#include <barq_native/internal/bridge/barq.hpp>
#include <barq_native/internal/bridge/utils.hpp>

#include <barq_native/macros.hpp>
#include <barq_native/results.hpp>
#include <barq_native/rbool.hpp>

namespace barq::sync {
    class MutableSubscriptionSet;
    class SubscriptionSet;
    class SubscriptionStore;
    class Subscription;
}

namespace barq::native {
    template <typename>
    struct object;
    class rbool;

// A SyncSubscription represents a single query that may be OR'd with other queries on the same object class to be
// send to the server in a QUERY or IDENT message.
    struct sync_subscription {
        // Returns the unique ID for this subscription.
        std::string identifier;
        // The name representing this subscription.
        std::optional<std::string> name;
        // Returns the timestamp of when this subscription was originally created.
        std::chrono::time_point<std::chrono::system_clock> created_at;
        // Returns the timestamp of the last time this subscription was updated by calling update_query.
        std::chrono::time_point<std::chrono::system_clock> updated_at;
        // Returns a stringified version of the query associated with this subscription.
        std::string query_string;
        // Returns the name of the object class of the query for this subscription.
        std::string object_class_name;
    private:
        sync_subscription(const ::barq::sync::Subscription&);

        friend struct sync_subscription_set;
        friend struct mutable_sync_subscription_set;
    };

// A MutableSyncSubscriptionSet represents a single query that may be OR'd with other queries on the same object class to be
// send to the server in a QUERY or IDENT message.
    struct mutable_sync_subscription_set {
    private:
        void insert_or_assign(const std::string& name, const internal::bridge::query&);
    public:
        mutable_sync_subscription_set() = delete;
        mutable_sync_subscription_set(const mutable_sync_subscription_set& other) = delete;
        mutable_sync_subscription_set& operator=(const mutable_sync_subscription_set& other) ;
        mutable_sync_subscription_set(mutable_sync_subscription_set&& other) = delete;
        mutable_sync_subscription_set& operator=(mutable_sync_subscription_set&& other);
        ~mutable_sync_subscription_set();

        // Inserts a new subscription into the set if one does not exist already.
        // If the `query_fn` parameter is left empty, the subscription will sync *all* objects
        // for the templated class type.
        template<typename T>
        void add(const std::string &name ,
                 std::optional<std::function<rbool(managed<T> &)>> &&query_fn = std::nullopt) {
            static_assert(sizeof(managed<T>), "Must declare schema for T");

            auto schema = m_barq.get().schema().find(managed<T>::schema.name);
            auto group = m_barq.get().read_group();
            auto table_ref = group.get_table(schema.table_key());
            auto root_query = internal::bridge::query(table_ref);

            if (query_fn) {
                rbool query = rbool(std::move(root_query));
                auto query_object = managed<T>::prepare_for_query(m_barq, &query);
                auto full_query = (*query_fn)(query_object).q;
                insert_or_assign(name, full_query);
            } else {
                insert_or_assign(name, root_query);
            }
        }

        // Removes a subscription for a given name. Will throw if subscription does
        // not exist.
        void remove(const std::string& name);

        // Finds a subscription for a given name. Will return `std::nullopt` is subscription does
        // not exist.
        std::optional<sync_subscription> find(const std::string& name);

        // Updates a subscription for a given name.
        // Will throw if subscription does not exist.
        // If the `query_fn` parameter is left empty, the subscription will sync *all* objects
        // for the templated class type.
        template <typename T>
        std::enable_if_t<std::is_base_of_v<object<T>, T>>
        update_subscription(const std::string& name, std::optional<std::function<rbool(T&)>>&& query_fn = std::nullopt) {
            remove(name);
            add(name, std::move(query_fn));
        }

        // Updates a subscription for a given name.
        // Will throw if subscription does not exist.
        // If the `query_fn` parameter is left empty, the subscription will sync *all* objects
        // for the templated class type.
        template <typename T>
        void update_subscription(const std::string& name,
                                 std::optional<std::function<rbool(managed<T>&)>>&& query_fn = std::nullopt) {
            remove(name);
            add(name, std::move(query_fn));
        }

        // Removes all subscriptions.
        void clear();

    private:
        mutable_sync_subscription_set(internal::bridge::barq&, const ::barq::sync::MutableSubscriptionSet& subscription_set);
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        internal::bridge::storage::MutableSyncSubscriptionSet m_subscription_set[1];
#else
        std::shared_ptr<::barq::sync::MutableSubscriptionSet> m_subscription_set;
#endif
        std::reference_wrapper<internal::bridge::barq> m_barq;
        friend struct sync_subscription_set;
        ::barq::sync::MutableSubscriptionSet get_subscription_set();
    };

    struct sync_subscription_set {
    public:
        sync_subscription_set() = delete;
        sync_subscription_set(const sync_subscription_set& other) = delete;
        sync_subscription_set& operator=(const sync_subscription_set& other) ;
        sync_subscription_set(sync_subscription_set&& other) = delete;
        sync_subscription_set& operator=(sync_subscription_set&& other);
        ~sync_subscription_set();

        /// The total number of subscriptions in the set.
        [[nodiscard]] size_t size() const;

        // Finds a subscription for a given name. Will return `std::nullopt` is subscription does
        // not exist.
        std::optional<sync_subscription> find(const std::string& name);

        std::future<bool> update(std::function<void(mutable_sync_subscription_set&)>&& fn);

        explicit sync_subscription_set(internal::bridge::barq& barq);
    private:
        friend struct db;
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        internal::bridge::storage::SyncSubscriptionSet m_subscription_set[1];
#else
        std::shared_ptr<::barq::sync::SubscriptionSet> m_subscription_set;
#endif
        std::reference_wrapper<internal::bridge::barq> m_barq;
    };

} // namespace barq

#endif /* BARQ_NATIVE_FLEXIBLE_SYNC_HPP */
