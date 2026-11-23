#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace utils {
  struct cmd_result {
    int exit_code;
    std::string output; // stdout and stderr combined
  };

  std::string trim(const std::string& str);

  // Returns nullopt if the file cannot be opened.
  std::optional<std::string> read_file(const std::filesystem::path& path);
  std::string read_between(const std::filesystem::path& path, const std::string& start_delim, const std::string& end_delim);
  // Writes content exactly as given. Returns false if the file cannot be opened.
  bool write_file(const std::filesystem::path& path, const std::string& content);

  void read_dir(const std::filesystem::path& path, std::vector<std::filesystem::path>& out);

  cmd_result exec_cmd(const std::string& cmd);

  // Splits on delim, dropping empty parts (e.g. from repeated delimiters).
  std::vector<std::string> split(const std::string& str, char delim);
  std::string join(const std::vector<std::string>& words, const std::string& sep = " ");
}
