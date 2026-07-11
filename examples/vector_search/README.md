# Vector (knn) search

Vector search with the Barq native C++ SDK, on **real data** — a sample of the
[Yandex Deep1B](https://research.yandex.com/blog/benchmarks-for-billion-scale-similarity-search)
benchmark (96-dimensional descriptors, L2 distance).

A `vector_indexed<>` property stores each embedding. `knn()` returns the nearest
vectors to a query, closest first. Because a knn result set is **live**, an
observer sees the ranking change the moment a nearer vector is written.

```cpp
struct DeepVec {
    barq::native::primary_key<int64_t> id;
    barq::native::vector_indexed<96, barq::native::vector_metric::l2> embedding;
};
BARQ_SCHEMA(DeepVec, id, embedding)

auto hits = db.objects<DeepVec>().knn(
    &DeepVec::embedding, query, barq::native::knn_options::approximate(10));
```

The demo loads the corpus from `deep1b_sample.json`, indexes it, and for each
query runs approximate knn and reports **recall against an exact oracle** — the
true top-10 L2 neighbours, precomputed independently and stored in the same
file. It ends with a short realtime section: insert a vector that matches a
query and watch the top result update in place.

## The data — `deep1b_sample.json`

Real Deep1B vectors, not toy numbers. Each entry is a genuine 96-d descriptor.

```json
{
  "dataset": "Yandex Deep1B (query split)",
  "metric": "l2",
  "dimension": 96,
  "items":   [ { "id": 1, "embedding": [ ... 96 floats ... ] }, ... ],
  "queries": [ { "id": 2001, "embedding": [ ... ], "neighbors": [ 754, 464, ... ] }, ... ]
}
```

`items` is the corpus; each query's `neighbors` are its exact top-10 L2 matches
within that corpus, computed offline as an independent oracle to check recall
against. (The sample is drawn from Deep1B's 10k held-out query vectors, which
ship as plain float32 — the billion-scale base set is not needed to run this.)

## Build & run

The example is built as part of the SDK. From the SDK root:

```sh
cmake -G Ninja -S . -B build -DBARQ_CORE_SOURCE_DIR=/path/to/barq-core
cmake --build build --target vector_search
./build/examples/vector_search/vector_search
```

Set `-DBARQ_NATIVE_NO_EXAMPLES=ON` to skip building the examples.
