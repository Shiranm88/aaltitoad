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
#include "parser.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "parser/hawk/scoped_template_builder/scoped_template_builder.h"
#include "plugin_system/plugin_system.h"

namespace aaltitoad::hawk::huppaal {
    auto huppaal_scanner::scan(compiler& ctx, const std::vector<std::string>& filepaths, const std::vector<std::string>& ignore_list) const noexcept -> std::expected<scanner::ok, error_t> {
        return std::unexpected(error_t({ctx.diag(not_implemented_yet())}));
        scoped_template_builder builder{};
        for(const auto& filepath : filepaths) {
            for(const auto &entry: std::filesystem::directory_iterator(filepath)) {
                try {
                    if(should_ignore(entry, ignore_list)) {
                        spdlog::trace("ignoring file {0}", entry.path().c_str());
                        continue;
                    }

                    std::ifstream input_filestream(entry.path());
                    auto json_file = nlohmann::json::parse(input_filestream,
                            /* callback */ nullptr,
                            /* allow exceptions */ true,
                            /* ignore_comments */ true);
                    if(json_file.contains("name")) {
                        spdlog::trace("loading file {0}", entry.path().c_str());
                        builder.add_template(json_file.get<tta_template>());
                    } else if(json_file.contains("parts")) {
                        spdlog::trace("loading parts file {0}", entry.path().c_str());
                        builder.add_global_symbols(json_file.at("parts").get<std::vector<part_t>>());
                    } else
                        spdlog::trace("ignoring file {0} (not a valid model file)", entry.path().c_str());
                } catch (std::exception& e) {
                    spdlog::error("unable to parse json file {0}: {1}", entry.path().c_str(), e.what());
                    throw;
                }
            }
        }
    }

    auto huppaal_scanner::should_ignore(const std::filesystem::directory_entry& entry, const std::vector<std::string>& ignore_list) const -> bool {
        return std::any_of(ignore_list.begin(), ignore_list.end(),
                           [&entry, this](const std::string& ig){ return should_ignore(entry, ig); });
    }

    auto huppaal_scanner::should_ignore(const std::filesystem::directory_entry& entry, const std::string& ignore_regex) const -> bool {
        return std::regex_match(entry.path().c_str(), std::regex{ignore_regex});
    }
    
    auto huppaal_parser::parse(compiler& ctx, const scanner::ok& stream) const noexcept -> std::expected<parser::ok, error_t> {
        return std::unexpected(error_t({ctx.diag(not_implemented_yet())}));
    }

    parser::parser() : compiler{} {
        
    }

    auto parser::parse_files(const std::vector<std::string>& files, const std::vector<std::string>& ignore_patterns) -> plugin::parse_result {
        auto compile_result = compiler->compile(files, ignore_patterns);
        if(!compile_result.has_value())
            return std::unexpected(plugin::parse_error{compile_result.error().diagnostics});
        return plugin::parse_ok{compile_result.value().ntta, compile_result.value().diagnostics};
    }

    auto parser::parse_model(const Buffer& buffer) -> plugin::parse_result {
        return std::unexpected(plugin::parse_error({compiler->diag(not_implemented_yet())}));
    }
}

extern "C" {
    const char* get_plugin_name() {
        return "huppaal_parser";
    }

    const char* get_plugin_version() {
        return "v1.1.0";
    }

    plugin_type get_plugin_type() {
        return plugin_type::parser;
    }

    aaltitoad::plugin::parser* create_parser() {
        return new aaltitoad::hawk::huppaal::parser();
    }
}
