// Historical compatibility example: this hosted endpoint retired on 2024-01-04.
#include "liboai.h"

using namespace liboai;

int main() {
  OpenAI oai;
  if (oai.auth.SetKeyEnv("OPENAI_API_KEY")) {
    try {
      Response response = oai.FineTune->retrieve(
        "ft-AF1WoRqd3aJAHsqc9NY7iL8F"
      );
      std::cout << response << std::endl;
    }
    catch (std::exception& e) {
      std::cout << e.what() << std::endl;
    }
  }
}
