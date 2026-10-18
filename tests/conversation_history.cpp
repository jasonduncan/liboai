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

nlohmann::json messages(const liboai::Conversation& conversation) {
    return conversation.GetJSON()["messages"];
}

void seed(liboai::Conversation& conversation, bool system) {
    nlohmann::json history = nlohmann::json::array();
    if (system) history.push_back({{"role", "system"}, {"content", "instructions"}});
    for (int i = 0; i < 6; ++i) {
        history.push_back({{"role", "user"}, {"content", std::to_string(i)}});
    }
    require(conversation.Import(nlohmann::json{{"messages", history}}.dump()), "import failed");
}
}

int main() {
    for (bool system : {false, true}) {
        for (size_t limit : {0, 1, 3, 20}) {
            for (bool named : {false, true}) {
                liboai::Conversation conversation;
                seed(conversation, system);
                conversation.SetMaxHistorySize(limit);
                require(messages(conversation).size() == (system ? 7 : 6), "setter must defer trimming");
                for (int i = 0; i < 3; ++i) {
                    require(named ? conversation.AddUserData("new", "alice") : conversation.AddUserData("new"),
                            "add failed");
                    auto history = messages(conversation);
                    const size_t cap = system && limit == 0 ? 1 : limit;
                    require(history.size() <= cap, "history exceeds limit after adding");
                    if (system) require(history.front()["content"] == "instructions", "system lost");
                    if (limit > (system ? 1u : 0u)) {
                        require(history.back()["content"] == "new", "newest message lost");
                        if (named) require(history.back()["name"] == "alice", "name lost");
                    }
                }
            }
        }
    }

    liboai::Conversation ordered;
    seed(ordered, true);
    ordered.SetMaxHistorySize(3);
    require(ordered.AddUserData("new"), "add failed");
    require(messages(ordered) == nlohmann::json::array({
        {{"role", "system"}, {"content", "instructions"}},
        {{"role", "user"}, {"content", "5"}},
        {{"role", "user"}, {"content", "new"}}
    }), "wrong messages retained after importing oversized history");

    for (size_t limit : {0, 1, 3}) {
        for (bool system : {false, true}) {
            for (bool null_content : {false, true}) {
                liboai::Conversation conversation;
                seed(conversation, system);
                conversation.SetMaxHistorySize(limit);
                nlohmann::json response = {{"choices", nlohmann::json::array({
                    {{"message", {{"role", "assistant"},
                        {"content", null_content ? nlohmann::json(nullptr) : nlohmann::json("reply")}}}}
                })}};
                require(conversation.Update(response.dump()), "update failed");
                auto history = messages(conversation);
                require(history.size() == (system && limit == 0 ? 1 : limit), "update exceeds limit");
                if (system) require(history.front()["role"] == "system", "update lost system");
                if (limit > (system ? 1u : 0u)) require(history.back()["role"] == "assistant", "update lost reply");
            }
        }
    }

    liboai::Conversation empty;
    empty.SetMaxHistorySize(0);
    require(empty.AddUserData("discarded"), "zero-limit add failed");
    require(messages(empty).empty(), "zero limit retained a user message");
    require(empty.SetSystemData("system"), "system setup failed");
    require(empty.AddUserData("discarded"), "system-only add failed");
    require(messages(empty).size() == 1 && messages(empty).front()["role"] == "system", "system-only trim failed");

    liboai::Conversation unlimited;
    for (int i = 0; i < 50; ++i) require(unlimited.AddUserData("retained"), "default add failed");
    require(messages(unlimited).size() == 50, "default history should be unlimited");
    require(!unlimited.AddUserData(""), "empty input accepted");
    require(messages(unlimited).size() == 50, "empty input changed history");
    std::cout << "Conversation history regression tests passed\n";
}
