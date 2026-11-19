#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace utils {
  std::string trim(const std::string& str);

  std::string read_file(std::string path);
  std::string read_between(const std::filesystem::path& path, const std::string& start_delim, const std::string& end_delim);
  void write_file(std::string path, std::string content);

  void read_dir(std::filesystem::path path, std::vector<std::filesystem::path>& out);

  std::string exec_cmd(std::string cmd);

  std::vector<std::string> split(const std::string& str, char delim);
  std::string join(const std::vector<std::string>& words);
}