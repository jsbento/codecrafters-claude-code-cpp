#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "skills.hpp"

namespace agent {
  inline const std::string SKILLS_BASE_PATH = ".claude/skills";

  struct tool_call {
    std::string id;
    std::string type;
    struct {
      std::string name;
      std::string arguments;
    } function;
  };

  struct message {
    std::string role;
    std::optional<std::string> content{};
    std::optional<std::string> tool_call_id{};
    std::optional<std::vector<tool_call>> tool_calls{};
  };

  void to_json(json& j, const tool_call& tc);
  void from_json(const json& j, tool_call& tc);

  void to_json(json& j, const message& m);
  void from_json(const json& j, message& m);

  class ai_client {
    public:
      ai_client(std::string host, std::string key)
        : _api_host{ std::move(host) }, _api_key{ std::move(key) }
      {
        load_skills();
      }

      void set_model(std::string model) { _model = std::move(model); }
      const std::string& get_model() const { return _model; }

      // Sends messages to the API and appends the assistant's reply.
      // Throws std::runtime_error on an HTTP error or an empty response.
      void do_req(std::vector<message>& messages);

      const skills::skill* find_skill(const std::string& name) const {
        auto it = _skills.find(name);
        return it == _skills.end() ? nullptr : &it->second;
      }

      bool has_skill(const std::string& name) const {
        return _skills.contains(name);
      }

      void add_skill_messages(std::vector<message>& messages) const;
    private:
      void load_skills();

      std::string _api_host;
      std::string _api_key;
      std::string _model;
      std::map<std::string, skills::skill> _skills; // keyed by skill name
  };
}
