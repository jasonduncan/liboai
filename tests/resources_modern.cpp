#include "components/conversations.h"
#include "components/files.h"
#include "components/responses.h"
#include "components/batch.h"
#include "components/uploads.h"
#include "components/vector_stores.h"
#include "components/containers.h"
#include "components/webhooks.h"
#include <stdexcept>
#include <fstream>
#include <chrono>

namespace {
void require_resource(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
void route(const liboai::ApiResult& result, const char* method, const std::string& path) {
    const auto& echo = result.response.raw_json;
    require_resource(echo.at("method") == method, "Resource HTTP verb mismatch");
    const auto actual = echo.at("path").get<std::string>();
    require_resource(actual.size() >= path.size() && actual.compare(actual.size()-path.size(), path.size(), path) == 0, "Resource path mismatch");
}
template<class F> void invalid_resource(F action) {
    bool threw = false;
    try { action(); } catch (const std::invalid_argument&) { threw = true; }
    require_resource(threw, "Invalid upload input accepted");
}
}

void test_resources(const std::string& root) {
    using namespace liboai;
    using nlohmann::json;
    const json body{{"unknown_future_field", {{"preserved", true}}}};
    const Query query{{"after", "cursor/+?"}, {"include[]", "one"}, {"include[]", "two"}};
    RequestOptions options;
    options.project = "resource-project";
    Conversations conv(root);
    { auto result = conv.create(body, options);
      route(result, "POST", "/conversations");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = conv.retrieve_async("conversation_id/ space", options).get();
      route(result, "GET", "/conversations/conversation_id%2F%20space");
    }
    { auto result = conv.update("conversation_id/ space", body, options);
      route(result, "POST", "/conversations/conversation_id%2F%20space");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = conv.remove_async("conversation_id/ space", options).get();
      route(result, "DELETE", "/conversations/conversation_id%2F%20space");
    }
    { auto result = conv.create_items("conversation_id/ space", body, query, options);
      route(result, "POST", "/conversations/conversation_id%2F%20space/items");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
      require_resource(result.response.raw_json.at("query").at("after").at(0) == "cursor/+?", "Resource cursor was altered");
      require_resource(result.response.raw_json.at("query").at("include[]").size() == 2, "Repeated resource query fields lost");
    }
    { auto result = conv.retrieve_item_async("conversation_id/ space", "item_id/ space", query, options).get();
      route(result, "GET", "/conversations/conversation_id%2F%20space/items/item_id%2F%20space");
      require_resource(result.response.raw_json.at("query").at("after").at(0) == "cursor/+?", "Resource cursor was altered");
      require_resource(result.response.raw_json.at("query").at("include[]").size() == 2, "Repeated resource query fields lost");
    }
    { auto result = conv.remove_item("conversation_id/ space", "item_id/ space", options);
      route(result, "DELETE", "/conversations/conversation_id%2F%20space/items/item_id%2F%20space");
    }
    { auto result = conv.list_items_async("conversation_id/ space", query, options).get();
      route(result, "GET", "/conversations/conversation_id%2F%20space/items");
      require_resource(result.response.raw_json.at("query").at("after").at(0) == "cursor/+?", "Resource cursor was altered");
      require_resource(result.response.raw_json.at("query").at("include[]").size() == 2, "Repeated resource query fields lost");
    }
    Batch batch(root);
    { auto result = batch.create(body, options);
      route(result, "POST", "/batches");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = batch.retrieve_async("batch_id/ space", options).get();
      route(result, "GET", "/batches/batch_id%2F%20space");
    }
    { auto result = batch.list(query, options);
      route(result, "GET", "/batches");
      require_resource(result.response.raw_json.at("query").at("after").at(0) == "cursor/+?", "Resource cursor was altered");
      require_resource(result.response.raw_json.at("query").at("include[]").size() == 2, "Repeated resource query fields lost");
    }
    { auto result = batch.cancel_async("batch_id/ space", options).get();
      route(result, "POST", "/batches/batch_id%2F%20space/cancel");
    }
    VectorStores stores(root);
    { auto result = stores.create(body, options);
      route(result, "POST", "/vector_stores");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = stores.retrieve_async("vector_store_id/ space", options).get();
      route(result, "GET", "/vector_stores/vector_store_id%2F%20space");
    }
    { auto result = stores.update("vector_store_id/ space", body, options);
      route(result, "POST", "/vector_stores/vector_store_id%2F%20space");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = stores.remove_async("vector_store_id/ space", options).get();
      route(result, "DELETE", "/vector_stores/vector_store_id%2F%20space");
    }
    { auto result = stores.list(query, options);
      route(result, "GET", "/vector_stores");
      require_resource(result.response.raw_json.at("query").at("after").at(0) == "cursor/+?", "Resource cursor was altered");
      require_resource(result.response.raw_json.at("query").at("include[]").size() == 2, "Repeated resource query fields lost");
    }
    { auto result = stores.search_async("vector_store_id/ space", body, options).get();
      route(result, "POST", "/vector_stores/vector_store_id%2F%20space/search");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = stores.create_file("vector_store_id/ space", body, options);
      route(result, "POST", "/vector_stores/vector_store_id%2F%20space/files");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = stores.retrieve_file_async("vector_store_id/ space", "file_id/ space", options).get();
      route(result, "GET", "/vector_stores/vector_store_id%2F%20space/files/file_id%2F%20space");
    }
    { auto result = stores.update_file("vector_store_id/ space", "file_id/ space", body, options);
      route(result, "POST", "/vector_stores/vector_store_id%2F%20space/files/file_id%2F%20space");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = stores.remove_file_async("vector_store_id/ space", "file_id/ space", options).get();
      route(result, "DELETE", "/vector_stores/vector_store_id%2F%20space/files/file_id%2F%20space");
    }
    { auto result = stores.list_files("vector_store_id/ space", query, options);
      route(result, "GET", "/vector_stores/vector_store_id%2F%20space/files");
      require_resource(result.response.raw_json.at("query").at("after").at(0) == "cursor/+?", "Resource cursor was altered");
      require_resource(result.response.raw_json.at("query").at("include[]").size() == 2, "Repeated resource query fields lost");
    }
    { auto result = stores.file_content_async("vector_store_id/ space", "file_id/ space", query, options).get();
      route(result, "GET", "/vector_stores/vector_store_id%2F%20space/files/file_id%2F%20space/content");
      require_resource(result.response.raw_json.at("query").at("after").at(0) == "cursor/+?", "Resource cursor was altered");
      require_resource(result.response.raw_json.at("query").at("include[]").size() == 2, "Repeated resource query fields lost");
    }
    { auto result = stores.create_file_batch("vector_store_id/ space", body, options);
      route(result, "POST", "/vector_stores/vector_store_id%2F%20space/file_batches");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = stores.retrieve_file_batch_async("vector_store_id/ space", "batch_id/ space", options).get();
      route(result, "GET", "/vector_stores/vector_store_id%2F%20space/file_batches/batch_id%2F%20space");
    }
    { auto result = stores.cancel_file_batch("vector_store_id/ space", "batch_id/ space", options);
      route(result, "POST", "/vector_stores/vector_store_id%2F%20space/file_batches/batch_id%2F%20space/cancel");
    }
    { auto result = stores.list_file_batch_files_async("vector_store_id/ space", "batch_id/ space", query, options).get();
      route(result, "GET", "/vector_stores/vector_store_id%2F%20space/file_batches/batch_id%2F%20space/files");
      require_resource(result.response.raw_json.at("query").at("after").at(0) == "cursor/+?", "Resource cursor was altered");
      require_resource(result.response.raw_json.at("query").at("include[]").size() == 2, "Repeated resource query fields lost");
    }
    Containers containers(root);
    { auto result = containers.create(body, options);
      route(result, "POST", "/containers");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = containers.retrieve_async("container_id/ space", options).get();
      route(result, "GET", "/containers/container_id%2F%20space");
    }
    { auto result = containers.remove("container_id/ space", options);
      route(result, "DELETE", "/containers/container_id%2F%20space");
    }
    { auto result = containers.list_async(query, options).get();
      route(result, "GET", "/containers");
      require_resource(result.response.raw_json.at("query").at("after").at(0) == "cursor/+?", "Resource cursor was altered");
      require_resource(result.response.raw_json.at("query").at("include[]").size() == 2, "Repeated resource query fields lost");
    }
    { auto result = containers.create_file("container_id/ space", body, options);
      route(result, "POST", "/containers/container_id%2F%20space/files");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = containers.retrieve_file_async("container_id/ space", "file_id/ space", options).get();
      route(result, "GET", "/containers/container_id%2F%20space/files/file_id%2F%20space");
    }
    { auto result = containers.remove_file("container_id/ space", "file_id/ space", options);
      route(result, "DELETE", "/containers/container_id%2F%20space/files/file_id%2F%20space");
    }
    { auto result = containers.list_files_async("container_id/ space", query, options).get();
      route(result, "GET", "/containers/container_id%2F%20space/files");
      require_resource(result.response.raw_json.at("query").at("after").at(0) == "cursor/+?", "Resource cursor was altered");
      require_resource(result.response.raw_json.at("query").at("include[]").size() == 2, "Repeated resource query fields lost");
    }
    { auto result = containers.file_content("container_id/ space", "file_id/ space", options);
      route(result, "GET", "/containers/container_id%2F%20space/files/file_id%2F%20space/content");
    }
    Webhooks hooks(root);
    { auto result = hooks.create(body, options);
      route(result, "POST", "/webhook_endpoints");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = hooks.retrieve_async("webhook_endpoint_id/ space", options).get();
      route(result, "GET", "/webhook_endpoints/webhook_endpoint_id%2F%20space");
    }
    { auto result = hooks.update("webhook_endpoint_id/ space", body, options);
      route(result, "POST", "/webhook_endpoints/webhook_endpoint_id%2F%20space");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = hooks.remove_async("webhook_endpoint_id/ space", options).get();
      route(result, "DELETE", "/webhook_endpoints/webhook_endpoint_id%2F%20space");
    }
    { auto result = hooks.list(query, options);
      route(result, "GET", "/webhook_endpoints");
      require_resource(result.response.raw_json.at("query").at("after").at(0) == "cursor/+?", "Resource cursor was altered");
      require_resource(result.response.raw_json.at("query").at("include[]").size() == 2, "Repeated resource query fields lost");
    }
    { auto result = hooks.rotate_secret_async("webhook_endpoint_id/ space", body, options).get();
      route(result, "POST", "/webhook_endpoints/webhook_endpoint_id%2F%20space/rotate_secret");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    { auto result = hooks.test("webhook_endpoint_id/ space", body, options);
      route(result, "POST", "/webhook_endpoints/webhook_endpoint_id%2F%20space/test");
      require_resource(result.response.raw_json.at("json") == body, "Resource JSON fields were altered");
    }
    // The new independent clients snapshot themselves and per-request options.
    FutureApiResult pending;
    {
        Batch temporary(root);
        RequestOptions local;
        local.project = "snapshot-project";
        pending = temporary.list_async({}, local);
        local.project = "changed";
    }
    require_resource(pending.get().response.raw_json.at("headers").at("openai-project") == "snapshot-project", "Async resource options were not isolated");

    Uploads uploads(root);
    const auto created = uploads.create_async({{"bytes", 3}, {"filename", "a.bin"}, {"mime_type", "application/octet-stream"}, {"purpose", "batch"}, {"future", true}}, options).get();
    route(created, "POST", "/uploads");
    require_resource(created.response.raw_json.at("json").at("future") == true, "Upload unknown fields lost");
    std::vector<unsigned char> binary{'A', 0, 'B'};
    auto part_future = uploads.add_part_async("upload/ space", binary, options);
    binary.assign(3, 'X');
    auto part = part_future.get();
    route(part, "POST", "/uploads/upload%2F%20space/parts");
    require_resource(part.response.raw_json.at("multipart").at(0).at("name") == "data", "Upload part field mismatch");
    require_resource(part.response.raw_json.at("multipart").at(0).at("data_base64") == "QQBC", "Async upload binary data changed");
    auto completed = uploads.complete_checked_async("upload/ space", {{"part_second", 1}, {"part_first", 2}}, 3, {{"md5", "opaque-checksum"}}, options).get();
    route(completed, "POST", "/uploads/upload%2F%20space/complete");
    require_resource(completed.response.raw_json.at("json").at("part_ids") == json::array({"part_second", "part_first"}), "Upload completion order lost");
    route(uploads.complete("upload", {{"part_ids", {"p"}}, {"future", 9}}, options), "POST", "/uploads/upload/complete");
    route(uploads.cancel_async("upload/ space", options).get(), "POST", "/uploads/upload%2F%20space/cancel");
    invalid_resource([&] { uploads.create({{"bytes", -1}}); });
    invalid_resource([&] { uploads.create({{"bytes", 0}}); });
    invalid_resource([&] { uploads.create({{"bytes", 1.5}}); });
    invalid_resource([&] { uploads.create({{"bytes", Uploads::max_upload_bytes + 1}}); });
    invalid_resource([&] { uploads.add_part("upload", {}); });
    invalid_resource([&] { uploads.complete("upload", {{"part_ids", {"duplicate", "duplicate"}}}); });
    invalid_resource([&] { uploads.complete("upload", {{"part_ids", json::array()}}); });
    invalid_resource([&] { uploads.complete_checked("upload", {{"p", 1}}, 2); });
    invalid_resource([&] { uploads.complete_checked("upload", {{"p", 2}}, 1); });
    invalid_resource([&] { uploads.complete_checked("upload", {{"p", Uploads::max_part_bytes+1}}, Uploads::max_part_bytes+1); });
    invalid_resource([&] { uploads.complete_checked("upload", {{"p", 1}}, 1, {{"part_ids", {"override"}}}); });

    const auto file = std::filesystem::temp_directory_path() / ("liboai-resource-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".bin");
    struct Cleanup { std::filesystem::path file; ~Cleanup() { std::error_code ec; std::filesystem::remove(file, ec); } } cleanup{file};
    { std::ofstream out(file, std::ios::binary); out.write("A\0B", 3); }
    const auto file_part = uploads.add_part_file_async("u", file).get();
    require_resource(file_part.response.raw_json.at("multipart").at(0).at("data_base64") == "QQBC", "Upload file bytes changed");
    const auto container_file = containers.upload_file_async("c/1", file).get();
    route(container_file, "POST", "/containers/c%2F1/files");
    require_resource(container_file.response.raw_json.at("multipart").at(0).at("name") == "file", "Container file field mismatch");
    require_resource(container_file.response.raw_json.at("multipart").at(0).at("data_base64") == "QQBC", "Container upload binary content lost");
    { std::ofstream out(file, std::ios::trunc); }
    invalid_resource([&] { uploads.add_part_file("u", file); });
    std::filesystem::resize_file(file, Uploads::max_part_bytes + 1);
    invalid_resource([&] { uploads.add_part_file("u", file); });
    // Stateful workflows verify sequencing and resource IDs, not just route shape.
    Files files(root);
    Responses responses(root);
    RequestOptions workflow;
    workflow.headers["X-Fixture-Resource"] = "conversation";
    const auto conversation_id = conv.create(json::object(), workflow).response.raw_json.at("id").get<std::string>();
    const auto first = responses.create_with_options({{"model", "synthetic"}, {"conversation", conversation_id}, {"input", "Remember Maple"}}, workflow);
    require_resource(first.response.raw_json.at("turn") == 1, "First conversation turn missing");
    const auto second = responses.create_with_options({{"model", "synthetic"}, {"conversation", conversation_id}, {"input", "Recall it"}}, workflow);
    require_resource(second.response.raw_json.at("turn") == 2 && second.response.raw_json.at("first_input") == "Remember Maple", "Durable conversation context lost");
    const auto page_one = conv.list_items(conversation_id, {{"limit", "1"}}, workflow).response.raw_json;
    require_resource(page_one.at("has_more") == true, "Conversation fixture must expose a next page");
    const auto page_two = conv.list_items(conversation_id, {{"after", page_one.at("last_id").get<std::string>()}, {"limit", "1"}}, workflow).response.raw_json;
    require_resource(page_two.at("has_more") == false && page_two.at("data").at(0).at("id") == "item_2", "Conversation pagination did not advance");
    conv.remove(conversation_id, workflow);

    { std::ofstream out(file, std::ios::binary); out << "Synthetic document"; }
    workflow.headers["X-Fixture-Resource"] = "vector";
    const auto source_id = files.create_multipart({{"purpose", "assistants"}, {"file", netimpl::components::File{file.string()}}}, workflow).response.raw_json.at("id").get<std::string>();
    const auto store_id = stores.create({{"name", "fixture"}}, workflow).response.raw_json.at("id").get<std::string>();
    stores.create_file(store_id, {{"file_id", source_id}}, workflow);
    bool ingested = false;
    int ingest_polls = 0;
    for (; ingest_polls < 3; ++ingest_polls) {
        const auto status = stores.retrieve_file(store_id, source_id, workflow).response.raw_json.at("status").get<std::string>();
        if (status == "completed") { ingested = true; break; }
        require_resource(status == "in_progress", "Unexpected ingestion state");
    }
    require_resource(ingested && ingest_polls == 1, "Ingestion must transition through in_progress before completion");
    const auto searched = responses.create_with_options({{"model", "synthetic"}, {"input", "Read source"}, {"tools", json::array({{{"type", "file_search"}, {"vector_store_ids", {store_id}}}})}}, workflow).response.raw_json;
    require_resource(searched.at("output").at(0).at("content").at(0).at("annotations").at(0).at("file_id") == source_id, "File search lost source provenance");
    stores.remove(store_id, workflow);

    workflow.headers["X-Fixture-Resource"] = "batch";
    const auto input_id = files.create_multipart({{"purpose", "batch"}, {"file", netimpl::components::File{file.string()}}}, workflow).response.raw_json.at("id").get<std::string>();
    const auto batch_id = batch.create({{"input_file_id", input_id}, {"endpoint", "/v1/responses"}, {"completion_window", "24h"}}, workflow).response.raw_json.at("id").get<std::string>();
    json terminal;
    int batch_polls = 0;
    for (; batch_polls < 4; ++batch_polls) {
        auto current = batch.retrieve(batch_id, workflow).response.raw_json;
        if (current.at("status") == "completed") { terminal = current; break; }
    }
    require_resource(!terminal.is_null() && batch_polls == 2, "Bounded batch polling missed its terminal state");
    const auto output = files.content(terminal.at("output_file_id").get<std::string>(), workflow).response.content;
    const auto errors = files.content(terminal.at("error_file_id").get<std::string>(), workflow).response.content;
    require_resource(json::parse(output).at("custom_id") == "success", "Batch output file missing");
    require_resource(json::parse(errors).at("custom_id") == "failure", "Batch error file missing");

    workflow.headers["X-Fixture-Resource"] = "uploads";
    const auto upload_id = uploads.create({{"bytes", 3}, {"filename", "parts.bin"}, {"purpose", "batch"}, {"mime_type", "application/octet-stream"}}, workflow).response.raw_json.at("id").get<std::string>();
    const auto p1 = uploads.add_part(upload_id, {'B', 'C'}, workflow).response.raw_json.at("id").get<std::string>();
    const auto p2 = uploads.add_part(upload_id, {'A'}, workflow).response.raw_json.at("id").get<std::string>();
    const auto assembled = uploads.complete_checked(upload_id, {{p2, 1}, {p1, 2}}, 3, json::object(), workflow).response.raw_json;
    require_resource(assembled.at("file").at("bytes") == 3 && assembled.at("assembled_base64") == "QUJD", "Upload assembly must follow declared order, not completion order");
    workflow.headers["X-Fixture-Resource"] = "container-content";
    const auto binary_content = containers.file_content("container", "file", workflow).response.content;
    require_resource(binary_content == std::string("container\0\xff" "bytes", 16), "Container binary content changed");

}
