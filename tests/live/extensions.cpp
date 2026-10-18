#include "common.h"
#include "components/decisions.h"
#include "components/skills.h"

namespace {
void observe_metadata(const std::string& name, const ApiResult& result) {
    record(name, "OBSERVED", {{"http", result.response.status_code},
        {"request_id", result.request_id()}, {"metadata", result.response.raw_json}});
}

void verify_archive(const std::string& name, const ApiResult& result) {
    check(result.response.content.size() > 4, "Downloaded Skills archive is empty");
    check(result.response.content.substr(0, 2) == "PK", "Downloaded Skills archive lacks ZIP signature");
    check(result.response.raw_json.is_null(), "Binary Skills archive was parsed as JSON");
    record(name, "PASS", {{"http", result.response.status_code},
        {"request_id", result.request_id()}, {"bytes", result.response.content.size()}});
}

void decisions_smoke() {
    Decisions decisions;
    // Exactly one synthetic model request: no retries, fallback model, or async duplicate.
    const auto result = decisions.create({{"model", "gpt-6-luna"},
        {"input", "Synthetic test: a parcel arrived with a cracked screen. The screen still displays an image."},
        {"questions", J::array({
            {{"type", "predicate"}, {"name", "damaged"},
                {"instructions", "Does the synthetic report mention physical damage?"}},
            {{"type", "choice"}, {"name", "route"},
                {"instructions", "Choose the appropriate category for this synthetic report."},
                {"choices", J::array({{{"value", "damage"}}, {{"value", "delivery_delay"}}})}},
            {{"type", "score"}, {"name", "severity"},
                {"instructions", "Rate reported damage severity, from none to severe."},
                {"levels", J::array({{{"label", "none"}}, {{"label", "moderate"}}, {{"label", "severe"}}})}}
        })}}, opts());
    const auto& raw = result.response.raw_json;
    const auto answers = parse_decision_answers(raw);
    check(answers.size() == 3, "Expected three Decisions answers");
    check(answers[0].name == "damaged" && answers[1].name == "route" && answers[2].name == "severity",
        "Decisions answer names or order changed");
    check(std::holds_alternative<DecisionPredicate>(answers[0].value), "Predicate answer missing");
    check(std::holds_alternative<DecisionChoice>(answers[1].value), "Choice answer missing");
    check(std::holds_alternative<DecisionScore>(answers[2].value), "Score answer missing");
    J keys = J::array();
    for (auto it = raw.begin(); it != raw.end(); ++it) keys.push_back(it.key());
    const auto& predicate = std::get<DecisionPredicate>(answers[0].value);
    const auto& choice = std::get<DecisionChoice>(answers[1].value);
    const auto& score = std::get<DecisionScore>(answers[2].value);
    const J selected = std::visit([](const auto& value) { return J(value); }, choice.choice);
    record("decisions.shape", "OBSERVED", {{"http", result.response.status_code},
        {"request_id", result.request_id()}, {"top_level_keys", keys}, {"raw_answers", raw.at("answers")},
        {"typed_predicate_probability", predicate.probability}, {"typed_choice", selected},
        {"typed_score", score.score}, {"usage", raw.value("usage", J())}});
}

void skills_smoke() {
    Skills skills;
    using namespace netimpl::components;
    std::string manifest = "---\nname: liboai-synthetic-live\ndescription: Summarize one synthetic note.\n---\n"
        "Revision one. Summarize only synthetic text. Do not execute commands.\n";
    Buffer manifest_buffer(manifest.begin(), manifest.end(),
        std::filesystem::path("liboai-synthetic-live/SKILL.md"));
    Multipart first{{"files[]", manifest_buffer, "text/markdown"}};
    const auto created = skills.create(first, opts());
    const auto skill_id = created.response.raw_json.at("id").get<std::string>();
    // Register immediately, before parsing any other creation fields or asserting.
    // Deleting this parent also removes every version if a later step fails.
    own("skill", skill_id, [skill_id] {
        const auto removed = Skills().remove(skill_id, opts());
        check(removed.response.raw_json.at("deleted") == true, "Skill cleanup not acknowledged");
        record("skills.remove", "PASS", {{"id", skill_id}, {"http", removed.response.status_code},
            {"request_id", removed.request_id()}});
    });
    const auto initial_version = created.response.raw_json.at("default_version").get<std::string>();
    observe_metadata("skills.create", created);

    std::string revision = "---\nname: liboai-synthetic-live\ndescription: Summarize one synthetic note.\n---\n"
        "Revision two. Summarize only synthetic text in one short sentence. Do not execute commands.\n";
    Buffer revision_buffer(revision.begin(), revision.end(),
        std::filesystem::path("liboai-synthetic-live/SKILL.md"));
    Multipart second{{"files[]", revision_buffer, "text/markdown"}, {"default", "false"}};
    const auto added = skills.create_version(skill_id, second, opts());
    const auto version = added.response.raw_json.at("version").get<std::string>();
    const auto version_id = added.response.raw_json.at("id").get<std::string>();
    observe_metadata("skills.create_version", added);
    check(added.response.raw_json.at("skill_id") == skill_id, "Version parent mismatch");
    check(version != initial_version, "New skill version did not advance");

    const auto readiness_start = std::chrono::steady_clock::now();
    // Poll only the acknowledged version ordinal. No upload/default mutation is replayed.
    wait_for_readiness("skills.version_readiness", [&] {
        const auto retrieved = skills.retrieve_version(skill_id, version, opts());
        check(retrieved.response.raw_json.at("id") == version_id, "Retrieved version object ID mismatch");
        check(retrieved.response.raw_json.at("version") == version, "Retrieved version ordinal mismatch");
        observe_metadata("skills.retrieve_version", retrieved);
        return true;
    }, 13, 10);
    record("skills.readiness_timing", "OBSERVED", {{"elapsed_seconds",
        std::chrono::duration<double>(std::chrono::steady_clock::now() - readiness_start).count()},
        {"max_attempts", 13}, {"delay_seconds", 10}});

    const auto retrieved = skills.retrieve(skill_id, opts());
    check(retrieved.response.raw_json.at("id") == skill_id, "Retrieved skill mismatch");
    observe_metadata("skills.retrieve", retrieved);
    const auto listed = skills.list({{"limit", "10"}, {"order", "desc"}}, opts());
    check(listed.response.raw_json.at("data").is_array(), "Skills list data missing");
    bool found = false;
    for (const auto& item : listed.response.raw_json.at("data")) if (item.at("id") == skill_id) found = true;
    record("skills.list", "OBSERVED", {{"http", listed.response.status_code},
        {"count", listed.response.raw_json.at("data").size()},
        {"has_more", listed.response.raw_json.value("has_more", false)}, {"own_skill_found", found}});
    // A bounded list can omit this resource if other clients created more recent skills.
    // Direct retrieval above establishes identity independently of that observation.
    const auto versions = skills.list_versions(skill_id, {{"limit", "10"}, {"order", "asc"}}, opts());
    check(versions.response.raw_json.at("data").is_array(), "Skill version list missing");
    bool found_initial = false, found_added = false;
    for (const auto& item : versions.response.raw_json.at("data")) {
        if (item.at("version") == initial_version) found_initial = true;
        if (item.at("version") == version) found_added = true;
    }
    observe_metadata("skills.list_versions", versions);
    check(found_initial && found_added, "Created skill versions missing from list");
    verify_archive("skills.content", skills.content(skill_id, opts()));
    verify_archive("skills.version_content", skills.version_content(skill_id, version, opts()));

    // Each pointer mutation is submitted once, and its direct response is recorded.
    const auto updated = skills.update(skill_id, {{"default_version", version}}, opts());
    observe_metadata("skills.update_default", updated);
    check(updated.response.raw_json.at("default_version") == version, "Default update response mismatch");
    const auto restored = skills.update(skill_id, {{"default_version", initial_version}}, opts());
    observe_metadata("skills.restore_default", restored);
    check(restored.response.raw_json.at("default_version") == initial_version, "Default restore response mismatch");
    const auto removed_version = skills.remove_version(skill_id, version, opts());
    observe_metadata("skills.remove_version", removed_version);
    check(removed_version.response.raw_json.at("deleted") == true, "Version deletion not acknowledged");
    // Whole-skill deletion happens exactly once through execute's registered cleanup.
    // No Responses request is made and no bundle code is extracted or executed.
}
}

int main() {
    return execute("extensions", [] {
        run("decisions.synthetic_three_questions", decisions_smoke);
        run("skills.lifecycle", skills_smoke);
    });
}
