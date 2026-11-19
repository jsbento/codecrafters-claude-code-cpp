#include <fstream>
#include <sstream>
#include <memory>
#include <array>
#include <iostream>

#include "utils.hpp"

namespace utils {
  std::string trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";

    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
  }

  std::string read_file(std::string path) {
    std::ifstream file(path);
    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();

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

  void write_file(std::string path, std::string content) {
    std::ofstream file(path);
    if (file.is_open()) {
      file << content << std::endl;
      file.close();
    }
  }

  void read_dir(std::filesystem::path path, std::vector<std::filesystem::path>& out) {
    namespace fs = std::filesystem;

    if (!(fs::exists(path) && fs::is_directory(path))) return;

    for (const auto& entry : fs::directory_iterator(path)) {
      fs::path entry_name = entry.path().filename();
      if (entry.is_directory()) {
        fs::path entry_path = path / entry_name;
        read_dir(entry_path, out);
      } else {
        out.emplace_back(path / entry_name);
      }
    }
  }

  std::string exec_cmd(std::string cmd) {
    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        return result;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
  }

  std::vector<std::string> split(const std::string& str, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(str);
    std::string part;

    while(std::getline(ss, part, delim))
      parts.push_back(part);
    return parts;
  }

  std::string join(const std::vector<std::string>& words) {
    std::string out = "";
    for(const std::string& word : words)
      out += word;
    return out;
  }
}