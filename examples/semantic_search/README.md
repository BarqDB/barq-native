# Realtime semantic search

A minimal example of vector (knn) search with the Barq native C++ SDK.

A `vector_indexed<>` property stores an embedding on each document. `knn()`
returns the documents nearest to a query vector, closest first. Because a knn
result set is **live**, an observer is notified the moment a better-matching
document is written — no polling.

```cpp
struct Document {
    barq::native::primary_key<int64_t> _id;
    std::string title;
    barq::native::vector_indexed<4, barq::native::vector_metric::cosine> embedding;
};
BARQ_SCHEMA(Document, _id, title, embedding)

auto hits = db.objects<Document>().knn(
    &Document::embedding, query_vector, barq::native::knn_options::approximate(3));
```

The demo seeds a small corpus (real apps get embeddings from a model; here they
are hand-placed on four toy axes — technology, nature, food, music), runs two
semantic queries, then registers an observer and writes a more-relevant document
to show the top result update in place.

## Build & run

The example is built as part of the SDK. From the SDK root:

```sh
cmake -G Ninja -S . -B build -DBARQ_CORE_SOURCE_DIR=/path/to/barq-core
cmake --build build --target semantic_search
./build/examples/semantic_search/semantic_search
```

Set `-DBARQ_NATIVE_NO_EXAMPLES=ON` to skip building the examples.
