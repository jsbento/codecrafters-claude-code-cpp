#include "parser.hpp"
#include "utils.hpp"
#include "agents.hpp"

namespace parser {
    void parse_input(agent::ai_client& ai_client, const std::string& in, std::vector<input>& out) {
        auto add_message = [&]() {
            input inp;
            inp.type = input_type::MESSAGE;
            inp.content = in;
            out.push_back(std::move(inp));
        };

        if (in.empty() || in[0] != '/') {
            add_message();
            return;
        }

        std::vector<std::string> tokens = utils::split(in, ' ');

        std::vector<std::string> skills;
        size_t args_start = tokens.size();
        for (size_t i = 0; i < tokens.size(); i++) {
            const std::string& token = tokens[i];
            if (token[0] != '/' || !ai_client.has_skill(token.substr(1))) {
                args_start = i;
                break;
            }
            skills.push_back(token.substr(1));
        }

        // No recognized skill: treat the whole prompt as a plain message.
        if (skills.empty()) {
            add_message();
            return;
        }

        std::vector<std::string> args(tokens.begin() + args_start, tokens.end());

        for (std::string& skill : skills) {
            input inp;
            inp.type = input_type::COMMAND;
            inp.command = std::move(skill);
            inp.args = args;
            out.push_back(std::move(inp));
        }
    }
}
