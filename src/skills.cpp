#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include "skills.hpp"
#include "utils.hpp"

namespace skills {
  std::string skill::get_content(const parser::input& input) {
    std::string raw_content = utils::read_file(_path);
    size_t content_start = raw_content.rfind(metadata_delim);
    std::string content = utils::trim(raw_content.substr(content_start + metadata_delim.length()));
    if (input.args.size() > 0) {
      if (content.find("$ARGUMENTS") != std::string::npos)
        content.replace(content.find("$ARGUMENTS"), 10, utils::join(input.args));
      for (int i = 0; i < input.args.size(); i++) {
        std::string arg = "$" + std::to_string(i);
        if (content.find(arg) == std::string::npos) return content;
        content.replace(content.find(arg), arg.length(), input.args[i]);
      }
    }
    return content;
  }

  void skill::load_metadata() {
    if (!std::filesystem::exists(_path)) return;

    std::string metadata =  utils::trim(utils::read_between(_path, metadata_delim, metadata_delim));
    size_t separator = metadata.find('\n');
    if (separator == std::string::npos) return;

    std::string name = metadata.substr(0, separator);
    std::string description = metadata.substr(separator + 1);

    _name = name.substr(name.find(' ') + 1);
    _description = description.substr(description.find(' ') + 1);

    std::pair<std::string, std::string> p_metadata = {
      _name, _description
    };
    _metadata = &p_metadata;
  }
}