#pragma once

#include <string>
#include <filesystem>
#include <utility>
#include <optional>

namespace skills {
  std::optional<std::pair<std::string, std::string>> read_skill_metadata(std::filesystem::path skill_path);
}