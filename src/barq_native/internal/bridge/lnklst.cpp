#include <barq_native/internal/bridge/lnklst.hpp>
#include <barq_native/internal/bridge/obj.hpp>
#include <barq_native/internal/bridge/obj_key.hpp>

#include <barq/list.hpp>

namespace barq::native::internal::bridge {
    
    lnklst::lnklst() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_lnk_lst) LnkLst();
#else
        m_lnk_lst = std::make_shared<LnkLst>();
#endif
    }
    
    lnklst::lnklst(const lnklst& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_lnk_lst) LnkLst(*reinterpret_cast<const LnkLst*>(&other.m_lnk_lst));
#else
        m_lnk_lst = other.m_lnk_lst;
#endif
    }

    lnklst& lnklst::operator=(const lnklst& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<LnkLst*>(&m_lnk_lst) = *reinterpret_cast<const LnkLst*>(&other.m_lnk_lst);
        }
#else
        m_lnk_lst = other.m_lnk_lst;
#endif
        return *this;
    }

    lnklst::lnklst(lnklst&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_lnk_lst) LnkLst(std::move(*reinterpret_cast<LnkLst*>(&other.m_lnk_lst)));
#else
        m_lnk_lst = std::move(other.m_lnk_lst);
#endif
    }

    lnklst& lnklst::operator=(lnklst&& other) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        if (this != &other) {
            *reinterpret_cast<LnkLst*>(&m_lnk_lst) = std::move(*reinterpret_cast<LnkLst*>(&other.m_lnk_lst));
        }
#else
        m_lnk_lst = std::move(other.m_lnk_lst);
#endif
        return *this;
    }

    lnklst::~lnklst() {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        reinterpret_cast<LnkLst*>(&m_lnk_lst)->~LnkLst();
#endif
    }

    lnklst::lnklst(const LnkLst &v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        new (&m_lnk_lst) LnkLst(v);
#else
        m_lnk_lst = std::make_shared<LnkLst>(v);
#endif
    }

    obj lnklst::create_and_insert_linked_object(size_t idx) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<LnkLst*>(&m_lnk_lst)->create_and_insert_linked_object(idx);
#else
        return m_lnk_lst->create_and_insert_linked_object(idx);
#endif
    }

    void lnklst::add(const obj_key &v) {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return reinterpret_cast<LnkLst*>(&m_lnk_lst)->add(v);
#else
        return m_lnk_lst->add(v);
#endif
    }

    lnklst::operator LnkLst() const {
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        return *reinterpret_cast<const LnkLst*>(&m_lnk_lst);
#else
        return *m_lnk_lst;
#endif
    }
}