////////////////////////////////////////////////////////////////////////////
//
// Vector (knn) search with the Barq native (C++) SDK, on real data.
//
// The corpus is a sample of genuine Yandex Deep1B descriptors — 96-dimensional
// float vectors — loaded from deep1b_sample.json. A `vector_indexed<>` property
// stores each embedding; knn() returns the nearest vectors to a query, closest
// first. Deep1B uses L2 distance, so the index does too.
//
// The JSON also carries, for each query, the exact top-10 L2 neighbours
// (precomputed as an independent oracle). We run Barq's approximate knn and
// report recall against that oracle.
//
////////////////////////////////////////////////////////////////////////////

#include <barq_native/sdk.hpp>

#include <external/json/json.hpp> // nlohmann::json, vendored by barq-core

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace barq::native;
using json = nlohmann::json;

// Deep1B vectors are 96-dimensional; L2 is the dataset's distance metric.
static constexpr size_t DIM = 96;

struct DeepVec {
    primary_key<int64_t> id;
    vector_indexed<DIM, vector_metric::l2> embedding;
};
BARQ_SCHEMA(DeepVec, id, embedding)

namespace {

double l2(const std::vector<float>& a, const std::vector<float>& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        double d = static_cast<double>(a[i]) - static_cast<double>(b[i]);
        sum += d * d;
    }
    return std::sqrt(sum);
}

} // namespace

int main() {
    // --- Load the sampled Deep1B vectors (path baked in at build time) -------
    std::ifstream in(DATA_PATH);
    if (!in) {
        std::cerr << "cannot open data file: " << DATA_PATH << "\n";
        return 1;
    }
    json data;
    in >> data;

    std::cout << "Loaded " << data["items"].size() << " real "
              << data["dataset"].get<std::string>() << " vectors "
              << "(dim " << data["dimension"].get<int>() << ", metric "
              << data["metric"].get<std::string>() << ")\n\n";

    auto path = std::filesystem::temp_directory_path() / "barq_vector_search_demo.barq";
    std::error_code ec;
    std::filesystem::remove(path, ec); // fresh run each time

    db_config config;
    config.set_path(path.string());
    auto barq = db(config); // opening builds the local vector index

    // --- Index the corpus ---------------------------------------------------
    barq.write([&] {
        for (const auto& item : data["items"]) {
            DeepVec v;
            v.id = item["id"].get<int64_t>();
            v.embedding = item["embedding"].get<std::vector<float>>();
            barq.add(std::move(v));
        }
    });
    std::cout << "Indexed " << barq.objects<DeepVec>().size() << " vectors.\n\n";

    // --- Run each real query; compare approximate knn to the exact oracle ----
    const int k = 10;
    double recall_sum = 0.0;
    int query_count = 0;

    for (const auto& query : data["queries"]) {
        auto qvec = query["embedding"].get<std::vector<float>>();

        std::unordered_set<int64_t> oracle;
        for (const auto& n : query["neighbors"])
            oracle.insert(n.get<int64_t>());

        auto hits = barq.objects<DeepVec>().knn(&DeepVec::embedding, qvec,
                                                knn_options::approximate(k));

        int found = 0;
        for (size_t i = 0; i < hits.size(); ++i) {
            int64_t id = hits[i].id;
            if (oracle.count(id))
                ++found;
        }
        recall_sum += static_cast<double>(found) / static_cast<double>(k);
        ++query_count;

        // Show the first query concretely: nearest ids with their L2 distance.
        if (query_count == 1) {
            std::cout << "Query " << query["id"].get<int64_t>()
                      << " — nearest neighbours (id : L2 distance):\n";
            for (size_t i = 0; i < hits.size() && i < 5; ++i) {
                std::vector<float> emb = hits[i].embedding;
                std::cout << "  " << (i + 1) << ". id " << static_cast<int64_t>(hits[i].id)
                          << "   " << l2(qvec, emb) << "\n";
            }
            std::cout << "\n";
        }
    }

    std::cout << "Approximate knn recall@" << k << " vs exact oracle: "
              << (100.0 * recall_sum / query_count) << "%"
              << "  (" << query_count << " queries)\n\n";

    // --- Realtime: a knn result set is live ---------------------------------
    // Observe the nearest vector to a query, then insert one that matches the
    // query exactly (distance 0). The top result updates in place — no polling.
    std::cout << "== Realtime ==\n\n";
    auto qvec = data["queries"][0]["embedding"].get<std::vector<float>>();
    auto live = barq.objects<DeepVec>().knn(&DeepVec::embedding, qvec,
                                            knn_options::approximate(1));
    int notifications = 0;
    auto token = live.observe([&](auto&&) { ++notifications; });
    barq.refresh(); // deliver the initial notification

    std::cout << "  best match before: id " << static_cast<int64_t>(live[0].id) << "\n";
    barq.write([&] {
        DeepVec v;
        v.id = 9999999; // an id outside the corpus
        v.embedding = qvec; // identical to the query, so its distance is 0
        barq.add(std::move(v));
    });
    barq.refresh(); // deliver the change

    std::cout << "  best match after:  id " << static_cast<int64_t>(live[0].id)
              << " (the vector just inserted)\n";
    std::cout << "  observer fired " << notifications << " times (initial + change)\n";
    return 0;
}
