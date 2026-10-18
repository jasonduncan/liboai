#include "components/skills.h"
#include <iostream>

// Explicitly invoked example: uploads only the synthetic manifest below.
// It prints a Responses attachment request, but never sends a model request.
int main(int argc, char** argv) {
    if (argc != 3 || (std::string(argv[1]) != "--upload" && std::string(argv[1]) != "--delete")) {
        std::cerr << "Usage: skills --upload MODEL | skills --delete SKILL_ID\n";
        return 1;
    }
    if (!liboai::Authorization::Authorizer().SetKeyEnv("OPENAI_API_KEY")) {
        std::cerr << "Set OPENAI_API_KEY before explicitly running this example.\n";
        return 1;
    }
    liboai::Skills skills;
    std::string created_id;
    try {
        if (std::string(argv[1]) == "--delete") {
            std::cout << skills.remove(argv[2]).response << '\n';
            return 0;
        }
        using namespace liboai::netimpl::components;
        std::string manifest = "---\nname: synthetic-summary\ndescription: Summarize a synthetic note in one sentence.\n---\n"
            "When given a synthetic note, summarize it in one sentence. Do not run commands.\n";
        Buffer file(manifest.begin(), manifest.end(), std::filesystem::path("synthetic-summary/SKILL.md"));
        Multipart bundle{{"files[]", file, "text/markdown"}};
        auto created = skills.create(bundle);
        created_id = created.response.raw_json.at("id").get<std::string>();
        std::cout << "Uploaded " << created_id << '\n' << skills.retrieve(created_id).response << '\n';

        // A new upload creates an immutable version; it does not overwrite version 1.
        auto version = skills.create_version(created_id, bundle).response.raw_json.at("version").get<std::string>();
        std::cout << skills.retrieve_version(created_id, version).response << '\n';
        auto downloaded = skills.version_content(created_id, version);
        std::cout << "Downloaded " << downloaded.response.content.size() << " archive bytes; not extracted or executed.\n";
        skills.update(created_id, {{"default_version", version}});
        nlohmann::json request = {
            {"model", argv[2]}, {"input", "Use the skill to summarize this synthetic note: the fixture passed."},
            {"max_output_tokens", 128},
            {"tools", nlohmann::json::array({{{"type", "shell"}, {"environment", {
                {"type", "container_auto"}, {"skills", nlohmann::json::array({{
                    {"type", "skill_reference"}, {"skill_id", created_id}, {"version", version}
                }})}
            }}}})}
        };
        std::cout << "Review this pinned hosted-shell request before using Responses; it has not been sent:\n"
                  << request.dump(2) << '\n';
        std::cout << "Cleanup when finished: skills --delete " << created_id << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        if (!created_id.empty()) std::cerr << "Created resource remains; cleanup: skills --delete " << created_id << '\n';
        return 2;
    }
}
