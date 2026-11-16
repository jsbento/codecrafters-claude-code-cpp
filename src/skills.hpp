#pragma once

#include <string>
#include <filesystem>
#include <utility>
#include <optional>

namespace skills {
  class skill {
    public:
      skill(std::filesystem::path path)
        : _path{ path }
      {
        load_metadata();
      }

      std::string get_content();

      const bool is_valid() const {
        return _metadata != nullptr && _name != "";
      }

      const std::string& name() const {
        return _name;
      }

      const std::string& description() const {
        return _description;
      }
    private:
      inline static const std::string metadata_delim = "---";

      void load_metadata();

      std::filesystem::path _path;
      std::string _name;
      std::string _description;
      std::pair<std::string, std::string>* _metadata;
  };
}