#include "parser.hpp"
#include "utils.hpp"

namespace parser {
    void parse_input(const std::string& in, input& out) {
        if (in[0] == '/') {
            out.type = input_type::COMMAND;
            out.command = in.substr(1, in.find(' '));
            out.args = utils::split(in.substr(in.find(' ') + 1), ' ');
        } else {
            out.type = input_type::MESSAGE;
            out.content = in;
        }
    }
}