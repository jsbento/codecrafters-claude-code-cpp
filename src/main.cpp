#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "agents.hpp"
#include "utils.hpp"
#include "parser.hpp"

namespace {
  using tool_handler = std::function<std::string(const json&)>;

  // Each handler takes the tool call's parsed arguments and returns the text
  // sent back to the model. Errors are reported in that text so the model can
  // react to them.
  const std::unordered_map<std::string, tool_handler> tool_handlers = {
    {"Read", [](const json& args) -> std::string {
      const auto path = args.at("file_path").get<std::string>();
      auto content = utils::read_file(path);
      if (!content) return "Error: could not read file: " + path;
      return *std::move(content);
    }},
    {"Write", [](const json& args) -> std::string {
      const auto path = args.at("file_path").get<std::string>();
      const auto content = args.at("content").get<std::string>();
      if (!utils::write_file(path, content)) return "Error: could not write file: " + path;
      return "Wrote " + std::to_string(content.size()) + " bytes to " + path;
    }},
    {"Bash", [](const json& args) -> std::string {
      auto result = utils::exec_cmd(args.at("command").get<std::string>());
      if (result.exit_code == 0) return std::move(result.output);
      return "Exit code " + std::to_string(result.exit_code) + "\n" + result.output;
    }},
  };

  std::string run_tool(const agent::tool_call& call) {
    auto it = tool_handlers.find(call.function.name);
    if (it == tool_handlers.end()) return "Error: unknown tool: " + call.function.name;

    try {
      return it->second(json::parse(call.function.arguments));
    } catch (const json::exception& e) {
      return std::string("Error: invalid arguments: ") + e.what();
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3 || std::string(argv[1]) != "-p") {
    std::cerr << "Expected first argument to be '-p'" << std::endl;
    return 1;
  }

  const std::string prompt = utils::trim(argv[2]);
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

  agent::ai_client client(std::move(base_url), std::move(api_key));
  client.set_model("anthropic/claude-haiku-4.5");

  std::vector<parser::input> inputs;
  parser::parse_input(client, prompt, inputs);

  std::vector<agent::message> messages;
  client.add_skill_messages(messages);

  for (const parser::input& input : inputs) {
    if (input.type == parser::input_type::COMMAND) {
      if (const skills::skill* skill = client.find_skill(input.command)) {
        messages.push_back({.role = "user", .content = skill->get_content(input)});
      }
    } else {
      messages.push_back({.role = "user", .content = input.content});
    }
  }

  try {
    while (true) {
      client.do_req(messages);

      // Copied because pushing tool results below can reallocate `messages`.
      const auto tool_calls = messages.back().tool_calls;
      if (!tool_calls) {
        std::cout << messages.back().content.value_or("") << std::endl;
        return 0;
      }

      for (const agent::tool_call& call : *tool_calls) {
        messages.push_back({
          .role = "tool",
          .content = run_tool(call),
          .tool_call_id = call.id,
        });
      }
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
