#pragma once

#include <string>
#include <vector>

namespace parser {
    enum class input_type {
      MESSAGE = 0,
      COMMAND = 1,  
    };

    struct input {
        input_type type;
        std::string content;
        std::string command; // todo: make enum
        std::vector<std::string> args;
    };

    void parse_input(const std::string& in, input& out);
}