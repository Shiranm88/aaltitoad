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
#include "plugin_system/plugin_system.h"
#include <parser/hawk/scoped_template_builder/scoped_template_builder.h>

namespace aaltitoad::hawk::graphedit {
    parser::parser() {

    }

    auto parser::parse_files(const std::vector<std::string>& filepaths, const std::vector<std::string> &ignore_list) -> std::unique_ptr<aaltitoad::ntta_t> {
        hawk::scoped_template_builder builder{};
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
                    model_t model{};
                    from_json(json_file, model);
                    builder.add_template(compile_model(model));
                } catch (std::exception& e) {
                    spdlog::error("unable to parse json file {0}: {1}", entry.path().c_str(), e.what());
                    throw;
                }
            }
        }
        spdlog::trace("building the ntta_t");
        return std::unique_ptr<aaltitoad::ntta_t>(builder.build_heap());
    }

    auto parser::should_ignore(const std::filesystem::directory_entry& entry, const std::vector<std::string>& ignore_list) -> bool {
        return std::any_of(ignore_list.begin(), ignore_list.end(),
                           [&entry, this](const std::string& ig){ return should_ignore(entry, ig); });
    }

    auto parser::should_ignore(const std::filesystem::directory_entry& entry, const std::string& ignore_regex) -> bool {
        return std::regex_match(entry.path().c_str(), std::regex{ignore_regex});
    }

    auto parser::parse_model(const Buffer& buffer) -> std::unique_ptr<aaltitoad::ntta_t> {
        hawk::scoped_template_builder builder{};
        builder.add_template(compile_model(to_model(buffer)));
        return std::unique_ptr<aaltitoad::ntta_t>(builder.build_heap());
    }

    auto parser::to_graph(const Graph& g) -> graph_t {
        std::unordered_map<std::string, vertex_t> vertices{};
        vertices.reserve(g.vertices().size());
        for(auto& v : g.vertices()) {
            vertex_t r{};
            from_json(nlohmann::json::parse(v.jsonencoding()), r);
            vertices[v.id()] = r;
        }
        std::unordered_map<std::string, edge_t> edges{};
        edges.reserve(g.edges().size());
        for(auto& e : g.edges()) {
            edge_t r{};
            from_json(nlohmann::json::parse(e.jsonencoding()), r);
            edges[e.id()] = r;
        }
        return graph_t{
            .declarations=g.declarations(),
            .vertices=vertices,
            .edges=edges
        };
    }

    auto parser::to_model(const Buffer& buffer) -> model_t {
        std::unordered_map<std::string, std::string> metadata{};
        metadata.reserve(buffer.metadata().size());
        for(auto& m : buffer.metadata())
            metadata[m.first] = m.second;
        return model_t{
            .metadata=metadata,
            .syntax=to_graph(buffer.graph())
        };
    }

    auto parser::compile_model(const model_t& model) -> tta_template {
        // TODO: BOOKMARK: Compile!
        return {};
    }
}

extern "C" {
    const char* get_plugin_name() {
        return "graphedit_parser";
    }

    const char* get_plugin_version() {
        return "v1.0.0";
    }

    plugin_type get_plugin_type() {
        return plugin_type::parser;
    }

    aaltitoad::plugin::parser* create_parser() {
        return new aaltitoad::hawk::graphedit::parser();
    }
}
