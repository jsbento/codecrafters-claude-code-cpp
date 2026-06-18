#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "agents.hpp"

#define DEBUGMSG(msg) std::cerr << msg << std::endl

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc < 3 || std::string(argv[1]) != "-p") {
        std::cerr << "Expected first argument to be '-p'" << std::endl;
        return 1;
    }

    std::string prompt = argv[2];

    if (prompt.empty()) {
        std::cerr << "Prompt must not be empty" << std::endl;
        return 1;
    }

    const char* api_key_env = std::getenv("OPENROUTER_API_KEY");
    const char* base_url_env = std::getenv("OPENROUTER_BASE_URL");

    std::string api_key = api_key_env ? api_key_env : "";
    std::string base_url = base_url_env ? base_url_env : "https://openrouter.ai/api/v1";

    if (api_key.empty()) {
        std::cerr << "OPENROUTER_API_KEY is not set" << std::endl;
        return 1;
    }

    agent::ai_client client(base_url, api_key);
    client.set_model("anthropic/claude-haiku-4.5");

    std::vector<agent::message> messages;
    messages.emplace_back(agent::message{ "user", prompt });

    while (true) {
        client.do_req(messages);
        agent::message message = messages.back();

        if (!message.tool_calls) {
            std::cout << *message.content << std::endl;
            return 0;
        }

        for (auto tool_call : *message.tool_calls) {
            if (tool_call.function.name == "Read") {
                json args = json::parse(tool_call.function.arguments);

                std::ifstream file(args["file_path"].get<std::string>());
                std::ostringstream ss;
                ss << file.rdbuf();
                file.close();
                std::string content = ss.str();

                messages.emplace_back(agent::message{
                    .role = "tool",
                    .content = content,
                    .tool_call_id = tool_call.id
                });
            }
        }
    }

    return 0;
}
