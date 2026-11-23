#pragma once

#include <string>
#include <vector>

namespace agent {
    class ai_client;
}

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

    void parse_input(agent::ai_client& ai_client, const std::string& in, std::vector<input>& out);
}