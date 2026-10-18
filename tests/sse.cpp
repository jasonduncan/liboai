#include "core/sse.h"
#include <stdexcept>
#include <vector>

namespace {
void require(bool value) { if (!value) throw std::runtime_error("SSE parser assertion failed"); }
}

void test_sse() {
    // Try every possible split, including the middle of CRLF and multibyte UTF-8.
    const std::string bytes = "\xEF\xBB\xBF:keepalive\r\nid: 4\r\nevent: custom\r\ndata: {\r\ndata: \"text\":\"\xE2\x98\x83\",\"new_field\":true}\r\n\r\n"
        "data: {\"type\":\"response.output_text.delta\",\"delta\":\"ok\"}\n\n"
        "data: [DONE]\r\rdata: unfinished";
    for (std::size_t split = 0; split <= bytes.size(); ++split) {
        std::vector<liboai::SseEvent> events;
        liboai::SseParser parser([&](const liboai::SseEvent& e) { events.push_back(e); return true; });
        require(parser.feed(std::string_view(bytes).substr(0, split)));
        require(parser.feed(std::string_view(bytes).substr(split)));
        parser.finish();
        require(events.size() == 3);
        require(events[0].type == "custom" && events[0].id == "4");
        require(events[0].json.at("text") == "\xE2\x98\x83" && events[0].json.at("new_field") == true);
        require(events[0].data.find('\n') != std::string::npos);
        require(events[1].text_delta() == "ok" && events[1].id == "4");
        require(events[2].data == "[DONE]" && events[2].json.is_null());
    }
    std::vector<liboai::SseEvent> events;
    liboai::SseParser bytes_parser([&](const liboai::SseEvent& e) { events.push_back(e); return true; });
    for (char c : bytes) require(bytes_parser.feed(std::string_view(&c, 1)));
    bytes_parser.finish();
    require(events.size() == 3);

    std::size_t count = 0;
    liboai::SseParser stop([&](const liboai::SseEvent&) { ++count; return false; });
    require(!stop.feed("data: one\n\ndata: two\n\n"));
    require(!stop.feed("data: three\n\n") && count == 1);

    events.clear();
    liboai::SseParser fields([&](const liboai::SseEvent& e) { events.push_back(e); return true; });
    fields.feed("event: unused\n\nid: original\n\ndata:\n\nid\nretry: 500\nunknown: ignored\ndata: x\n\n");
    fields.finish();
    require(events.size() == 2 && events[0].type == "message" && events[0].data.empty());
    require(events[0].id == "original" && events[1].id.empty());

    // A newline terminates a line, not a frame. EOF must not dispatch either form.
    for (const std::string suffix : {"data: lost", "data: lost\n", "event: x\ndata: lost\r\n"}) {
        std::size_t dispatched = 0;
        liboai::SseParser incomplete([&](const liboai::SseEvent&) { ++dispatched; return true; });
        incomplete.feed(suffix);
        incomplete.finish();
        require(dispatched == 0 && !incomplete.feed("\n"));
    }
    events.clear();
    liboai::SseParser malformed([&](const liboai::SseEvent& e) { events.push_back(e); return true; });
    malformed.feed("id: stable\n\n");
    malformed.feed(std::string("id: bad\0id\ndata: broken JSON\n\n", 30));
    malformed.finish();
    require(events.size() == 1 && events[0].id == "stable" && events[0].json.is_null());

    bool threw = false;
    liboai::SseParser exception([](const liboai::SseEvent&) -> bool { throw std::logic_error("callback"); });
    try { exception.feed("data: x\n\n"); } catch (const std::logic_error&) { threw = true; }
    require(threw);
}
