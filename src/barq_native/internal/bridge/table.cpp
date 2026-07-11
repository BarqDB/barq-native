#include <barq_native/internal/bridge/table.hpp>
#include <barq_native/internal/bridge/mixed.hpp>
#include <barq_native/internal/bridge/obj.hpp>
#include <barq_native/internal/bridge/query.hpp>
#include <barq_native/internal/bridge/utils.hpp>

#include <barq/mixed.hpp>
#include <barq/table.hpp>
#include <barq/table_view.hpp>
#include <barq/index_vector.hpp>

#include <stdexcept>

namespace barq::native::internal::bridge {

    table::table() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_table) TableRef();
#else
        m_table = std::make_shared<TableRef>();
#endif
    }

    table::table(const table& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_table) TableRef(*reinterpret_cast<const TableRef*>(&other.m_table));
#else
        m_table = other.m_table;
#endif
    }

    table& table::operator=(const table& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<TableRef*>(&m_table) = *reinterpret_cast<const TableRef*>(&other.m_table);
        }
#else
        m_table = other.m_table;
#endif
        return *this;
    }

    table::table(table&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_table) TableRef(std::move(*reinterpret_cast<TableRef*>(&other.m_table)));
#else
        m_table = std::move(other.m_table);
#endif
    }

    table& table::operator=(table&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<TableRef*>(&m_table) = std::move(*reinterpret_cast<TableRef*>(&other.m_table));
        }
#else
        m_table = std::move(other.m_table);
#endif
        return *this;
    }

    table::~table() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<TableRef*>(&m_table)->~TableRef();
#endif
    }
    table::table(const TableRef & ref) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_table) TableRef(ref);
#else
        m_table = std::make_shared<TableRef>(ref);
#endif
    }

    table::table(const ConstTableRef &ref) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_table) ConstTableRef(ref);
#else
        m_table = std::make_shared<TableRef>(ref.cast_away_const());
#endif
    }

    table::operator TableRef() const {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return *reinterpret_cast<const TableRef*>(&m_table);
#else
        return *m_table;
#endif
    }

    table::operator ConstTableRef() const {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return *reinterpret_cast<const ConstTableRef*>(&m_table);
#else
        return *m_table;
#endif
    }

    bool table::is_embedded() const {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return (*reinterpret_cast<const TableRef*>(&m_table))->is_embedded();
#else
        return m_table->cast_away_const()->is_embedded();
#endif
    }

    query table::query(const std::string& a,
                       const std::vector<mixed>& b) const {
        std::vector<Mixed> v2;
        for (auto& v : b) {
            v2.push_back(v.operator ::barq::Mixed());
        }
        return static_cast<TableRef>(*this)->query(a, v2);
    }

    obj table::create_object(const obj_key &obj_key) const {
        return static_cast<TableRef>(*this)->create_object(obj_key);
    }

    void table::remove_object(const obj_key & v) const {
        return static_cast<TableRef>(*this)->remove_object(v);
    }

    col_key table::get_column_key(const std::string_view &name) const {
        return static_cast<TableRef>(*this)->get_column_key(name);
    }

    uint32_t table::get_key() const {
        return static_cast<TableRef>(*this)->get_key().value;
    }

    std::string table::get_name() const {
        return static_cast<TableRef>(*this)->get_name();
    }

    table table::get_link_target(const col_key col_key) const {
        return static_cast<TableRef>(*this)->get_link_target(col_key);
    }

    link_chain table::get_link(const col_key col_key) const {
        return static_cast<const TableRef>(*this)->link(col_key);
    }

    obj table::create_object_with_primary_key(const bridge::mixed& key) const {
        return static_cast<TableRef>(*this)->create_object_with_primary_key(key.operator ::barq::Mixed());
    }
    bool table::is_valid(const obj_key &key) const {
        return static_cast<TableRef>(*this)->is_valid(key);
    }
    obj table::get_object(const obj_key &key) const {
        return static_cast<TableRef>(*this)->get_object(key);
    }

    static ::barq::VectorIndexConfig to_core_vector_config(const vector_index_config& config) {
        ::barq::VectorIndexConfig core_config;
        core_config.metric = static_cast<::barq::VectorMetric>(static_cast<uint8_t>(config.metric));
        core_config.encoding = static_cast<::barq::VectorEncoding>(static_cast<uint8_t>(config.encoding));
        core_config.dimensions = config.dimensions;
        core_config.m = config.m;
        core_config.ef_construction = config.ef_construction;
        core_config.ef_search = config.ef_search;
        core_config.build_threads = config.build_threads;
        return core_config;
    }

    void table::add_vector_index(const col_key& col_key, const vector_index_config& config) const {
        static_cast<TableRef>(*this)->add_vector_index(col_key, to_core_vector_config(config));
    }
    void table::add_vector_index(const col_key& col_key) const {
        static_cast<TableRef>(*this)->add_vector_index(col_key);
    }
    void table::remove_vector_index(const col_key& col_key) const {
        static_cast<TableRef>(*this)->remove_vector_index(col_key);
    }
    bool table::has_vector_index(const col_key& col_key) const {
        return static_cast<TableRef>(*this)->has_vector_index(col_key);
    }
    vector_index_config table::get_vector_index_config(const col_key& col_key) const {
        auto* index = static_cast<TableRef>(*this)->get_vector_index(col_key);
        if (!index) {
            throw std::logic_error("Column has no vector index");
        }
        const auto& core_config = index->config();
        vector_index_config config;
        config.metric = static_cast<vector_metric>(static_cast<uint8_t>(core_config.metric));
        config.encoding = static_cast<vector_encoding>(static_cast<uint8_t>(core_config.encoding));
        config.dimensions = core_config.dimensions;
        config.m = core_config.m;
        config.ef_construction = core_config.ef_construction;
        config.ef_search = core_config.ef_search;
        config.build_threads = core_config.build_threads;
        return config;
    }

    bool operator ==(table const& lhs, table const& rhs) {
        return static_cast<TableRef>(lhs) == static_cast<TableRef>(rhs);
    }
    bool operator!=(table const &lhs, table const &rhs) {
        return static_cast<TableRef>(lhs) != static_cast<TableRef>(rhs);
    }

    table_view::table_view() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_table_view) TableView();
#else
        m_table_view = std::make_shared<TableView>();
#endif
    }
    table_view::table_view(const table_view& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_table_view) TableView(*reinterpret_cast<const TableRef*>(&other.m_table_view));
#else
        m_table_view = other.m_table_view;
#endif
    }
    table_view& table_view::operator=(const table_view& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<TableView*>(&m_table_view) = *reinterpret_cast<const TableView*>(&other.m_table_view);
        }
#else
        m_table_view = other.m_table_view;
#endif
        return *this;
    }
    table_view::table_view(table_view&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_table_view) TableView(std::move(*reinterpret_cast<TableView*>(&other.m_table_view)));
#else
        m_table_view = std::move(other.m_table_view);
#endif
    }
    table_view& table_view::operator=(table_view&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<TableView*>(&m_table_view) = std::move(*reinterpret_cast<TableView*>(&other.m_table_view));
        }
#else
        m_table_view = std::move(other.m_table_view);
#endif
        return *this;
    }
    table_view::~table_view() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<TableView*>(&m_table_view)->~TableView();
#endif
    }
    table_view::table_view(const TableView &ref) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_table_view) TableView(ref);
#else
        m_table_view = std::make_shared<TableView>(ref);
#endif
    }
    table_view::operator TableView() const {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return *reinterpret_cast<const TableView*>(&m_table_view);
#else
        return *m_table_view;
#endif
    }
}
