#include <barq_native/internal/bridge/barq.hpp>

#include <barq_native/logger.hpp>
#include <barq_native/internal/bridge/async_open_task.hpp>
#include <barq_native/internal/bridge/dictionary.hpp>
#include <barq_native/internal/bridge/obj.hpp>
#include <barq_native/internal/bridge/object.hpp>
#include <barq_native/internal/bridge/object_schema.hpp>
#include <barq_native/internal/bridge/schema.hpp>
#include <barq_native/internal/bridge/sync_error.hpp>
#include <barq_native/internal/bridge/sync_session.hpp>
#include <barq_native/internal/bridge/table.hpp>
#include <barq_native/internal/bridge/thread_safe_reference.hpp>
#include <barq_native/internal/scheduler/barq_core_scheduler.hpp>
#include <barq_native/schedulers/default_scheduler.hpp>

#include <barq/object-store/dictionary.hpp>
#include <barq/object-store/object_store.hpp>

#include <barq/object-store/schema.hpp>
#include <barq/object-store/shared_barq.hpp>
#include <barq/object-store/sync/sync_session.hpp>
#include <barq/object-store/sync/sync_user.hpp>
#include <barq/object-store/thread_safe_reference.hpp>
#include <barq/object-store/util/scheduler.hpp>
#include <barq/sync/config.hpp>

#if defined(BARQ_AOSP_VENDOR)
#include <unistd.h>
#else
#include <filesystem>
#endif

namespace barq::native::internal::bridge {
    static_assert((uint8_t)barq::config::schema_mode::automatic == (uint8_t)::barq::SchemaMode::Automatic);
    static_assert((uint8_t)barq::config::schema_mode::immutable == (uint8_t)::barq::SchemaMode::Immutable);
    static_assert((uint8_t)barq::config::schema_mode::read_only == (uint8_t)::barq::SchemaMode::ReadOnly);
    static_assert((uint8_t)barq::config::schema_mode::soft_reset_file == (uint8_t)::barq::SchemaMode::SoftResetFile);
    static_assert((uint8_t)barq::config::schema_mode::hard_reset_file == (uint8_t)::barq::SchemaMode::HardResetFile);
    static_assert((uint8_t)barq::config::schema_mode::additive_discovered == (uint8_t)::barq::SchemaMode::AdditiveDiscovered);
    static_assert((uint8_t)barq::config::schema_mode::additive_explicit == (uint8_t)::barq::SchemaMode::AdditiveExplicit);
    static_assert((uint8_t)barq::config::schema_mode::manual == (uint8_t)::barq::SchemaMode::Manual);

    static_assert((uint8_t)client_reset_mode::discard_unsynced == (uint8_t)::barq::ClientResyncMode::DiscardLocal);
    static_assert((uint8_t)client_reset_mode::manual == (uint8_t)::barq::ClientResyncMode::Manual);
    static_assert((uint8_t)client_reset_mode::recover == (uint8_t)::barq::ClientResyncMode::Recover);
    static_assert((uint8_t)client_reset_mode::recover_or_discard == (uint8_t)::barq::ClientResyncMode::RecoverOrDiscard);

    class null_logger : public ::barq::native::logger {
    public:
        null_logger() {
           set_level_threshold(logger::level::off);
        };
        void do_log(logger::level, const std::string&) override {}
    };

    barq::barq(std::shared_ptr<Barq> v)
    : m_barq(std::move(v)){}

    barq::operator std::shared_ptr<Barq>() const {
        return m_barq;
    }
    void barq::begin_transaction() const {
        m_barq->begin_transaction();
    }

    void barq::commit_transaction() const {
        m_barq->commit_transaction();
    }

    barq::barq(thread_safe_reference&& tsr, const std::optional<std::shared_ptr<struct scheduler>>& s) {
        if (s) {
            m_barq = Barq::get_shared_barq(std::move(tsr), create_scheduler_shim(*s));
        } else {
            m_barq = Barq::get_shared_barq(std::move(tsr), create_scheduler_shim(default_scheduler::make_default()));
        }
    }

    barq::config::config() {
        BarqConfig config;
        config.cache = true;
#if defined(BARQ_AOSP_VENDOR)
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        std::string path = cwd;
        path.append("/default.barq");
        config.path = path;
#else
        config.path = std::filesystem::current_path().append("default.barq").generic_string();
#endif
        config.scheduler = create_scheduler_shim(default_scheduler::make_default());
        config.schema_version = 0;
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_config) BarqConfig(config);
#else
        m_config = std::make_shared<BarqConfig>(config);
#endif
    }

    barq::config::config(const config& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_config) BarqConfig(*reinterpret_cast<const BarqConfig*>(&other.m_config));
#else
        m_config = other.m_config;
#endif
    }

    barq::config& barq::config::operator=(const config& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<BarqConfig*>(&m_config) = *reinterpret_cast<const BarqConfig*>(&other.m_config);
        }
