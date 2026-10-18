#include "liboai.h"

#include <cstdlib>
#include <iostream>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

template <typename Call>
void expect_error(Call call) {
    try {
        call();
    } catch (const liboai::exception::OpenAIException&) {
        return;
    }
    require(false, "expected an API error");
}
}

int main(int argc, char** argv) {
    require(argc == 2, "expected local API root");
    liboai::OpenAI client(argv[1]);
    require(client.auth.SetKey("synthetic-test-token"), "failed to set synthetic token");
    std::string document = "xxfirstyy";
    std::vector<std::string_view> documents{std::string_view(document).substr(2, 5), "second\nquoted \"text\""};
    auto sync = client.Rerank->create("model", "query", documents);
    require(sync.raw_json["request"]["documents"] == nlohmann::json::array({"first", "second\nquoted \"text\""}),
            "document serialization mismatch");
    require(!sync.raw_json["request"].contains("top_n"), "unset top_n must be omitted");
    require(sync.raw_json["results"][0]["index"] == 1, "response indices altered");
    require(sync.raw_json["results"][0]["relevance_score"] == 0.75, "response score altered");

    liboai::FutureResponse future;
    {
        std::string temporary(100000, 'a');
        std::string model = "model", query = "query";
        std::vector<std::string_view> views{temporary};
        future = client.Rerank->create_async(model, query, views, 1);
        temporary.assign(100000, 'b');
        model.assign("changed");
        query.assign("changed");
    }
    auto async = future.get();
    require(async.raw_json["request"]["documents"][0] == std::string(100000, 'a'), "async documents not owned");
    require(async.raw_json["request"]["model"] == "model", "async model not owned");
    require(async.raw_json["request"]["query"] == "query", "async query not owned");
    require(async.raw_json["request"]["top_n"] == 1, "explicit top_n missing");
    expect_error([&] { (void)client.Rerank->create("model", "error", documents); });
    expect_error([&] { (void)client.Rerank->create_async("model", "error", documents).get(); });
    std::cout << "Rerank request, ownership, response, and error tests passed\n";
}
