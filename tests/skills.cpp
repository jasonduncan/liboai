#include "components/skills.h"
#include "components/responses.h"
#include <stdexcept>

namespace {
using Json = nlohmann::json;
using namespace liboai::netimpl::components;
void require(bool value, const char* description) {
    if (!value) throw std::runtime_error(std::string("Skills: ") + description);
}
void route(const liboai::ApiResult& result, const std::string& method, const std::string& path) {
    require(result.response.raw_json.at("method") == method, "HTTP method");
    require(result.response.raw_json.at("path") == "/v1" + path, "encoded path");
}
}

void test_skills(const std::string& root) {
    liboai::Skills skills(root);
    liboai::RequestOptions options;
    options.headers["X-Fixture-Extension"] = "skills";
    options.project = "proj_skills";
    std::string manifest = "---\nname: synthetic\ndescription: Test fixture only\n---\nNo execution.\n";
    std::vector<unsigned char> asset = {'P', 'K', 3, 4, 0, 255};
    Buffer text(manifest.begin(), manifest.end(), std::filesystem::path("synthetic/SKILL.md"));
    Buffer bytes(asset.begin(), asset.end(), std::filesystem::path("synthetic/assets/binary.bin"));
    Multipart bundle{{"files[]", text, "text/markdown"}, {"files[]", bytes, "application/octet-stream"}, {"future", "kept"}};
    auto created = skills.create(bundle, options);
    route(created, "POST", "/skills");
    const auto& parts = created.response.raw_json.at("multipart");
    require(parts.size() == 3, "multiple directory entries");
    require(parts.at(0).at("name") == "files[]" && parts.at(0).at("filename") == "synthetic/SKILL.md", "relative folder filename");
    require(parts.at(0).at("data") == manifest, "manifest bytes");
    require(parts.at(0).at("content_type") == "text/markdown" && parts.at(1).at("content_type") == "application/octet-stream", "directory MIME types");
    require(parts.at(1).at("filename") == "synthetic/assets/binary.bin" && parts.at(1).at("data_base64") == "UEsDBAD/", "binary multipart bytes");
    require(parts.at(2).at("data") == "kept", "unknown multipart fields");
    require(created.response.raw_json.at("headers").at("openai-project") == "proj_skills", "request project");
    require(!created.response.raw_json.at("headers").contains("openai-beta"), "no invented beta header");

    // Snapshot before worker launch; neither caller memory nor client lifetime is borrowed.
    auto future = liboai::Skills(root).create_async(bundle, options);
    manifest.assign(manifest.size(), 'X');
    asset.assign(asset.size(), 0);
    options.project = "changed_after_launch";
    auto copied = future.get();
    route(copied, "POST", "/skills");
    require(copied.response.raw_json.at("multipart") == parts, "async buffer snapshot");
    require(copied.response.raw_json.at("headers").at("openai-project") == "proj_skills", "async options snapshot");
    options.project = "proj_skills";

    const Json update = {{"default_version", "2"}, {"future", {{"nested", true}}}};
    auto updated = skills.update("skill /?#", update, options);
    route(updated, "POST", "/skills/skill%20%2F%3F%23");
    require(updated.response.raw_json.at("json") == update, "raw update JSON");
    route(skills.update_async("skill", update, options).get(), "POST", "/skills/skill");
    route(skills.retrieve("skill", options), "GET", "/skills/skill");
    route(skills.retrieve_async("skill", options).get(), "GET", "/skills/skill");
    route(skills.remove("skill", options), "DELETE", "/skills/skill");
    route(skills.remove_async("skill", options).get(), "DELETE", "/skills/skill");
    auto listed = skills.list({{"after", "cursor +/"}, {"limit", "20"}, {"order", "asc"}, {"future[]", "one"}, {"future[]", "two"}}, options);
    route(listed, "GET", "/skills");
    require(listed.response.raw_json.at("query").at("after") == Json::array({"cursor +/"}), "escaped cursor");
    require(listed.response.raw_json.at("query").at("future[]") == Json::array({"one", "two"}), "unknown repeated query");
    route(skills.list_async({}, options).get(), "GET", "/skills");

    std::string zip("PK\x03\x04\x00\xff", 6);
    Buffer zip_bytes(zip.begin(), zip.end(), std::filesystem::path("synthetic.zip"));
    Multipart archive{{"files", zip_bytes, "application/zip"}, {"default", "true"}, {"future", "kept"}};
    auto version = skills.create_version("skill", archive, options);
    route(version, "POST", "/skills/skill/versions");
    require(version.response.raw_json.at("multipart").at(0).at("data_base64") == "UEsDBAD/", "ZIP bytes");
    require(version.response.raw_json.at("multipart").at(0).at("name") == "files", "ZIP field name");
    require(version.response.raw_json.at("multipart").at(0).at("content_type") == "application/zip", "ZIP MIME type");
    require(version.response.raw_json.at("multipart").at(1).at("data") == "true", "default multipart field");
    auto version_future = liboai::Skills(root).create_version_async("skill", archive, options);
    zip.assign(zip.size(), 'X');
    require(version_future.get().response.raw_json.at("multipart") == version.response.raw_json.at("multipart"), "version async snapshot");
    route(skills.list_versions("skill", {{"after", "version +/"}, {"order", "asc"}}, options), "GET", "/skills/skill/versions");
    route(skills.list_versions_async("skill", {}, options).get(), "GET", "/skills/skill/versions");
    route(skills.retrieve_version("skill /", "2 /", options), "GET", "/skills/skill%20%2F/versions/2%20%2F");
    route(skills.retrieve_version_async("skill", "2", options).get(), "GET", "/skills/skill/versions/2");
    route(skills.remove_version("skill", "2", options), "DELETE", "/skills/skill/versions/2");
    route(skills.remove_version_async("skill", "2", options).get(), "DELETE", "/skills/skill/versions/2");

    const std::string expected("PK\x03\x04\x00\xffsynthetic-zip-content", 27);
    const auto downloads = std::vector<std::pair<liboai::ApiResult, std::string>>{
        {skills.content("skill /", options), "/v1/skills/skill%20%2F/content"},
        {skills.content_async("skill", options).get(), "/v1/skills/skill/content"},
        {skills.version_content("skill", "2 /", options), "/v1/skills/skill/versions/2%20%2F/content"},
        {skills.version_content_async("skill", "2", options).get(), "/v1/skills/skill/versions/2/content"}};
    for (const auto& download : downloads) {
        const auto& result = download.first;
        require(result.response.content == expected, "binary download preserved");
        require(result.response.raw_json.is_null(), "archive remains binary");
        require(result.request_id() == "req_skill_content", "download diagnostics");
        require(result.headers.at("X-Fixture-Method") == "GET" && result.headers.at("X-Fixture-Path") == download.second, "content request route");
    }

    options.headers["X-Fixture-Skills-Pages"] = "true";
    auto first = skills.list({}, options).response.raw_json;
    auto second = skills.list({{"after", first.at("last_id").get<std::string>()}}, options).response.raw_json;
    require(first.at("has_more") == true && second.at("has_more") == false, "skill pagination");
    first = skills.list_versions("skill", {}, options).response.raw_json;
    second = skills.list_versions("skill", {{"after", first.at("last_id").get<std::string>()}}, options).response.raw_json;
    require(first.at("has_more") == true && second.at("has_more") == false, "version pagination");
    require(second.at("data").at(0).at("unknown") == 2, "unknown returned fields");
    options.headers.erase("X-Fixture-Skills-Pages");
    bool error = false;
    try { skills.retrieve("error", options); }
    catch (const liboai::exception::ApiError& e) {
        error = e.result.response.status_code == 404 && e.error.at("code") == "skill_not_found" && e.error.at("future") == true;
    }
    require(error, "structured server errors");
    bool invalid = false;
    try { skills.retrieve_version("skill", "..", options); } catch (const std::invalid_argument&) { invalid = true; }
    require(invalid, "invalid path segment rejected");

    // A bounded end-to-end fake workflow: use returned IDs and versions, then
    // prove changing the default pointer cannot change an explicitly pinned request.
    options.headers["X-Fixture-Skills-Workflow"] = "true";
    std::string workflow_manifest = "---\nname: synthetic-workflow\ndescription: Fixture only\n---\nNo execution.\n";
    Buffer workflow_file(workflow_manifest.begin(), workflow_manifest.end(), std::filesystem::path("synthetic-workflow/SKILL.md"));
    Multipart workflow_bundle{{"files[]", workflow_file, "text/markdown"}};
    auto workflow_skill = skills.create(workflow_bundle, options).response.raw_json;
    auto skill_id = workflow_skill.at("id").get<std::string>();
    auto initial_version = workflow_skill.at("default_version").get<std::string>();
    auto workflow_version = skills.create_version(skill_id, workflow_bundle, options).response.raw_json;
    auto pinned_version = workflow_version.at("version").get<std::string>();
    require(workflow_version.at("skill_id") == skill_id && pinned_version != initial_version, "workflow version identity");
    skills.update(skill_id, {{"default_version", pinned_version}}, options);
    require(skills.retrieve(skill_id, options).response.raw_json.at("default_version") == pinned_version, "workflow default update");
    require(skills.retrieve_version(skill_id, pinned_version, options).response.raw_json.at("id") == workflow_version.at("id"), "workflow version retrieve");
    require(skills.content(skill_id, options).headers.at("X-Fixture-Version") == pinned_version, "default content follows selected version");

    skills.update(skill_id, {{"default_version", initial_version}}, options);
    require(skills.retrieve(skill_id, options).response.raw_json.at("default_version") == initial_version, "workflow default changed again");
    require(skills.version_content(skill_id, pinned_version, options).headers.at("X-Fixture-Version") == pinned_version, "pinned download independent of default");
    Json attachment = {{"model", "fixture-model"}, {"input", "Synthetic workflow only"}, {"max_output_tokens", 16},
        {"tools", Json::array({{{"type", "shell"}, {"environment", {{"type", "container_auto"},
            {"skills", Json::array({{{"type", "skill_reference"}, {"skill_id", skill_id}, {"version", pinned_version}}})}}}}})}};
    liboai::Responses responses(root);
    auto attached = responses.create_with_options(attachment, options).response.raw_json;
    require(attached.at("captured_request") == attachment, "hosted shell attachment JSON preserved");
    require(attached.at("resolved_version") == pinned_version && attached.at("default_version") == initial_version, "attachment remains pinned after default changes");
    auto removed = skills.remove(skill_id, options).response.raw_json;
    require(removed.at("id") == skill_id && removed.at("deleted") == true, "workflow cleanup");
    bool cleaned_up = false;
    try { skills.retrieve(skill_id, options); }
    catch (const liboai::exception::ApiError& e) { cleaned_up = e.result.response.status_code == 404; }
    require(cleaned_up, "workflow resource no longer exists");
}
