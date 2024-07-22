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
    parser::parser() {

    }

    auto parser::parse_files(const std::vector<std::string>& filepaths, const std::vector<std::string> &ignore_list) -> plugin::parse_result {
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
        spdlog::trace("building the ntta_t");
        return builder.build();
    }

    auto parser::parse_model(const Buffer& buffer) -> plugin::parse_result {
        spdlog::error("parsing model buffers not supported yet");
        throw std::logic_error{"parsing model buffers not supported yet"};
    }

    auto parser::should_ignore(const std::filesystem::directory_entry& entry, const std::vector<std::string>& ignore_list) -> bool {
        return std::any_of(ignore_list.begin(), ignore_list.end(),
                           [&entry, this](const std::string& ig){ return should_ignore(entry, ig); });
    }

    auto parser::should_ignore(const std::filesystem::directory_entry& entry, const std::string& ignore_regex) -> bool {
        return std::regex_match(entry.path().c_str(), std::regex{ignore_regex});
    }

    auto json_parser_t::scan(compiler& ctx, const std::vector<std::string>& filepaths, const std::vector<std::string>& ignore_list) const noexcept -> std::expected<std::string, error> {
        // BOOKMARK: What should the scanner actually return?
        // Traditionally, a scanner would give a linear token stream. If we were to take the metaphor directly, we would
        // return something like a list of "tokens" i.e. different kindas of nodes and edges, i.e. an adjacency list.
        // But isn't that just excactly what the AST would be equivalent to (the parser's output)?
        // Say that we do that. The scanner outputs a list of unchecked tokens and reports errors if there are
        // unrecognized vertex types, missing keys, etc.
        // Then what would the parser do? - Checking graph consistency and semantics is the job of the semantic analyzer
        // not the parser, so we can't do that...
        // Traditionally, the parser would take the token stream and do LALR walking and generate an AST. But our AST
        // (no decorations) _is_ the adjacency list. - maybe the parser step should ensure compileability of the expr
        // strings? No. That would require a scoped parser, and that is a semantic analyzer thing.
        // WHAT DOES THE PARSER DO?
        // 
        // Okay... Maybe the scanner doesnt output the adjacency list then. The question is then: What thing do we need
        // to generate an adjacency list? Just a json structure? I mean... The only thing that seems icky about that is
        // that we would make the compiler dependent on nlohmann, but is that a problem? It would ensure a valid json
        // syntax. Compared to a traditional scanner, would that fit?
        //
        // Let's see. A C scanner would gladly accept this nonsense: hello int "world" void = 2 = = ==
        // The C parser would say that the token stream is fucking stupid, but that's fine. It's valid scanner input.
        // Comparatively, if our scanner gets: `{ "hello": 321 }` - that would also be valid scanner input. Again, the
        // parser will flip it's shit, because that is clearly not a hawk graph.
        // So. The scanner expects valid tokens, and the parser expects that the tokens are sorted in such a manner that
        // it makes sense as a hawk graph.
        //
        // We could also have a custom token stream format instead, that represents an adjacency list, but then again
        // why would we do that? To streamline the process, make it more robust and make it easier to extend? - blah!
        // i.e. a "DSL" representing an annotated graph - what's it called... a Labelled Transition System!
        //
        // If we say that the scanner should output valid "tokens" without any context embedded, then an adjacency list
        // with "tokens", i.e. nodes, edges and freefloating expressions (symbol declarations) would be the most direct
        // approach.
        // This approach would make the `{ "hello": 321 }` example not valid scanner input, since it does not describe
        // any valid tokens (nodes, edges, declarations).
        // A minimal, scanner friendly, but not parser friendly input would then be something like (fluctuations in
        // syntaxes can be addressed by other scanners):
        // ```json
        // {
        //   "declarations": "...",
        //   "edges": [
        //      {...}
        //   ],
        //   "vertices": [
        //      {...}
        //   ]
        // }
        // ```
        // In any case, the output of the scanner is a struct with nodes (vertices), edges and symbol declarations.
        // Without regard of context consistency.
        //
        // Now the question stands: Who should complain about missing json keys? = THE SCANNER, because a missing key in
        // a vertex is an invalid "token". Not an invalid grammar thing. The parser should complain if the graph is not
        // a properly defined graph (i.e. the _contents_ of the keys in the vertices are consistent), that could be a 
        // badly formed (syntactically incorrect) update expression, a non-existent edge source, or even an unrecognized
        // vertex type (yes, that is the parser's responsibility to check. Not the scanner. Think; scanner checks that
        // the keys exist, parser checks that the values at least make some sense) - then we can do semantic analysis,
        // and all that cool optimization magic code!
        //
        // Now. A HUPPAAL HAWK file's edges will be annotated with guards and updates directly on the edge object, which
        // is fine and all, but a Graphedit HAWK file's edges are not structured like that. Whom'stve's responsibility
        // is it to unify these two different ways of modelling? The scanner? Perhaps...
        // Scanner: 
        //  - The GrapheditScanner should then translate the edge L--g--u-->L' into [L,L'], [<L,g,u,L'>]. The edge would
        //      be "recreated", so we should keep track of the original edge identifiers for error reporting.
        //  - The HUPPAALScanner should probably not do much.
        //
        // Parser:
        //  - The HawkParser should manage two different types of edge management. Yea no. This violates SOLID. The
        //    platform specific shit should be shoved as far in to the scanner as it can.
        for(const auto& filepath : filepaths) {
            for(const auto& entry: std::filesystem::directory_iterator(filepath)) {
                try {
                    if(should_ignore(entry, ignore_list)) {
                        spdlog::trace("ignoring file {}", entry.path().c_str());
                        continue;
                    }

                    std::ifstream input_filestream(entry.path());
                    auto json_file = nlohmann::json::parse(input_filestream,
                            /* callback */ nullptr,
                            /* allow exceptions */ true,
                            /* ignore_comments */ true);
                    if(json_file.contains("name")) {
                        spdlog::trace("loading file {0}", entry.path().c_str());
                    } else if(json_file.contains("parts")) {
                        spdlog::trace("loading parts file {0}", entry.path().c_str());
                    } else
                        spdlog::trace("ignoring file {0} (not a valid model file)", entry.path().c_str());
                } catch (std::exception& e) {
                    spdlog::error("unable to parse json file {0}: {1}", entry.path().c_str(), e.what());
                    return std::unexpected(error()); // TODO: Create a real diagnostic
                }
            }
        }
    }

    auto json_parser_t::parse(compiler& ctx, const std::string& stream) const noexcept -> std::expected<int, error> {

    }

    auto json_parser_t::should_ignore(const std::filesystem::directory_entry& entry, const std::vector<std::string>& ignore_list) const -> bool {
        return std::any_of(ignore_list.begin(), ignore_list.end(),
                           [&entry, this](const std::string& ig){ return should_ignore(entry, ig); });
    }

    auto json_parser_t::should_ignore(const std::filesystem::directory_entry& entry, const std::string& ignore_regex) const -> bool {
        return std::regex_match(entry.path().c_str(), std::regex{ignore_regex});
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
