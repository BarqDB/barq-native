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

#ifndef BARQ_NATIVE_DB_HPP
#define BARQ_NATIVE_DB_HPP

#include <barq_native/accessors.hpp>
#include <barq_native/macros.hpp>
#include <barq_native/results.hpp>
#include <barq_native/scheduler.hpp>
#include <barq_native/schema.hpp>
#include <barq_native/types.hpp>

#include <barq_native/internal/bridge/sync_session.hpp>
#include <barq_native/internal/bridge/thread_safe_reference.hpp>
#include <barq_native/internal/bridge/sync_session.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <utility>

namespace barq::native {
    namespace {
        template<typename T>
        using is_optional = internal::type_info::is_optional<T>;
    }
    namespace schemagen {
        template <typename Class, typename ...Properties>
        struct schema;
        template <auto Ptr, bool IsPrimaryKey, bool IsIndexed, bool IsFulltext>
        struct property;
    }

    using sync_config = internal::bridge::barq::sync_config;
    using db_config = internal::bridge::barq::config;
    using sync_session = internal::bridge::sync_session;
    using sync_error = internal::bridge::sync_error;

    struct sync_subscription_set;

    template <typename T>
    struct thread_safe_reference;
}

namespace barq::native {

    struct db {
        static inline std::vector<internal::bridge::object_schema> schemas;
        // Reconcilers registered by BARQ_SCHEMA, one per object type. A vector
        // (knn) index is local-only and never travels in the schema or over
        // sync, so instead of being a column flag it is (re)built here after the
        // schema is applied and the tables exist. Each reconciler creates its
        // type's missing vector indexes; on reopen the persisted index is
        // already there, so it is a no-op.
        static inline std::vector<void (*)(db&)> vector_index_reconcilers;

        internal::bridge::barq m_barq;
        explicit db(barq::native::db_config config)
        {
            if (!config.get_schema())
                config.set_schema(db::schemas);
            m_barq = internal::bridge::barq(config);
            reconcile_vector_indexes();
        }

        // Registered from BARQ_SCHEMA for every declared object type.
        template <typename Cls>
        static void register_vector_index_reconciler() {
            vector_index_reconcilers.push_back(&db::reconcile_vector_indexes_for<Cls>);
        }

        void begin_write() const { m_barq.begin_transaction(); }
        void commit_write() const { m_barq.commit_transaction(); }
        void cancel_write() const { m_barq.cancel_transaction(); }

        // Runs `fn` inside a write transaction. If `fn` throws, the transaction
        // is rolled back before the exception propagates, so a failed write
        // never leaves the barq stuck in an open transaction.
        template <typename Fn>
        std::invoke_result_t<Fn> write(Fn&& fn) const {
            begin_write();
            try {
                if constexpr (!std::is_void_v<std::invoke_result_t<Fn>>) {
                    auto val = fn();
                    commit_write();
                    return val;
                } else {
                    fn();
                    commit_write();
                }
            } catch (...) {
                cancel_write();
                throw;
            }
        }
        template <typename U>
        managed<std::remove_const_t<U>> add(U &&v) {
            using T = std::remove_const_t<U>;
            static_assert(sizeof(managed<T>), "Must declare schema for T");
            auto table = m_barq.table_for_object_type(managed<std::remove_const_t<T>>::schema.name);
            internal::bridge::obj m_obj;
            if constexpr (managed<std::remove_const_t<T>>::schema.HasPrimaryKeyProperty) {
                auto pk = v.*(managed<std::remove_const_t<T>>::schema.primary_key().ptr);
                m_obj = table.create_object_with_primary_key(barq::native::internal::bridge::mixed(serialize(pk.value)));
            } else {
                m_obj = table.create_object();
            }

            std::apply([&m_obj, &v, this](auto && ...p) {
                (accessor<typename std::decay_t<decltype(p)>::Result>::set(
                        m_obj, m_obj.get_table().get_column_key(p.name), m_barq, v.*(std::decay_t<decltype(p)>::ptr)
                ), ...);
            }, managed<T>::schema.ps);
            return managed<T>(std::move(m_obj), m_barq);
        }
        template <typename T>
        void remove(T& object)
        {
            auto table = m_barq.table_for_object_type(T::schema.name);
            table.remove_object(object.m_obj.get_key());
        }
        template <typename T>
        void insert(const std::vector<T> &v) {
            static_assert(sizeof(managed<T>), "Must declare schema for T");
            internal::bridge::table table = m_barq.table_for_object_type(managed<T>::schema.name);
            for (auto& obj : v) {
                internal::bridge::obj m_obj;
                if constexpr (managed<T>::schema.HasPrimaryKeyProperty) {
                    auto pk = obj.*(managed<T>::schema.primary_key().ptr);
                    m_obj = table.create_object_with_primary_key(barq::native::internal::bridge::mixed(serialize(pk.value)));
                } else {
                    m_obj = table.create_object();
                }
                std::apply([&m_obj, &obj](auto && ...p) {
                    (accessor<typename std::decay_t<decltype(p)>::Result>::set(
                            m_obj, m_obj.get_table().get_column_key(p.name), obj.*(std::decay_t<decltype(p)>::ptr)
                    ), ...);
                }, managed<T>::schema.ps);
            }
        }

