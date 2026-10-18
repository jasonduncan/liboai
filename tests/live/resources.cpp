#include "common.h"
#include <memory>

static constexpr const char* root = "https://api.openai.com/v1";

// Opt-in only. One batch request (gpt-4.1-nano, at most 16 output tokens),
// one 1 GiB container, synthetic text only. No create/generation retries.
// Ingestion uses three reads, spaced three seconds apart. Batch cleanup uses
// at most 61 polling reads spaced ten seconds apart, stopping at 900 seconds
// since suite start to leave time for other cleanup. Unsettled IDs remain in
// the durable receipt. The launcher bounds the entire process to 1200 seconds.
// Every cleanup owns its IDs by value and creates fresh clients.
static void own_file(const std::string& file) {
    own("file", file, [file] {
        Files files(root);
        check(files.remove(file).raw_json.value("deleted", false), "File deletion not confirmed");
        release(file);
    });
}

static void own_upload(const std::string& upload) {
    own("upload", upload, [upload] {
        Uploads uploads;
        check(uploads.cancel(upload, opts()).response.raw_json.at("status") == "cancelled",
              "Upload cancellation not confirmed");
        release(upload);
    });
}

struct BatchOwnership {
    bool submitted = false;
    bool terminal = false;
    bool cancel_attempted = false;
    const std::chrono::steady_clock::time_point deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(900);
};

static std::string id(const ApiResult& r) {
    return r.response.raw_json.at("id").get<std::string>();
}

static bool terminal(const std::string& s) {
    return s == "completed" || s == "cancelled" || s == "expired" || s == "failed";
}

