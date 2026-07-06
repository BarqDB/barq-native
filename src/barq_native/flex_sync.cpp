#include <barq_native/flex_sync.hpp>

#include <barq/object-store/shared_barq.hpp>
#include <barq/sync/subscriptions.hpp>

namespace barq::native {
    sync_subscription::sync_subscription(const ::barq::sync::Subscription &v)
    {
       identifier = v.id.to_string();
       name = v.name;
       created_at = v.created_at.get_time_point();
       updated_at = v.updated_at.get_time_point();
       query_string = v.query_string;
       object_class_name = v.object_class_name;
    }
    mutable_sync_subscription_set& mutable_sync_subscription_set::operator=(const mutable_sync_subscription_set& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<::barq::sync::MutableSubscriptionSet*>(&m_subscription_set) = *reinterpret_cast<const ::barq::sync::MutableSubscriptionSet*>(&other.m_subscription_set);
        }
#else
        m_subscription_set = other.m_subscription_set;
#endif
        return *this;
    }

    mutable_sync_subscription_set& mutable_sync_subscription_set::operator=(mutable_sync_subscription_set&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<::barq::sync::MutableSubscriptionSet*>(&m_subscription_set) = std::move(*reinterpret_cast<::barq::sync::MutableSubscriptionSet*>(&other.m_subscription_set));
        }
#else
        m_subscription_set = std::move(other.m_subscription_set);
#endif

        return *this;
    }

    mutable_sync_subscription_set::~mutable_sync_subscription_set() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<::barq::sync::MutableSubscriptionSet*>(&m_subscription_set)->~MutableSubscriptionSet();
#endif
    }
    mutable_sync_subscription_set::mutable_sync_subscription_set(internal::bridge::barq& barq,
                                                                 const ::barq::sync::MutableSubscriptionSet &subscription_set)
            : m_barq(barq)
    {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (m_subscription_set) ::barq::sync::MutableSubscriptionSet(subscription_set);
#else
        m_subscription_set = std::make_shared<::barq::sync::MutableSubscriptionSet>(subscription_set);
#endif
    }
    void mutable_sync_subscription_set::insert_or_assign(const std::string &name, const internal::bridge::query &query) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<::barq::sync::MutableSubscriptionSet *>(&m_subscription_set)->insert_or_assign(name, query);
#else
        m_subscription_set->insert_or_assign(name, query);
#endif
    }

    // Removes all subscriptions.
    void mutable_sync_subscription_set::clear() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<::barq::sync::MutableSubscriptionSet *>(&m_subscription_set)->clear();
#else
        m_subscription_set->clear();
#endif
    }

    ::barq::sync::MutableSubscriptionSet mutable_sync_subscription_set::get_subscription_set() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return *reinterpret_cast<::barq::sync::MutableSubscriptionSet *>(&m_subscription_set);
#else
        return *m_subscription_set;
#endif
    }

    // Removes a subscription for a given name. Will throw if subscription does
    // not exist.
    void mutable_sync_subscription_set::remove(const std::string& name) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        auto* set = reinterpret_cast<::barq::sync::MutableSubscriptionSet *>(&m_subscription_set);
#else
        auto* set = m_subscription_set.get();
#endif
        if (set->erase(name))
            return;
        throw std::logic_error("Subscription cannot be found");
    }

    // Finds a subscription for a given name. Will return `std::nullopt` is subscription does
    // not exist.
    std::optional<sync_subscription> mutable_sync_subscription_set::find(const std::string& name) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        auto* set = reinterpret_cast<::barq::sync::MutableSubscriptionSet *>(&m_subscription_set);
#else
        auto* set = m_subscription_set.get();
#endif
        if (auto it = set->find(name)) {
            return sync_subscription(*it);
        }
        return std::nullopt;
    }

    sync_subscription_set& sync_subscription_set::operator=(const sync_subscription_set& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<::barq::sync::SubscriptionSet*>(&m_subscription_set) = *reinterpret_cast<const ::barq::sync::SubscriptionSet*>(&other.m_subscription_set);
        }
#else
        m_subscription_set = other.m_subscription_set;
#endif
        return *this;
    }

    sync_subscription_set& sync_subscription_set::operator=(sync_subscription_set&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<::barq::sync::SubscriptionSet*>(&m_subscription_set) = std::move(*reinterpret_cast<::barq::sync::SubscriptionSet*>(&other.m_subscription_set));
        }
#else
        m_subscription_set = std::move(other.m_subscription_set);
#endif
        return *this;
    }

    sync_subscription_set::~sync_subscription_set() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<::barq::sync::SubscriptionSet*>(&m_subscription_set)->~SubscriptionSet();
#endif
    }

    size_t sync_subscription_set::size() const {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<const ::barq::sync::SubscriptionSet *>(&m_subscription_set)->size();
#else
        return m_subscription_set->size();
#endif
    }

    std::optional<sync_subscription> sync_subscription_set::find(const std::string& name) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        auto* set = reinterpret_cast<::barq::sync::SubscriptionSet *>(&m_subscription_set);
#else
        auto* set = m_subscription_set.get();
#endif
        if (auto it = set->find(name)) {
            return sync_subscription(*it);
        }
        return std::nullopt;
    }

    std::future<bool> sync_subscription_set::update(std::function<void(mutable_sync_subscription_set&)>&& fn) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        auto* set = reinterpret_cast<::barq::sync::SubscriptionSet *>(&m_subscription_set);
#else
        auto* set = m_subscription_set.get();
#endif
        auto mutable_set = mutable_sync_subscription_set(m_barq, set->make_mutable_copy());
        fn(mutable_set);

#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<::barq::sync::SubscriptionSet*>(&m_subscription_set)->~SubscriptionSet();
        new (&m_subscription_set) ::barq::sync::SubscriptionSet(mutable_set.get_subscription_set().commit());
#else
        m_subscription_set = std::make_shared<::barq::sync::SubscriptionSet>(mutable_set.get_subscription_set().commit());
#endif

        std::promise<bool> p;
        std::future<bool> f = p.get_future();

#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        auto* subscription_set = reinterpret_cast<::barq::sync::SubscriptionSet *>(&m_subscription_set);
#else
        auto* subscription_set = m_subscription_set.get();
#endif
        subscription_set->get_state_change_notification(barq::sync::SubscriptionSet::State::Complete)
                .get_async([p = std::move(p)](const barq::StatusWith<barq::sync::SubscriptionSet::State>& state) mutable noexcept {
                    p.set_value(state == ::barq::sync::SubscriptionSet::State::Complete);
                });

        return f;
    }

    sync_subscription_set::sync_subscription_set(internal::bridge::barq& barq)
            : m_barq(barq)
    {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_subscription_set) ::barq::sync::SubscriptionSet(static_cast<SharedBarq>(barq)->get_active_subscription_set());
#else
        m_subscription_set = std::make_shared<::barq::sync::SubscriptionSet>(static_cast<SharedBarq>(barq)->get_active_subscription_set());
#endif
    }
}
