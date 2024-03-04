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
#include "cli_options.h"
#include "lsp_server.h"
#include <aaltitoadpch.h>
#include <cli/cli_common.h>
#include <csignal>
#include <nlohmann/json.hpp>
#include <plugin_system/plugin_system.h>
#include <timer>

auto load_plugins(std::map<std::string, argument_t>& cli_arguments) -> plugin_map_t;

void siginthandler(int param) {
    spdlog::info("exiting ({})", param);
    exit(param);
}

int main(int argc, char** argv) {
    signal(SIGINT, siginthandler);
    auto options = get_options();
    auto cli_arguments = get_arguments(options, argc, argv);
    if(cli_arguments["verbosity"])
        spdlog::set_level(static_cast<spdlog::level::level_enum>(SPDLOG_LEVEL_OFF - cli_arguments["verbosity"].as_integer()));
    if(cli_arguments["version"])
        return print_version();
    if(cli_arguments["help"])
        return print_help(argv[0], options);
    if(!is_required_provided(cli_arguments, options))
        return print_required_args();

    spdlog::trace("welcome to {} v{}", PROJECT_NAME, PROJECT_VER);

    /// Load plugins
    auto available_plugins = load_plugins(cli_arguments);
    if(cli_arguments["list-plugins"]) {
        std::cout << "available plugins:\n" << available_plugins;
        return 0;
    }

    /// Get the parser
    auto selected_parser = cli_arguments["parser"].as_string_or_default("huppaal_parser");
    if(!available_plugins.contains(selected_parser) || available_plugins.at(selected_parser).type != plugin_type::parser) {
        spdlog::error("no such parser available: '{0}'", selected_parser);
        return 1;
    }

    spdlog::trace("building parser {}", selected_parser);
    auto parser = std::get<parser_ctor_t>(available_plugins.at(selected_parser).function)();
    spdlog::trace("starting language server...");
    aaltitoad::lsp::proto::LanguageServerImpl{
        cli_arguments["port"].as_integer_or_default(5001),
        PROJECT_VER,
        *parser
    }.start();
    spdlog::trace("shutting down {} v{}", PROJECT_NAME, PROJECT_VER);
    return 0;
}

auto load_plugins(std::map<std::string, argument_t>& cli_arguments) -> plugin_map_t {
    auto rpath = std::getenv("AALTITOAD_LIBPATH");
    std::vector<std::string> look_dirs = { ".", "lib", "plugins" };
    if(rpath)
        look_dirs.emplace_back(rpath);
    auto provided_dirs = cli_arguments["plugin-dir"].as_list_or_default({});
    look_dirs.insert(look_dirs.end(), provided_dirs.begin(), provided_dirs.end());
    return aaltitoad::plugins::load(look_dirs);
}