    private:
        void reconcile_vector_indexes() {
            for (auto reconciler : vector_index_reconcilers)
                reconciler(*this);
        }

        // For one property: if it is vector_indexed and its index is not yet on
        // disk, record the column and the config to build.
        template <typename Property>
        static void collect_missing_vector_index(
                internal::bridge::table& table, const Property& p,
                std::vector<std::pair<internal::bridge::col_key,
                                      internal::bridge::vector_index_config>>& out) {
            using Result = typename Property::Result;
            if constexpr (internal::type_info::is_vector_indexed<Result>::value) {
                auto col = table.get_column_key(p.name);
                if (!table.has_vector_index(col)) {
                    internal::bridge::vector_index_config cfg;
                    cfg.metric = Result::metric;
                    cfg.encoding = Result::encoding;
                    cfg.dimensions = Result::dimensions;
                    out.push_back({col, cfg});
                }
            }
        }

        template <typename Cls>
        static void reconcile_vector_indexes_for(db& d) {
            auto table = d.m_barq.table_for_object_type(managed<Cls>::schema.name);
            std::vector<std::pair<internal::bridge::col_key,
                                  internal::bridge::vector_index_config>> to_create;
            std::apply([&](auto&&... p) {
                (db::collect_missing_vector_index(table, p, to_create), ...);
            }, managed<Cls>::schema.ps);
            if (to_create.empty())
                return;
            // add_vector_index needs a write; only enter one when there is work.
            // Re-acquire the table inside the transaction so the index is built
            // against the write-version table accessor, not the read one.
            d.write([&] {
                auto write_table = d.m_barq.table_for_object_type(managed<Cls>::schema.name);
                for (auto& entry : to_create)
                    write_table.add_vector_index(entry.first, entry.second);
            });
        }

        template <size_t N, typename Tpl, typename ...Ts> auto v_add(const Tpl& tpl, const std::tuple<Ts...>& vs) {
            if constexpr (N + 1 == sizeof...(Ts)) {
                auto managed = add(std::move(std::get<N>(vs)));
                return std::tuple_cat(tpl, std::make_tuple(std::move(managed)));
            } else {
                auto managed = add(std::move(std::get<N>(vs)));
                return v_add<N + 1>(std::tuple_cat(tpl, std::make_tuple(std::move(managed))), vs);
            }
        }
    public:
        template <typename ...Ts>
        std::tuple<managed<Ts>...> insert(Ts&&... v) {
            std::tuple<> tpl;
            return v_add<0>(tpl, std::make_tuple(v...));
        }
        template <typename T>
        results<T> objects()
        {
            return results<T>(internal::bridge::results(m_barq, m_barq.table_for_object_type(managed<T>::schema.name)));
        }

        [[maybe_unused]] bool refresh()
        {
            return m_barq.refresh();
        }

        ::barq::native::sync_subscription_set subscriptions();

        std::optional<sync_session> get_sync_session() const {
            return m_barq.get_sync_session();
        }

        template <typename T>
        managed<T> resolve(thread_safe_reference<T>&& tsr)
        {
            auto object = internal::bridge::resolve<internal::bridge::object>(m_barq, std::move(tsr.m_tsr));
            internal::bridge::obj m_obj = object.get_obj();
            return managed<T>(std::move(m_obj), m_barq);
        }

        bool is_frozen() const;
        db freeze();
        db thaw();
        void invalidate();
        void close();
        bool is_closed();
        friend struct ::barq::native::thread_safe_reference<db>;
        template <typename, typename> friend struct managed;
        template<typename T>
        friend void internal::bridge::barq::config::set_client_reset_handler(const client_reset_mode_base<T>&);
    private:
        db(internal::bridge::barq&& r)
        {
            m_barq = std::move(r);
        }
        db(const internal::bridge::barq& r)
        {
            m_barq = r;
        }
    };

    bool operator==(const db&, const db&);
    bool operator!=(const db&, const db&);

    template <typename ...Ts>
    inline db open(const db_config& config) {
        auto config_copy = config;
        if constexpr (sizeof...(Ts) == 0) {
            config_copy.set_schema(db::schemas);
        } else {
            std::vector<internal::bridge::object_schema> schema;
            (schema.push_back(managed<Ts>::schema.to_core_schema()), ...);
            config_copy.set_schema(schema);
        }
        return db(config_copy);
    }
    template <typename ...Ts>
    [[deprecated("This function is deprecated and is replaced by `db::open(const db_config& config)`.")]]
    inline db open(const std::string& path, const std::shared_ptr<scheduler>& scheduler) {
        return open<Ts...>(db_config(path, scheduler));
    }

    template <typename T>
    inline std::ostream& operator<< (std::ostream& stream, const T*& object)
    {
        stream << "link:" << object << std::endl;
        return stream;
    }
}

#endif //BARQ_NATIVE_DB_HPP
