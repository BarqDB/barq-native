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

#ifndef BARQ_NATIVE_BRIDGE_VECTOR_INDEX_HPP
#define BARQ_NATIVE_BRIDGE_VECTOR_INDEX_HPP

#include <cstddef>
#include <cstdint>

namespace barq::native::internal::bridge {

    // Distance metric for a vector index. The values mirror barq::VectorMetric
    // in barq-core; the table bridge maps between the two.
    enum class vector_metric : uint8_t {
        inner_product = 0, // dot product (higher = closer); expects pre-normalized vectors
        l2 = 1,            // squared euclidean distance (lower = closer)
        cosine = 2,        // inner product on vectors normalized at insert/query time
    };

    // How the index stores its copy of the vectors. Values mirror
    // barq::VectorEncoding.
    enum class vector_encoding : uint8_t {
        float32 = 0, // full precision, 4 bytes per dimension
        sq8 = 1,     // scalar-quantized to 1 byte per dimension, exact re-rank on read
    };

    // Build/search parameters for a vector index, mirroring
    // barq::VectorIndexConfig. Defaults match core so an all-default config
    // reproduces core's own defaults.
    struct vector_index_config {
        vector_metric metric = vector_metric::inner_product;
        vector_encoding encoding = vector_encoding::float32;
        size_t dimensions = 0;        // 0 = infer from the first vector; >0 = enforced
        size_t m = 16;                // HNSW out-degree
        size_t ef_construction = 200; // build-time beam width
        size_t ef_search = 0;         // query-time beam width floor; 0 = auto by size
        size_t build_threads = 0;     // 0 = one per core (not persisted)
    };
}

#endif //BARQ_NATIVE_BRIDGE_VECTOR_INDEX_HPP
