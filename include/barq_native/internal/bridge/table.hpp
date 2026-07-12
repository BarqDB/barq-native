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

#ifndef BARQ_NATIVE_BRIDGE_TABLE_HPP
#define BARQ_NATIVE_BRIDGE_TABLE_HPP

#include <string>
#include <vector>
#include <barq_native/internal/bridge/obj_key.hpp>
#include <barq_native/internal/bridge/vector_index.hpp>

namespace barq {
    class TableRef;
    class ConstTableRef;
    class Mixed;
    class TableView;
}

namespace barq::native::internal::bridge {
        struct col_key;
        struct link_chain;
        struct mixed;
        struct obj;
        struct query;

        struct table {
            table();
            table(const table& other) ;
            table& operator=(const table& other) ;
            table(table&& other);
            table& operator=(table&& other);
            ~table();
            table(const TableRef &);
            table(const ConstTableRef &);
            operator TableRef() const;
            operator ConstTableRef() const;

            // False when this wraps a null TableRef — e.g. the object type is
            // not part of the opened file's schema.
            [[nodiscard]] bool is_valid() const;

            col_key get_column_key(const std::string_view &name) const;
            uint32_t get_key() const;
            std::string get_name() const;

            obj create_object_with_primary_key(const mixed &key) const;

            obj create_object(const obj_key &obj_key = {}) const;

            table get_link_target(const col_key col_key) const;
            link_chain get_link(const col_key col_key) const;

            [[nodiscard]] bool is_embedded() const;

            struct query query(const std::string &, const std::vector <mixed>&) const;
            struct query where() const;

            void remove_object(const obj_key &) const;
            obj get_object(const obj_key&) const;
            bool is_valid(const obj_key&) const;

            // Vector (knn) index management. The index is a local, on-disk HNSW
            // structure over a list-of-float column; it is never synced.
            void add_vector_index(const col_key& col_key, const vector_index_config& config) const;
            void add_vector_index(const col_key& col_key) const;
            void remove_vector_index(const col_key& col_key) const;
            [[nodiscard]] bool has_vector_index(const col_key& col_key) const;
            // Returns the persisted config; throws if the column has no vector index.
            [[nodiscard]] vector_index_config get_vector_index_config(const col_key& col_key) const;

            using underlying = TableRef;
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        storage::TableRef m_table[1];
#else
        std::shared_ptr<TableRef> m_table;
#endif
        };

        struct table_view {
            table_view();
            table_view(const table_view& other) ;
            table_view& operator=(const table_view& other) ;
            table_view(table_view&& other);
            table_view& operator=(table_view&& other);
            ~table_view();
            table_view(const TableView &);
            operator TableView() const;
            using underlying = TableView;
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
            storage::TableView m_table_view[1];
#else
            std::shared_ptr<TableView> m_table_view;
#endif
        };

        bool operator==(const table &, const table &);

        bool operator!=(const table &, const table &);
}

#endif //BARQ_NATIVE_BRIDGE_TABLE_HPP
