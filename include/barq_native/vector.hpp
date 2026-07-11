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

#ifndef BARQ_NATIVE_VECTOR_HPP
#define BARQ_NATIVE_VECTOR_HPP

#include <cstddef>

namespace barq::native {

    // Options for a k-nearest-neighbour (knn) vector search. A search is either
    // approximate or exact, never both, so the only way to build one is through
    // the two named factories below.
    struct knn_options {
        // Approximate (HNSW graph) search for the `k` nearest neighbours.
        // `ef` is the search beam width: higher trades speed for recall. 0 lets
        // the index choose a width that scales with how many vectors it holds.
        static knn_options approximate(size_t k, size_t ef = 0) {
            return knn_options(k, ef, false);
        }

        // Exact search: scans every candidate vector and returns the true `k`
        // nearest neighbours. Slower than approximate, but no recall loss.
        static knn_options exact(size_t k) {
            return knn_options(k, 0, true);
        }

        [[nodiscard]] size_t k() const noexcept { return m_k; }
        [[nodiscard]] size_t ef() const noexcept { return m_ef; }
        [[nodiscard]] bool is_exact() const noexcept { return m_exact; }

    private:
        knn_options(size_t k, size_t ef, bool exact) noexcept
            : m_k(k), m_ef(ef), m_exact(exact) {}
        size_t m_k;
        size_t m_ef;
        bool m_exact;
    };
}

#endif //BARQ_NATIVE_VECTOR_HPP
