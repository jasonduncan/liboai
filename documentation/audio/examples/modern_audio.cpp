#include "liboai.h"
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) { std::cerr << "Usage: modern_audio MODEL_ID\n"; return 1; }
    if (!liboai::Authorization::Authorizer().SetKeyEnv("OPENAI_API_KEY")) {
        std::cerr << "Set OPENAI_API_KEY before explicitly running this example.\n"; return 1;
    }
    try {
    liboai::Audio audio("https://api.openai.com/v1");
    auto result = audio.speech_json({{"model", argv[1]}, {"voice", "alloy"}, {"input", "Hello."}, {"response_format", "wav"}});
    std::ofstream output("modern-speech.wav", std::ios::binary);
    output.write(result.response.content.data(), static_cast<std::streamsize>(result.response.content.size()));
    if (!output) throw std::runtime_error("Could not write modern-speech.wav");
    // SSE-capable models such as gpt-4o-mini-tts can use speech_stream.
    // A whisper-1 timestamp request uses multipart fields:
    // {"model", "whisper-1"}, {"file", File{"audio.wav"}},
    // {"response_format", "verbose_json"}, {"timestamp_granularities[]", "word"}.
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
