#pragma once

#include <string>
#include <optional>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "skills.hpp"

namespace agent {
  static const std::string SKILLS_BASE_PATH = ".claude/skills";

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
    std::optional<std::string> content;
    std::optional<std::string> tool_call_id;
    std::optional<std::vector<tool_call>> tool_calls;
  };

  void to_json(json& j, const tool_call& tc);
  void from_json(const json& j, tool_call& tc);

  void to_json(json& j, const message& m);
  void from_json(const json& j, message& m);

  static json tools = json::array({
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

  class ai_client {
    public:
      ai_client(std::string host, std::string key)
        : _api_host{ host }, _api_key{ key }
      {
        load_skills();
      }
      
      void set_model(std::string model) { _model = model; }
      const std::string get_model() const { return _model; }

      void do_req(std::vector<message>& messages);

      bool has_skill(const std::string& name) const {
        return _skill_cache.count(name) > 0;
      }

      void add_skill_messages(std::vector<message>& messages);
    private:
      void load_skills();

      std::string _api_host;
      std::string _api_key;
      std::string _model;
      std::vector<skills::skill> _skills;
      std::unordered_map<std::string, bool> _skill_cache;
  };
}
