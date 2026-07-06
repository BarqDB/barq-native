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

#ifndef BARQ_NATIVE_BARQ_CORE_SCHEDULER_HPP
#define BARQ_NATIVE_BARQ_CORE_SCHEDULER_HPP

#include <barq_native/scheduler.hpp>

namespace barq {
    namespace util {
        class Scheduler;
    }
}
namespace barq::native::internal {

    /**
     * A type erased scheduler used for wrapping default scheduler implementations from Barq Core.
     */
    struct barq_core_scheduler final : public scheduler {
        /**
         * Invoke the given function on the scheduler's thread.
         * This function can be called from any thread.
         */
        void invoke(std::function<void()> &&fn) final;

        /**
         * Check if the caller is currently running on the scheduler's thread.
         * This function can be called from any thread.
         */
        [[nodiscard]] bool is_on_thread() const noexcept final;

        /**
         * Checks if this scheduler instance wraps the same underlying instance.
         * This is up to the platforms to define, but if this method returns true,
         * caching may occur.
         */
        bool is_same_as(const scheduler *other) const noexcept final;

        /**
         * Check if this scheduler actually can support invoke(). Invoking may be
         * either not implemented, not applicable to a scheduler type, or simply not
         * be possible currently (e.g. if the associated event loop is not actually
         * running).
         *
         * This function is not thread-safe.
         */
        [[nodiscard]] bool can_invoke() const noexcept final;
        ~barq_core_scheduler() final = default;
        barq_core_scheduler() = delete;
        explicit barq_core_scheduler(std::shared_ptr<util::Scheduler> s) : s(std::move(s)) {}
        operator std::shared_ptr<util::Scheduler>();
    private:
        std::shared_ptr<util::Scheduler> s;
    };

    std::shared_ptr<util::Scheduler> create_scheduler_shim(const std::shared_ptr<scheduler>& s);
} // namespace barq::native::internal

#endif//BARQ_NATIVE_BARQ_CORE_SCHEDULER_HPP
