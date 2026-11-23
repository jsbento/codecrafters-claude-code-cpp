#include "parser.hpp"
#include "utils.hpp"
#include "agents.hpp"

namespace parser {
    void parse_input(agent::ai_client& ai_client, const std::string& in, std::vector<input>& out) {
        std::vector<std::string> tokens = utils::split(in, ' ');
        bool is_skill_call = in[0] == '/';

        if (!is_skill_call) {
            input inp;
            inp.type = input_type::MESSAGE;
            inp.content = in; 
            out.emplace_back(inp);
            return;
        }

        std::vector<std::string> skills;
        size_t args_start = tokens.size();
        for (int i = 0; i < tokens.size(); i++) {
            std::string token = tokens[i];
            if (token[0] != '/' || !ai_client.has_skill(token.substr(1))) {
                args_start = i;
                break;
            } else {
                skills.emplace_back(token.substr(1));
            }
        }

        // No recognized skill: treat the whole prompt as a plain message.
        if (skills.empty()) {
            input inp;
            inp.type = input_type::MESSAGE;
            inp.content = in;
            out.emplace_back(inp);
            return;
        }

        std::vector<std::string>::iterator start = tokens.begin() + args_start;
        std::vector<std::string>::iterator end = tokens.begin() + tokens.size();
        std::vector<std::string> args(start, end);


        for (const std::string& skill : skills) {
            input inp;
            inp.type = input_type::COMMAND;
            inp.command = skill;
            inp.args = args;
            out.emplace_back(inp);
        }
    }
}