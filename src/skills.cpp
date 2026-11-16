#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include "skills.hpp"
#include "utils.hpp"

namespace skills {
  std::optional<std::pair<std::string, std::string>> read_skill_metadata(std::filesystem::path skill_path) {
    if (!std::filesystem::exists(skill_path)) return std::nullopt;

    const std::string delim = "---";
    std::string metadata =  utils::trim(utils::read_between(skill_path, delim, delim));
    size_t separator = metadata.find('\n');
    if (separator == std::string::npos) return std::nullopt;

    std::string name = metadata.substr(0, separator);
    std::string description = metadata.substr(separator + 1);

    return {
      {
        name.substr(name.find(' ') + 1),
        description.substr(description.find(' ') + 1)
      }
    };
  }
}