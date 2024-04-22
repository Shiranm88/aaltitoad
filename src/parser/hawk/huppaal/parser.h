/**
 * aaltitoad - a verification engine for tick tock automata models
   Copyright (C) 2023 Asger Gitz-Johansen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef AALTITOAD_HAWK_PARSER_H
#define AALTITOAD_HAWK_PARSER_H
#include "lsp.pb.h"
#include "plugin_system/parser.h"
#include <nlohmann/json.hpp>

namespace aaltitoad::hawk::huppaal {
    auto create_parser() -> plugin::parser*;

    class parser : public plugin::parser {
    public:
        parser();
        ~parser() override = default;
        auto parse_files(const std::vector<std::string>& filepaths, const std::vector<std::string> &ignore_list) -> plugin::parse_result override;
        auto parse_model(const Buffer& buffer) -> plugin::parse_result override;
    private:
        auto should_ignore(const std::filesystem::directory_entry& entry, const std::vector<std::string>& ignore_list) -> bool;
        auto should_ignore(const std::filesystem::directory_entry& entry, const std::string& ignore_regex) -> bool;
        auto load_part(const nlohmann::json& json_file) -> std::string;
    };
}

#endif //AALTITOAD_HAWK_PARSER_H
