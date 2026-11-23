#include <iostream>

#include "agents.hpp"
#include "utils.hpp"

namespace agent {
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
      std::cerr << "HTTP error: " << resp.status_code << std::endl << resp.text << std::endl;
      return;
    }

    json result = json::parse(resp.text);
    if (!result.contains("choices") || result["choices"].empty()) {
      std::cerr << "No choices in response" << std::endl;
      return;
    }

    json m_json = result["choices"][0]["message"];
    messages.emplace_back(m_json.get<message>());
  }

  void ai_client::add_skill_messages(std::vector<message>& messages) {
    if (_skills.size() == 0) return;

    std::string skills_msg = "You have access to the following skills:\n\n";
    for (const auto& skill : _skills) {
      if (!skill.is_valid()) continue;
      skills_msg += "- " + skill.name() + ": " + skill.description() + "\n";
    }
    messages.emplace_back(message{"system", skills_msg});
  }

  void ai_client::load_skills() {
    _skills.clear();
    _skill_cache.clear();

    std::vector<std::filesystem::path> skill_paths;
    utils::read_dir(SKILLS_BASE_PATH, skill_paths);

    for (const auto& skill_path : skill_paths) {
      skills::skill skill(skill_path);
      _skills.emplace_back(skills::skill(skill));
      _skill_cache[skill.name()] = true;
    }
  }
}