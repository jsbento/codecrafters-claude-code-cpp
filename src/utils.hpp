#pragma once

#include <string>

namespace utils {
  std::string read_file(std::string path);
  void write_file(std::string path, std::string content);

  std::string exec_cmd(std::string cmd);
}