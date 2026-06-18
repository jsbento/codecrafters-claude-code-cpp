#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "agents.hpp"
#include "utils.hpp"

#define DEBUGMSG(msg) std::cerr << msg << std::endl

using json = nlohmann::json;

inline void add_message(
    std::vector<agent::message>& messages,
    std::string role,
    std::optional<std::string> content,
    std::optional<std::string> tool_call_id
) {
    messages.emplace_back(agent::message{
        .role = std::move(role),
        .content = std::move(content),
        .tool_call_id = std::move(tool_call_id)
    });
}

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
            auto f = tool_call.function;
            json args = json::parse(f.arguments);
            if (f.name == "Read") {
                std::string file_path = args["file_path"].get<std::string>();
                std::string content = utils::read_file(file_path);

                add_message(messages, "tool", content, tool_call.id);
            }
            if (f.name == "Write") {
                std::string file_path = args["file_path"].get<std::string>();
                std::string content = args["content"].get<std::string>();

                utils::write_file(file_path, content);

                add_message(messages, "tool", message.content, tool_call.id);
            }
            if (f.name == "Bash") {
                std::string command = args["command"].get<std::string>();
                std::string result = utils::exec_cmd(command);

                add_message(messages, "tool", result, tool_call.id);
            }
        }
    }

    return 0;
}
