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

#ifndef BARQ_NATIVE_BRIDGE_THREAD_SAFE_REFERENCE_HPP
#define BARQ_NATIVE_BRIDGE_THREAD_SAFE_REFERENCE_HPP

#include <barq_native/internal/bridge/utils.hpp>

namespace barq {
    class ThreadSafeReference;
}
namespace barq::native::internal::bridge {
    struct object;
    struct dictionary;
    struct barq;

    struct thread_safe_reference {
        thread_safe_reference();
        thread_safe_reference(const thread_safe_reference& other) = delete;
        thread_safe_reference& operator=(const thread_safe_reference& other) = delete;
        thread_safe_reference(thread_safe_reference&& other);
        thread_safe_reference& operator=(thread_safe_reference&& other);
        ~thread_safe_reference();
        thread_safe_reference(const object&);
        thread_safe_reference(const dictionary&);
        thread_safe_reference(ThreadSafeReference&&);
        operator ThreadSafeReference&&();
        operator bool() const; //NOLINT(google-explicit-constructor)
    private:
        friend struct barq;
        template <typename T>
        friend T resolve(const barq&, thread_safe_reference&& tsr);
#ifdef BARQ_NATIVE_HAVE_GENERATED_BRIDGE_TYPES
        storage::ThreadSafeReference m_thread_safe_reference[1];
#else
        std::shared_ptr<ThreadSafeReference> m_thread_safe_reference;
#endif
    };

}

#endif //BARQ_NATIVE_BRIDGE_THREAD_SAFE_REFERENCE_HPP
