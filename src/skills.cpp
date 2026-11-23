#include <cctype>
#include <filesystem>

#include "skills.hpp"
#include "utils.hpp"

namespace skills {
  namespace {
    // Strips the leading "---\n...\n---" metadata block, if present.
    std::string strip_metadata(const std::string& raw, const std::string& delim) {
      size_t open = raw.find(delim);
      if (open == std::string::npos) return raw;

      size_t close = raw.find(delim, open + delim.length());
      if (close == std::string::npos) return raw;

      return raw.substr(close + delim.length());
    }

    // Replaces every $ARGUMENTS with all args joined by spaces, and every $N
    // (0-indexed, any number of digits) with args[N]. $N beyond the arg count
    // is left untouched.
    std::string substitute_args(const std::string& content, const std::vector<std::string>& args) {
      static const std::string all_args_token = "ARGUMENTS";
      const std::string all_args = utils::join(args);

      std::string out;
      out.reserve(content.size());

      for (size_t i = 0; i < content.size(); i++) {
        if (content[i] != '$') {
          out += content[i];
          continue;
        }

        if (content.compare(i + 1, all_args_token.length(), all_args_token) == 0) {
          out += all_args;
          i += all_args_token.length();
          continue;
        }

        size_t digits_end = i + 1;
        while (digits_end < content.size() && std::isdigit(static_cast<unsigned char>(content[digits_end])))
          digits_end++;

        size_t num_digits = digits_end - i - 1;
        if (num_digits > 0 && num_digits <= 9) { // cap keeps stoul from overflowing
          size_t idx = std::stoul(content.substr(i + 1, digits_end - i - 1));
          if (idx < args.size()) {
            out += args[idx];
            i = digits_end - 1;
            continue;
          }
        }

        out += content[i];
      }

      return out;
    }
  }

  std::string skill::get_content(const parser::input& input) const {
    std::string raw_content = utils::read_file(_path).value_or("");
    std::string content = utils::trim(strip_metadata(raw_content, metadata_delim));

    if (input.args.empty()) return content;
    return substitute_args(content, input.args);
  }

  void skill::load_metadata() {
    if (!std::filesystem::exists(_path)) return;

    std::string metadata = utils::trim(utils::read_between(_path, metadata_delim, metadata_delim));
    size_t separator = metadata.find('\n');
    if (separator == std::string::npos) return;

    std::string name = metadata.substr(0, separator);
    std::string description = metadata.substr(separator + 1);

    _name = name.substr(name.find(' ') + 1);
    _description = description.substr(description.find(' ') + 1);
  }
}
