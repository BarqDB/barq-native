#include <barq_native/db.hpp>
#include <barq_native/flex_sync.hpp>

namespace barq::native {
    ::barq::native::sync_subscription_set db::subscriptions() {
        return ::barq::native::sync_subscription_set(m_barq);
    }

    bool db::is_frozen() const {
        return m_barq.is_frozen();
    }
    db db::freeze() {
        return db(m_barq.freeze());
    }

    db db::thaw() {
        return db(m_barq.thaw());
    }

    void db::invalidate() {
        m_barq.invalidate();
    }

    void db::close() {
        m_barq.close();
    }

    bool db::is_closed() {
        return m_barq.is_closed();
    }

    bool operator==(const db& lhs, const db& rhs) {
        return lhs.m_barq == rhs.m_barq;
    }
    bool operator!=(const db& lhs, const db& rhs) {
        return lhs.m_barq != rhs.m_barq;
    }

} // namespace barq
