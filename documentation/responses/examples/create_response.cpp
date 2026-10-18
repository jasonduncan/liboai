#include "liboai.h"

using namespace liboai;

int main() {
  OpenAI oai;

  if (oai.auth.SetKeyEnv("OPENAI_API_KEY")) {
    try {
      nlohmann::json request;
      request["model"] = "gpt-5.2-pro";
      request["input"] = "Hello from the Responses API.";

      Response response = oai.Responses->create(request);

      const auto output_text = Responses::output_text(response.raw_json);

      if (!output_text.empty()) {
        std::cout << output_text << std::endl;
      }
      else {
        std::cout << response << std::endl;
      }
    }
    catch (std::exception& e) {
      std::cout << e.what() << std::endl;
    }
  }
}
