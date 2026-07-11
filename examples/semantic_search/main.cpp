////////////////////////////////////////////////////////////////////////////
//
// Realtime semantic search with Barq's native (C++) SDK.
//
// A `vector_indexed<>` property stores an embedding; knn() finds the nearest
// documents to a query vector, ordered closest first. Because a knn result set
// is live, an observer sees the ranking change the moment a better match is
// written — no polling.
//
// Real apps get embeddings from a model (OpenAI, sentence-transformers, ...).
// To keep the demo self-contained the "embeddings" here are hand-placed on four
// toy axes: [ technology, nature, food, music ].
//
////////////////////////////////////////////////////////////////////////////

#include <barq_native/sdk.hpp>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace barq::native;

struct Document {
    primary_key<int64_t> _id;
    std::string title;
    vector_indexed<4, vector_metric::cosine> embedding;
};
BARQ_SCHEMA(Document, _id, title, embedding)

namespace {

// Cosine ignores magnitude, so these need not be normalized.
struct Seed {
    int64_t id;
    const char* title;
    std::vector<float> embedding;
};

const std::vector<Seed> corpus = {
    {1, "Training deep neural networks on GPUs", {1.0f, 0.0f, 0.0f, 0.0f}},
    {2, "A field guide to alpine wildflowers", {0.0f, 1.0f, 0.0f, 0.0f}},
    {3, "The perfect Neapolitan pizza dough", {0.0f, 0.0f, 1.0f, 0.0f}},
    {4, "Improvising over jazz chord changes", {0.0f, 0.0f, 0.0f, 1.0f}},
    {5, "Attention, transformers, and LLMs", {0.96f, 0.0f, 0.0f, 0.1f}},
    {6, "Photographing mountain landscapes", {0.15f, 0.92f, 0.0f, 0.0f}},
    {7, "Sourdough starter, step by step", {0.0f, 0.05f, 0.95f, 0.0f}},
};

void print_ranked(const std::string& query_label, results<Document>& hits) {
    std::cout << "  query: \"" << query_label << "\"\n";
    for (size_t i = 0; i < hits.size(); ++i) {
        auto doc = hits[i];
        std::cout << "    " << (i + 1) << ". " << static_cast<std::string>(doc.title) << "\n";
    }
    std::cout << "\n";
}

} // namespace

int main() {
    auto path = std::filesystem::temp_directory_path() / "barq_semantic_search_demo.barq";
    std::error_code ec;
    std::filesystem::remove(path, ec); // fresh run each time

    db_config config;
    config.set_path(path.string());
    auto barq = db(config); // opening reconciles (builds) the local vector index

    std::cout << "Seeding " << corpus.size() << " documents...\n\n";
    barq.write([&] {
        for (const auto& s : corpus) {
            Document d;
            d._id = s.id;
            d.title = s.title;
            d.embedding = s.embedding;
            barq.add(std::move(d));
        }
    });

    // --- Semantic search: closest documents to a query embedding ---
    std::cout << "== Semantic search (top 3, closest first) ==\n\n";
    {
        // "machine learning" leans on the technology axis.
        auto hits = barq.objects<Document>().knn(
            &Document::embedding, std::vector<float>{1.0f, 0.0f, 0.0f, 0.0f},
            knn_options::approximate(3));
        print_ranked("machine learning", hits);
    }
    {
        // "baking bread" leans on the food axis.
        auto hits = barq.objects<Document>().knn(
            &Document::embedding, std::vector<float>{0.0f, 0.0f, 1.0f, 0.0f},
            knn_options::approximate(3));
        print_ranked("baking bread", hits);
    }

    // --- Realtime: an observer sees the ranking change on the next write ---
    std::cout << "== Realtime update ==\n\n";
    // A query that no seed matches exactly, so a better document can arrive.
    auto query = std::vector<float>{1.0f, 0.5f, 0.0f, 0.0f};
    auto live = barq.objects<Document>().knn(&Document::embedding, query,
                                             knn_options::approximate(1));

    int notifications = 0;
    auto token = live.observe([&](auto&&) { notifications++; });
    barq.refresh(); // deliver the initial notification

    std::cout << "  best match before: "
              << static_cast<std::string>(live[0].title) << "\n";

    std::cout << "  ...a new, more relevant document arrives...\n";
    barq.write([&] {
        Document d;
        d._id = 100;
        d.title = "Machine learning for climate modeling"; // matches the query exactly
        d.embedding = std::vector<float>{1.0f, 0.5f, 0.0f, 0.0f};
        barq.add(std::move(d));
    });
    barq.refresh(); // deliver the change notification

    std::cout << "  best match after:  "
              << static_cast<std::string>(live[0].title) << "\n";
    std::cout << "  observer fired " << notifications << " times (initial + change)\n\n";

    std::cout << "Done.\n";
    return 0;
}
