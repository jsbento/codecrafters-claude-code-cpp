#include <stdexcept>

#include <cpr/cpr.h>

#include "agents.hpp"
#include "utils.hpp"

namespace agent {
  namespace {
    const json tools = json::array({
      {
        {"type", "function"},
        {"function", {
          {"name", "Read"},
          {"description", "Read and return the contents of a file"},
          {"parameters", {
            {"type", "object"},
            {"properties", {
              {"file_path", {
                {"type", "string"},
                {"description", "The path of the file to read"}
              }}
            }},
            {"required", json::array({"file_path"})}
          }}
        }}
      },
      {
        {"type", "function"},
        {"function", {
          {"name", "Write"},
          {"description", "Write content to a file"},
          {"parameters", {
            {"type", "object"},
            {"properties", {
              {"file_path", {
                {"type", "string"},
                {"description", "The path of the file to write to"}
              }},
              {"content", {
                {"type", "string"},
                {"description", "The content to write to the file"}
              }}
            }},
            {"required", json::array({"file_path", "content"})}
          }}
        }}
      },
      {
        {"type", "function"},
        {"function", {
          {"name", "Bash"},
          {"description", "Execute a shell command"},
          {"parameters", {
            {"type", "object"},
            {"properties", {
              {"command", {
                {"type", "string"},
                {"description", "The command to execute"}
              }}
            }},
            {"required", json::array({"command"})}
          }}
        }}
      }
    });
  }

  void to_json(json& j, const tool_call& tc) {
    j = json{
      {"id", tc.id},
      {"type", tc.type},
      {"function", {
        {"name", tc.function.name},
        {"arguments", tc.function.arguments}
      }}
    };
  }

  void from_json(const json& j, tool_call& tc) {
    j.at("id").get_to(tc.id);
    j.at("type").get_to(tc.type);

    const auto& f = j.at("function");
    f.at("name").get_to(tc.function.name);
    f.at("arguments").get_to(tc.function.arguments);
  }

  void to_json(json& j, const message& m) {
    j = {{"role", m.role}};

    if (m.content) j["content"] = *m.content;

    if (m.tool_call_id) j["tool_call_id"] = *m.tool_call_id;

    if (m.tool_calls) j["tool_calls"] = *m.tool_calls;
  }

  void from_json(const json& j, message& m) {
    j.at("role").get_to(m.role);

    m.content.reset();
    m.tool_call_id.reset();
    m.tool_calls.reset();

    if (j.contains("content") && !j["content"].is_null())
      m.content = j["content"].get<std::string>();

    if (j.contains("tool_call_id") && !j["tool_call_id"].is_null())
      m.tool_call_id = j["tool_call_id"].get<std::string>();

    if (j.contains("tool_calls") && !j["tool_calls"].is_null())
      m.tool_calls = j["tool_calls"].get<std::vector<tool_call>>();
  }

  void ai_client::do_req(std::vector<message>& messages) {
    json req_body = {
      {"model", _model},
      {"tools", tools},
      {"messages", messages}
    };

    cpr::Response resp = cpr::Post(
      cpr::Url{_api_host + "/chat/completions"},
      cpr::Header{
        {"Authorization", "Bearer " + _api_key},
        {"Content-Type", "application/json"}
      },
      cpr::Body{req_body.dump()}
    );

    if (resp.status_code != 200) {
      throw std::runtime_error(
        "HTTP error " + std::to_string(resp.status_code) + ":\n" + resp.text);
    }

    json result = json::parse(resp.text);
    if (!result.contains("choices") || result["choices"].empty()) {
      throw std::runtime_error("No choices in response:\n" + resp.text);
    }

    messages.push_back(result["choices"][0]["message"].get<message>());
  }

  void ai_client::add_skill_messages(std::vector<message>& messages) const {
    if (_skills.empty()) return;

    std::string skills_msg = "You have access to the following skills:\n\n";
    for (const auto& [name, skill] : _skills) {
      skills_msg += "- " + name + ": " + skill.description() + "\n";
    }
    messages.push_back(message{"system", std::move(skills_msg)});
  }

  void ai_client::load_skills() {
    _skills.clear();

    std::vector<std::filesystem::path> skill_paths;
    utils::read_dir(SKILLS_BASE_PATH, skill_paths);

    for (auto& skill_path : skill_paths) {
      skills::skill skill(std::move(skill_path));
      // Skip files without valid metadata (e.g. helper files in a skill folder).
      if (!skill.is_valid()) continue;
      std::string name = skill.name();
      _skills.emplace(std::move(name), std::move(skill));
    }
  }
}
