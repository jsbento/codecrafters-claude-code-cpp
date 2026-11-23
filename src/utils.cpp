#include <array>
#include <cstdio>
#include <fstream>
#include <sstream>

#include <sys/wait.h>

#include "utils.hpp"

namespace utils {
  std::string trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";

    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
  }

  std::optional<std::string> read_file(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) return std::nullopt;

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
  }

  std::string read_between(const std::filesystem::path& path, const std::string& start_delim, const std::string& end_delim) {
    std::ifstream file(path);
    std::string line, result;
    bool captured = false;

    while (std::getline(file, line)) {
      if (!captured) {
        size_t start = line.find(start_delim);
        if (start != std::string::npos) {
          captured = true;
          start += start_delim.length();

          size_t end = line.find(end_delim, start);
          if (end != std::string::npos) {
            return line.substr(start, end - start);
          }
          result += line.substr(start) + "\n";
        }
      } else {
        size_t end = line.find(end_delim);
        if (end != std::string::npos) {
          result += line.substr(0, end);
          break;
        }
        result += line + "\n";
      }
    }

    return result;
  }

  bool write_file(const std::filesystem::path& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) return false;

    file << content;
    return static_cast<bool>(file);
  }

  void read_dir(const std::filesystem::path& path, std::vector<std::filesystem::path>& out) {
    namespace fs = std::filesystem;

    if (!fs::is_directory(path)) return;

    for (const auto& entry : fs::directory_iterator(path)) {
      if (entry.is_directory()) {
        read_dir(entry.path(), out);
      } else {
        out.push_back(entry.path());
      }
    }
  }

  cmd_result exec_cmd(const std::string& cmd) {
    // Group the command so stderr is captured for every part of a compound
    // command (`a; b`), not just the last one. The newline ends any trailing
    // `#` comment before the closing brace.
    const std::string full_cmd = "{ " + cmd + "\n} 2>&1";
    FILE* pipe = popen(full_cmd.c_str(), "r");
    if (!pipe) return {-1, "Error: failed to start command"};

    std::array<char, 4096> buffer;
    std::string output;
    size_t n;
    while ((n = fread(buffer.data(), 1, buffer.size(), pipe)) > 0) {
      output.append(buffer.data(), n);
    }

    int status = pclose(pipe);
    int exit_code = (status != -1 && WIFEXITED(status)) ? WEXITSTATUS(status) : -1;
    return {exit_code, std::move(output)};
  }

  std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(str);
    std::string part;

    while (std::getline(ss, part, delim)) {
      if (!part.empty()) parts.push_back(std::move(part));
    }
    return parts;
  }

  std::string join(const std::vector<std::string>& words, const std::string& sep) {
    std::string out;
    for (size_t i = 0; i < words.size(); i++) {
      if (i > 0) out += sep;
      out += words[i];
    }
    return out;
  }
}
