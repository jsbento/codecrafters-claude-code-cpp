#pragma once

#include <string>
#include <filesystem>

#include "parser.hpp"

namespace skills {
  class skill {
    public:
      explicit skill(std::filesystem::path path)
        : _path{ std::move(path) }
      {
        load_metadata();
      }

      std::string get_content(const parser::input& input) const;

      bool is_valid() const {
        return !_name.empty();
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
  };
}
