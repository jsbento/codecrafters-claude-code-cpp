#include <fstream>
#include <sstream>
#include <memory>
#include <array>

#include "utils.hpp"

namespace utils {
  std::string read_file(std::string path) {
    std::ifstream file(path);
    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();

    return ss.str();
  }

  void write_file(std::string path, std::string content) {
    std::ofstream file(path);
    if (file.is_open()) {
      file << content << std::endl;
      file.close();
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
}