#else
        m_config = other.m_config;
#endif
        return *this;
    }

    barq::config::config(config&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_config) BarqConfig(std::move(*reinterpret_cast<BarqConfig*>(&other.m_config)));
#else
        m_config = std::move(other.m_config);
#endif
    }

    barq::config& barq::config::operator=(config&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<BarqConfig*>(&m_config) = std::move(*reinterpret_cast<BarqConfig*>(&other.m_config));
        }
#else
        m_config = std::move(other.m_config);
#endif
        return *this;
    }

    barq::config::~config() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<BarqConfig*>(&m_config)->~BarqConfig();
#endif
    }

    barq::config::config(const BarqConfig &v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_config) BarqConfig(v);
#else
        m_config = std::make_shared<BarqConfig>(v);
#endif
    }
    barq::config::config(const std::string& path,
                          const std::shared_ptr<struct scheduler>& scheduler) {
        BarqConfig config;
        config.cache = true;
        config.path = path;
        config.scheduler = create_scheduler_shim(scheduler);
        config.schema_version = 0;
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_config) BarqConfig(config);
#else
        m_config = std::make_shared<BarqConfig>(config);
#endif
    }

    barq::sync_config::sync_config(const std::shared_ptr<SyncConfig> &v) {
        m_config = v;
    }
    barq::sync_config::operator std::shared_ptr<SyncConfig>() const {
        return m_config;
    }

    inline BarqConfig* barq::config::get_config() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<BarqConfig*>(&m_config);
#else
        return m_config.get();
#endif
    }

    inline const BarqConfig* barq::config::get_config() const {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<const BarqConfig*>(&m_config);
#else
        return m_config.get();
#endif
    }

    std::string barq::config::path() const {
        return get_config()->path;
    }
    barq::config barq::get_config() const {
        return m_barq->config();
    }
    void barq::config::set_schema(const std::vector<object_schema> &v) {
        std::vector<ObjectSchema> v2;
        for (auto& os : v) {
            v2.push_back(os);
        }
        get_config()->schema = v2;
    }
    void barq::config::set_schema_mode(schema_mode mode) {
        get_config()->schema_mode = static_cast<::barq::SchemaMode>(mode);
    }

    std::optional<schema> barq::config::get_schema() {
        if (auto s = get_config()->schema) {
            return *s;
        }
        return std::nullopt;
    }

    schema barq::schema() const {
        return m_barq->schema();
    }

    table barq::table_for_object_type(const std::string &object_type) {
        return read_group().get_table(object_type);
    }

    barq::barq() { }
    barq::config::operator BarqConfig() const {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return *reinterpret_cast<const BarqConfig*>(&m_config);
#else
        return *m_config;
#endif
    }
    barq::barq(const config &v) {
        static bool initialized;
        if (!initialized) {
            auto logger = std::make_shared<null_logger>();
            logger->set_level_threshold(logger::level::off);
            set_default_logger(logger);
            initialized = true;
        }
        m_barq = Barq::get_shared_barq(static_cast<BarqConfig>(v));
    }
    bool operator==(barq const &lhs, barq const &rhs) {
        return static_cast<SharedBarq>(lhs) == static_cast<SharedBarq>(rhs);
    }
    bool operator!=(barq const& lhs, barq const& rhs) {
        return static_cast<SharedBarq>(lhs) != static_cast<SharedBarq>(rhs);
    }
    template <>
    dictionary resolve(const barq& r, thread_safe_reference &&tsr) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<ThreadSafeReference*>(tsr.m_thread_safe_reference)->resolve<Dictionary>(r);
#else
        return reinterpret_cast<ThreadSafeReference*>(tsr.m_thread_safe_reference.get())->resolve<Dictionary>(r);
#endif
    }
    template <>
    object resolve(const barq& r, thread_safe_reference &&tsr) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<ThreadSafeReference*>(tsr.m_thread_safe_reference)->resolve<Object>(r);
#else
        return reinterpret_cast<ThreadSafeReference*>(tsr.m_thread_safe_reference.get())->resolve<Object>(r);