int main() {
    return execute("resources", [] {
        Authorization::Authorizer().SetMaxTimeout(45000);
        auto o = opts();
        Conversations conv;
        Uploads uploads;
        VectorStores stores;
        Containers containers;
        Batch batches;
        Webhooks hooks;
        Files files(root);
        std::string conversation, item, upload_cancel, upload_complete, source, store,
            vector_batch, container, container_file, batch_input, batch;
        auto batch_ownership = std::make_shared<BatchOwnership>();

        run("conversations.create", [&] {
            conversation = id(conv.create({{"metadata", {{"purpose", "liboai-live-synthetic"}}}}, o));
            own("conversation", conversation, [conversation] {
                Conversations client;
                check(client.remove(conversation, opts()).response.raw_json.value("deleted", false),
                      "Conversation deletion not confirmed");
                release(conversation);
            });
        });
        if (!conversation.empty()) {
            run("conversations.retrieve", [&] {
                check(id(conv.retrieve(conversation, o)) == conversation, "Conversation ID mismatch");
            });
            run("conversations.update", [&] {
                conv.update(conversation,
                            {{"metadata", {{"purpose", "liboai-live-synthetic-updated"}}}}, o);
            });
            run("conversations.create_items", [&] {
                auto r = conv.create_items(conversation, {
                    {"items", J::array({{
                        {"type", "message"}, {"role", "user"}, {"content", "Synthetic marker: cedar."}
                    }})}
                }, {}, o).response.raw_json;
                item = r.at("data").at(0).at("id").get<std::string>();
                own("conversation_item", item, [conversation, item] {
                    Conversations client;
                    check(client.remove_item(conversation, item, opts()).response.raw_json.value("deleted", false),
                          "Item deletion not confirmed");
                    release(item);
                }, conversation);
            });
            run("conversations.list_items", [&] {
                auto r = conv.list_items(conversation, {{"limit", "1"}, {"order", "asc"}}, o).response.raw_json;
                check(r.at("data").is_array(), "Missing item list");
            });
            if (!item.empty()) {
                run("conversations.retrieve_item", [&] {
                    conv.retrieve_item(conversation, item, {}, o);
                });
            }
        }

        const std::string synthetic = "The synthetic marker is cedar. This is a liboai live API smoke test.\n";
        run("uploads.create_cancel", [&] {
            upload_cancel = id(uploads.create({
                {"bytes", synthetic.size()}, {"filename", "liboai-synthetic.txt"},
                {"mime_type", "text/plain"}, {"purpose", "assistants"}
            }, o));
            own_upload(upload_cancel);
        });
        if (!upload_cancel.empty()) {
            run("uploads.cancel", [&] {
                check(uploads.cancel(upload_cancel, o).response.raw_json.at("status") == "cancelled",
                      "Upload not cancelled");
                release(upload_cancel);
                upload_cancel.clear();
            });
        }
        run("uploads.create_complete", [&] {
            upload_complete = id(uploads.create({
                {"bytes", synthetic.size()}, {"filename", "liboai-synthetic.txt"},
                {"mime_type", "text/plain"}, {"purpose", "assistants"}
            }, o));
            own_upload(upload_complete);
        });
        if (!upload_complete.empty()) {
            std::string part;
            run("uploads.add_part", [&] {
                part = id(uploads.add_part(upload_complete,
                    std::vector<unsigned char>(synthetic.begin(), synthetic.end()), o));
            });
            if (!part.empty()) {
                run("uploads.complete_checked", [&] {
                    auto r = uploads.complete_checked(upload_complete, {{part, synthetic.size()}},
                        synthetic.size(), J::object(), o).response.raw_json;
                    source = r.at("file").at("id").get<std::string>();
                    own_file(source);
                    release(upload_complete);
                    upload_complete.clear();
                });
            }
        }

        run("files.list_with_options", [&] {
            auto r = files.list_with_options({
                {"purpose", "assistants"}, {"limit", "1"}, {"order", "desc"}
            }, o).response.raw_json;
            check(r.at("data").is_array(), "Missing files list");
        });
        if (!source.empty()) {
            run("files.retrieve", [&] {
                check(files.retrieve(source).raw_json.at("id") == source, "File ID mismatch");
            });
            run("vector_stores.create", [&] {
                store = id(stores.create({
                    {"name", "liboai-live-synthetic"},
                    {"expires_after", {{"anchor", "last_active_at"}, {"days", 1}}}
                }, o));
                own("vector_store", store, [store] {
                    VectorStores client;
                    check(client.remove(store, opts()).response.raw_json.value("deleted", false),
                          "Vector store deletion not confirmed");
                    release(store);
                });
            });
        }
        if (!store.empty()) {
            run("vector_stores.retrieve", [&] {
                stores.retrieve(store, o);
            });
            run("vector_stores.update", [&] {
                stores.update(store, {{"metadata", {{"test", "synthetic"}}}}, o);
            });
            run("vector_stores.list", [&] {
                stores.list({{"limit", "1"}}, o);
            });
            bool attached = run("vector_stores.create_file", [&] {
                stores.create_file(store, {{"file_id", source}}, o);
            });
            bool ready = false;
            if (attached) {
                run("vector_stores.retrieve_file", [&] {
                    wait_for_readiness("vector_stores.ingestion", [&] {
                        const auto status = stores.retrieve_file(store, source, o)
                            .response.raw_json.at("status").get<std::string>();
                        if (status == "completed") {
                            return true;
                        }
                        check(status == "in_progress", "Vector ingestion failed or cancelled");
                        return false;
                    }, 3, 3);
                    ready = true;
                });
                run("vector_stores.update_file", [&] {
                    stores.update_file(store, source, {{"attributes", {{"test", "synthetic"}}}}, o);
                });
                run("vector_stores.list_files", [&] {
                    stores.list_files(store, {{"limit", "1"}}, o);
                });
                if (ready) {
                    run("vector_stores.file_content", [&] {
                        auto r = stores.file_content(store, source, {}, o).response.raw_json;
                        check(r.at("data").is_array(), "Missing parsed content");
                    });
                    run("vector_stores.search", [&] {
                        auto r = stores.search(store, {
                            {"query", "What is the synthetic marker?"}, {"max_num_results", 1}
                        }, o).response.raw_json;
                        check(!r.at("data").empty(), "No synthetic search hit");
                    });
                }
                run("vector_stores.remove_file", [&] {
                    stores.remove_file(store, source, o);
                });
            }
            run("vector_stores.create_file_batch", [&] {
                vector_batch = id(stores.create_file_batch(store, {{"file_ids", {source}}}, o));
            });
            if (!vector_batch.empty()) {
                std::string state;
                run("vector_stores.retrieve_file_batch", [&] {
                    state = stores.retrieve_file_batch(store, vector_batch, o)
                        .response.raw_json.at("status").get<std::string>();
                });
                run("vector_stores.list_file_batch_files", [&] {
                    stores.list_file_batch_files(store, vector_batch, {{"limit", "1"}}, o);
                });
                if (state == "in_progress") {
                    run("vector_stores.cancel_file_batch", [&] {
                        stores.cancel_file_batch(store, vector_batch, o);
                    });
                } else {
                    record("vector_stores.cancel_file_batch", "SKIPPED", {
                        {"reason", "Batch already terminal"}, {"state", state}
                    });
                }
            }
        }

        run("containers.create", [&] {
            container = id(containers.create({
                {"name", "liboai-live-synthetic"}, {"memory_limit", "1g"}
            }, o));
            own("container", container, [container] {
                Containers client;
                client.remove(container, opts());
                release(container);
            });
        });
        if (!container.empty()) {
            run("containers.retrieve", [&] {
                containers.retrieve(container, o);
            });
            run("containers.list", [&] {
                containers.list({{"limit", "1"}}, o);
            });
            const std::filesystem::path container_path = artifact("container-synthetic.txt");
            {
                std::ofstream out(container_path, std::ios::binary);
                out << synthetic;
                check(static_cast<bool>(out), "Cannot write synthetic container file");
            }
            run("containers.upload_file", [&] {
                container_file = id(containers.upload_file(container, container_path, o));
                own("container_file", container_file, [container, container_file] {
                    Containers client;
                    client.remove_file(container, container_file, opts());
                    release(container_file);
                }, container);
            });
            if (!container_file.empty()) {
                run("containers.retrieve_file", [&] {
                    containers.retrieve_file(container, container_file, o);
                });
                run("containers.file_content", [&] {
                    check(containers.file_content(container, container_file, o).response.content == synthetic,
                          "Container content differs");
                });
                run("containers.remove_file", [&] {
                    containers.remove_file(container, container_file, o);
                    release(container_file);
                    container_file.clear();
                });
            }
            if (!source.empty()) {
                run("containers.create_file", [&] {
                    container_file = id(containers.create_file(container, {{"file_id", source}}, o));
                    own("container_file", container_file, [container, container_file] {
                        Containers client;
                        client.remove_file(container, container_file, opts());
                        release(container_file);
                    }, container);
                });
            }
            run("containers.list_files", [&] {
                containers.list_files(container, {{"limit", "1"}}, o);
            });
        }

        const J batch_request = {
            {"custom_id", "liboai-synthetic-one"}, {"method", "POST"}, {"url", "/v1/responses"},
            {"body", {
                {"model", "gpt-4.1-nano"}, {"input", "Reply OK."},
                {"max_output_tokens", 16}, {"store", false}
            }}
        };
        auto batch_line = batch_request.dump() + "\n";
        run("files.create_multipart_batch", [&] {
            netimpl::components::Buffer bytes(batch_line.begin(), batch_line.end(),
                                             std::filesystem::path("liboai-synthetic.jsonl"));
            batch_input = id(files.create_multipart({
                {"purpose", "batch"}, {"file", bytes, "application/jsonl"}
            }, o));
            own("batch_input_file", batch_input, [batch_input, batch_ownership] {
                check(!batch_ownership->submitted || batch_ownership->terminal,
                      "Batch still active or submission outcome unknown; input retained");
                Files client(root);
                check(client.remove(batch_input).raw_json.value("deleted", false),
                      "Batch input deletion not confirmed");
                release(batch_input);
            });
        });
        if (!batch_input.empty()) {
            // Content permissions depend on purpose: use this batch file, not assistants.
            run("files.content", [&] {
                check(files.content(batch_input, o).response.content == batch_line, "Batch input content differs");
            });
            run("batch.create", [&] {
                // If submission times out, conservatively retain the input; do not retry.
                batch_ownership->submitted = true;
                batch = id(batches.create({
                    {"input_file_id", batch_input}, {"endpoint", "/v1/responses"}, {"completion_window", "24h"}
                }, o));
                own("batch", batch, [batch, batch_ownership] {
                    Batch client;
                    check(std::chrono::steady_clock::now() < batch_ownership->deadline,
                          "Batch cleanup deadline reached; IDs retained");
                    J state = client.retrieve(batch, opts()).response.raw_json;
                    if (!terminal(state.value("status", "")) && !batch_ownership->cancel_attempted) {
                        batch_ownership->cancel_attempted = true;
                        state = client.cancel(batch, opts()).response.raw_json;
                    }
                    if (!terminal(state.value("status", ""))) {
                        wait_for_readiness("batch.cancellation", [&] {
                            check(std::chrono::steady_clock::now() < batch_ownership->deadline,
                                  "Batch cleanup deadline reached; IDs retained");
                            state = client.retrieve(batch, opts()).response.raw_json;
                            return terminal(state.value("status", ""));
                        }, 61, 10);
                    }
                    // Save outputs before retiring the batch receipt, including failed batches.
                    for (const auto* key : {"output_file_id", "error_file_id"}) {
                        if (state.contains(key) && state.at(key).is_string()) {
                            const auto file = state.at(key).get<std::string>();
                            if (!file.empty()) {
                                own_file(file);
                                run(std::string("batch.") + key, [file] {
                                    Files files(root);
                                    check(!files.content(file, opts()).response.content.empty(),
                                          "Generated batch file is empty");
                                });
                            }
                        }
                    }
                    batch_ownership->terminal = true;
                    release(batch);
                    const auto status = state.value("status", "");
                    if (status == "failed" || status == "expired") {
                        record("batch.final_status", "FAIL", {
                            {"status", status}, {"errors", state.value("errors", J::object())}
                        });
                    } else {
                        record("batch.final_status", "OBSERVED", {{"status", status}});
                    }
                }, batch_input);
            });
        }
        if (!batch.empty()) {
            J state;
            run("batch.retrieve", [&] {
                state = batches.retrieve(batch, o).response.raw_json;
            });
            run("batch.list", [&] {
                batches.list({{"limit", "1"}}, o);
            });
            if (state.is_object() && terminal(state.value("status", ""))) {
                record("batch.cancel", "SKIPPED", {
                    {"reason", "Batch already terminal"}, {"state", state.value("status", "")}
                });
            } else {
                run("batch.cancel", [&] {
                    batch_ownership->cancel_attempted = true;
                    state = batches.cancel(batch, o).response.raw_json;
                });
            }
            // Cleanup performs bounded terminal polling and discovers any generated files.
        }

        run("webhooks.list_only", [&] {
            auto r = hooks.list({{"limit", "1"}}, o).response.raw_json;
            check(r.at("data").is_array(), "Missing webhook list");
        });
        // Cleanup callbacks run even if a later operation throws. No list-derived ID
        // is ever modified; only this invocation's creations are registered.
        record("resource_limits", "INFO", {
            {"batch_model", "gpt-4.1-nano"}, {"batch_requests", 1}, {"max_output_tokens", 16},
            {"container_memory", "1g"}, {"ingestion_polls", 3}, {"batch_cleanup_polls", 61},
            {"batch_cleanup_deadline_seconds", 900}, {"webhooks", "list only"}
        });
    });
}
