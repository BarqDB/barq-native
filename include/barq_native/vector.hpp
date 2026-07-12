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
#include <initializer_list>
#include <utility>
#include <vector>

#include <barq_native/internal/bridge/vector_index.hpp>

namespace barq::native {

    // The distance metric and on-disk encoding a vector index is built with.
    // These name the same choices as the bridge/core enums; the SDK just
    // re-exports them so users write barq::native::vector_metric::cosine.
    using internal::bridge::vector_metric;
    using internal::bridge::vector_encoding;

    // Declares a property as a float embedding with a persisted vector (knn)
    // index, e.g. `vector_indexed<768, vector_metric::cosine> embedding;`.
    // The remaining optional template arguments tune the encoding, graph degree,
    // construction beam, default search beam, and full-build worker count.
    //
    // The dimension count is required and enforced. Cosine is the SDK default
    // because it is what most text/image embeddings expect. Storage is a plain
    // list of floats; the property is assigned and read as a whole vector (the
    // wrapper deliberately does not expose element-level mutation, since a
    // half-updated embedding is meaningless).
    template <size_t Dims,
              vector_metric Metric = vector_metric::cosine,
              vector_encoding Encoding = vector_encoding::float32,
              size_t M = 16,
              size_t EfConstruction = 200,
              size_t EfSearch = 0,
              size_t BuildThreads = 0>
    struct vector_indexed {
        static_assert(Dims > 0, "vector_indexed<> must declare a positive dimension count");
        static_assert(M > 0, "vector_indexed<> must use a positive graph degree");
        static_assert(EfConstruction > 0, "vector_indexed<> must use a positive construction beam");

        static constexpr size_t dimensions = Dims;
        static constexpr vector_metric metric = Metric;
        static constexpr vector_encoding encoding = Encoding;
        static constexpr size_t m = M;
        static constexpr size_t ef_construction = EfConstruction;
        static constexpr size_t ef_search = EfSearch;
        static constexpr size_t build_threads = BuildThreads;
        using value_type = float;

        vector_indexed() = default;
        vector_indexed(std::vector<float> v) // NOLINT(google-explicit-constructor)
            : value(std::move(v)) {}
        vector_indexed(std::initializer_list<float> v) // NOLINT(google-explicit-constructor)
            : value(v) {}
        // Assignment is via the converting constructors above plus the implicit
        // copy/move assignment; a dedicated operator=(std::vector<float>) would
        // make `embedding = {1, 2, 3}` ambiguous with the initializer_list ctor.
        operator const std::vector<float>&() const { return value; } // NOLINT(google-explicit-constructor)

        std::vector<float> value;
    };

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
