#include <extensions/string_extensions.h>
#include "pipe_tocker.h"
#include <string>

pipe_tocker_t::pipe_tocker_t(const std::string& input_pipe_filename, const std::string& output_pipe_filename, const ntta_t& ntta)
 : associated_tta{ntta}, input_pipe{input_pipe_filename}, output_pipe{output_pipe_filename} {
    if(!input_pipe.is_open())
        throw std::logic_error("Could not open "+input_pipe_filename);
    if(!output_pipe.is_open())
        throw std::logic_error("Could not open "+output_pipe_filename);
}

symbol_table_t pipe_tocker_t::tock(const symbol_table_t &environment) const {
    output_pipe << stream_mods::json << associated_tta;;
    std::string line;
    std::getline(input_pipe, line);
    spdlog::debug("pipe_tocker read '{0}'", line);
    return {};
}

extern "C" {
    const char* get_plugin_name() {
        return "pipe_tocker";
    }

    pipe_tocker_t* create_pipe_tocker(const std::string &argument, const ntta_t& ntta) {
        if(!contains(argument, ";"))
            throw std::logic_error("invalid argument format for pipe_tocker, please provide both an input and output pipe seperated by semicolon");
        auto s = split(argument, ";");
        assert(s.size() == 2);
        return new pipe_tocker_t{s[0], s[1], ntta};
    }
}