#endif
    }
    void barq::config::set_scheduler(const std::shared_ptr<struct scheduler> &s) {
        if (auto core_scheduler = dynamic_cast<barq_core_scheduler *>(s.get())) {
            get_config()->scheduler = core_scheduler->operator std::shared_ptr<util::Scheduler>();
            return;
        }
        get_config()->scheduler = create_scheduler_shim(s);
    }
    void barq::config::set_sync_config(const std::optional<struct sync_config> &s) {
        if (s)
            get_config()->sync_config = static_cast<std::shared_ptr<SyncConfig>>(*s);
        else
            get_config()->sync_config = nullptr;
    }

    void barq::config::set_custom_http_headers(const std::map<std::string, std::string>& headers) {
        if (get_config()->sync_config) {
            get_config()->sync_config->custom_http_headers = headers;
        } else {
            throw std::logic_error("HTTP headers can only be set on a config for a synced Barq.");
        }
    }

    void barq::config::set_proxy_config(const sync_config::proxy_config& config) {
        if (get_config()->sync_config) {
            SyncConfig::ProxyConfig core_config;
            core_config.address = config.address;
            core_config.port = config.port;
            core_config.type = SyncConfig::ProxyConfig::Type::HTTP;
            get_config()->sync_config->proxy_config = std::move(core_config);
        } else {
            throw std::logic_error("Proxy configuration can only be set on a config for a synced Barq.");
        }
    }

    void barq::config::set_schema_version(uint64_t version) {
        get_config()->schema_version = version;
    }

    void barq::config::set_encryption_key(const std::array<char, 64>& encryption_key) {
        auto key = std::vector<char>();
        key.resize(64);
        key.assign(encryption_key.begin(), encryption_key.end());
        get_config()->encryption_key = std::move(key);
    }

    void barq::config::should_compact_on_launch(std::function<bool(uint64_t total_bytes, uint64_t unused_bytes)>&& fn) {
        get_config()->should_compact_on_launch_function = std::move(fn);
    }

    void barq::config::enable_forced_sync_history() {
        get_config()->force_sync_history = true;
    }

    enum client_reset_mode barq::config::get_client_reset_mode() const {
        return static_cast<enum client_reset_mode>(get_config()->sync_config->client_resync_mode);
    }

    void barq::config::set_client_reset_mode(enum client_reset_mode mode) {
        get_config()->sync_config->client_resync_mode = static_cast<ClientResyncMode>(mode);
    }

    void barq::config::before_client_reset(std::function<void(barq old_barq)> callback) {
        get_config()->sync_config->notify_before_client_reset = [cb = std::move(callback)](::barq::SharedBarq old) {
            cb(barq(old));
        };
    }
    void barq::config::after_client_reset(std::function<void(barq local_barq, barq remote_barq)> callback) {
        get_config()->sync_config->notify_after_client_reset = [cb = std::move(callback)](::barq::SharedBarq local,
                                                                                          ::barq::ThreadSafeReference remote, bool) {
            cb(barq(local), barq(::barq::Barq::get_shared_barq(std::move(remote), create_scheduler_shim(default_scheduler::make_default()))));
        };
    }

    barq::sync_config barq::config::sync_config() const {
        return get_config()->sync_config;
    }

    std::shared_ptr<scheduler> barq::config::scheduler() const {
        return std::make_shared<barq_core_scheduler>(barq_core_scheduler(get_config()->scheduler));
    }

    std::shared_ptr<scheduler> barq::scheduler() const {
        return std::make_shared<barq_core_scheduler>(barq_core_scheduler(m_barq->scheduler()));
    }

    async_open_task barq::get_synchronized_barq(const config &c) {
        return Barq::get_synchronized_barq(c);
    }

    void barq::sync_config::set_error_handler(std::function<void(const sync_session &, const sync_error &)> &&fn) {
        m_config->error_handler = [fn = std::move(fn)](const std::shared_ptr<SyncSession>& session,
                                                       SyncError&& error) {
            fn(session, std::move(error));
        };
    }

    void barq::sync_config::set_stop_policy(barq::sync_session_stop_policy &&v) {
        m_config->stop_policy = static_cast<SyncSessionStopPolicy>(v);
    }

    barq::sync_config::sync_config(const std::shared_ptr<SyncUser> &user) {
        m_config = std::make_shared<SyncConfig>(user, SyncConfig::FLXSyncEnabled{});
    }

    void barq::config::set_path(const std::string &path) {
        get_config()->path = path;
    }

    bool barq::refresh() {
        return m_barq->refresh();
    }

    bool barq::is_frozen() const {
        return m_barq->is_frozen();
    }

    barq barq::freeze() {
        m_barq->verify_thread();
        if (is_frozen())
            return *this;
        barq barq;
        barq.m_barq = m_barq->freeze();
        barq.m_barq->read_group();
        return barq;
    }

    barq barq::thaw() {
        m_barq->verify_thread();
        if (!is_frozen())
            return *this;
        auto config = m_barq->config();
        config.cache = true;
        config.scheduler = create_scheduler_shim(default_scheduler::make_default());
        return barq(std::move(config));
    }

    void barq::close() {
        m_barq->close();
    }

    bool barq::is_closed() {
        return m_barq->is_closed();
    }

    void barq::invalidate() {
        m_barq->verify_thread();
        m_barq->invalidate();
    }

    obj barq::import_copy_of(const obj& o) const {
        return m_barq->import_copy_of(o.operator Obj());
    }

    [[nodiscard]] std::optional<sync_session> barq::get_sync_session() const {
        auto& config = m_barq->config().sync_config;
        if (!config) {
            return std::nullopt;
        }

        return sync_session(m_barq->sync_session());
    }

    table barq::get_table(const uint32_t &key) {
        return m_barq->read_group().get_table(TableKey(key));
    }
}
