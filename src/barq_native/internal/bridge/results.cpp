#include <barq_native/internal/bridge/results.hpp>

#include <barq_native/internal/bridge/obj.hpp>
#include <barq_native/internal/bridge/query.hpp>
#include <barq_native/internal/bridge/barq.hpp>
#include <barq_native/internal/bridge/table.hpp>
#include <barq/object-store/results.hpp>

namespace barq::native::internal::bridge {
    sort_descriptor::operator std::pair<std::string, bool>() const {
        return {key_path, ascending};
    }

    results::results() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_results) Results();
#else
        m_results = std::make_shared<Results>();
#endif
    }

    results::results(const results& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_results) Results(*reinterpret_cast<const Results*>(&other.m_results));
#else
        m_results = other.m_results;
#endif
    }

    results& results::operator=(const results& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<Results*>(&m_results) = *reinterpret_cast<const Results*>(&other.m_results);
        }
#else
        m_results = other.m_results;
#endif
        return *this;
    }

    results::results(results&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_results) Results(std::move(*reinterpret_cast<Results*>(&other.m_results)));
#else
        m_results = std::move(other.m_results);
#endif
    }

    results& results::operator=(results&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<Results*>(&m_results) = std::move(*reinterpret_cast<Results*>(&other.m_results));
        }
#else
        m_results = std::move(other.m_results);
#endif
        return *this;
    }

    results::~results() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<Results*>(&m_results)->~Results();
#endif
    }

    results::results(const barq &barq, const query &query) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_results) Results(barq, query);
#else
        m_results = std::make_shared<Results>(Results(barq, query));
#endif
    }

    results::results(const Results &v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_results) Results(v);
#else
        m_results = std::make_shared<Results>(v);
#endif
    }

    size_t results::size() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<Results*>(&m_results)->size();
#else
        return m_results->size();
#endif
    }

    barq results::get_barq() const {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<const Results*>(&m_results)->get_barq();
#else
        return m_results->get_barq();
#endif
    }

    table results::get_table() const {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<const Results*>(&m_results)->get_table();
#else
        return m_results->get_table();
#endif
    }

    results results::sort(const std::vector<sort_descriptor>& descriptors) {
        std::vector<std::pair<std::string, bool>> results_descriptors;
        results_descriptors.resize(descriptors.size());
        std::transform(descriptors.begin(), descriptors.end(), results_descriptors.begin(),
                       [](const sort_descriptor& sd) -> std::pair<std::string, bool>{
            return sd.operator std::pair<std::string, bool>();
        });
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<const Results*>(&m_results)->sort(results_descriptors);
#else
        return m_results->sort(results_descriptors);
#endif
    }

    results results::knn(const col_key& column, const std::vector<float>& query_vector,
                         size_t k, size_t ef, bool exact) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<Results*>(&m_results)->knn_search(column, query_vector, k, ef, exact);
#else
        return m_results->knn_search(column, query_vector, k, ef, exact);
#endif
    }

    results results::with_query(const query& q) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        auto& self = *reinterpret_cast<Results*>(&m_results);
        return Results(self.get_barq(), q, self.get_descriptor_ordering());
#else
        return Results(m_results->get_barq(), q, m_results->get_descriptor_ordering());
#endif
    }

    results results::freeze(const barq& target) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<Results*>(&m_results)->freeze(target);
#else
        return m_results->freeze(target);
#endif
    }

    template <>
    obj get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<Results*>(&res.m_results)->get(v);
#else
        return res.m_results->get(v);
#endif
    }

    template <>
    int64_t get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<Results*>(&res.m_results)->get<int64_t>(v);
#else
        return res.m_results->get<int64_t>(v);
#endif
    }

    template <>
    bool get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<Results*>(&res.m_results)->get<bool>(v);
#else
        return res.m_results->get<bool>(v);
#endif
    }

    template <>
    double get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<Results*>(&res.m_results)->get<double>(v);
#else
        return res.m_results->get<double>(v);
#endif
    }

    template <>
    float get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<Results*>(&res.m_results)->get<float>(v);
#else
        return res.m_results->get<float>(v);
#endif
    }

    template <>
    std::string get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<Results*>(&res.m_results)->get<StringData>(v);
#else
        return res.m_results->get<StringData>(v);
#endif
    }

    template <>
    ::barq::native::uuid get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return uuid(reinterpret_cast<Results*>(&res.m_results)->get<UUID>(v)).operator ::barq::native::uuid();
#else
        return uuid(res.m_results->get<UUID>(v)).operator ::barq::native::uuid();
#endif
    }

    template <>
    ::barq::native::object_id get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return object_id(reinterpret_cast<Results*>(&res.m_results)->get<ObjectId>(v)).operator ::barq::native::object_id();
#else
        return object_id(res.m_results->get<ObjectId>(v)).operator ::barq::native::object_id();
#endif
    }

    template <>
    ::barq::native::decimal128 get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return decimal128(reinterpret_cast<Results*>(&res.m_results)->get<Decimal128>(v)).operator ::barq::native::decimal128();
#else
        return decimal128(res.m_results->get<Decimal128>(v)).operator ::barq::native::decimal128();
#endif
    }

    template <>
    std::vector<uint8_t> get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return binary(reinterpret_cast<Results*>(&res.m_results)->get<BinaryData>(v)).operator std::vector<uint8_t>();
#else
        return binary(res.m_results->get<BinaryData>(v)).operator std::vector<uint8_t>();
#endif
    }

    template <>
    std::chrono::time_point<std::chrono::system_clock> get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return timestamp(reinterpret_cast<Results*>(&res.m_results)->get<Timestamp>(v)).operator std::chrono::time_point<std::chrono::system_clock>();
#else
        return timestamp(res.m_results->get<Timestamp>(v)).operator std::chrono::time_point<std::chrono::system_clock>();
#endif
    }


    template <>
    mixed get(results& res, size_t v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return mixed(reinterpret_cast<Results*>(&res.m_results)->get_any(v));
#else
        return mixed(res.m_results->get<Mixed>(v));
#endif
    }

    notification_token results::add_notification_callback(std::shared_ptr<collection_change_callback> &&cb) {
        struct wrapper : CollectionChangeCallback {
            std::shared_ptr<collection_change_callback> m_cb;
            explicit wrapper(std::shared_ptr<collection_change_callback>&& cb)
                    : m_cb(std::move(cb)) {}
            void before(const CollectionChangeSet& v) const {
                m_cb->before(v);
            }
            void after(const CollectionChangeSet& v) const {
                m_cb->after(v);
            }
        } ccb(std::move(cb));
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<Results*>(&m_results)->add_notification_callback(ccb);
#else
        return m_results->add_notification_callback(ccb);
#endif
    }

    results::results(const barq &barq, const table_view &tv) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_results) Results(barq, tv);
#else
        m_results = std::make_shared<Results>(Results(barq, tv));
#endif
    }
}
