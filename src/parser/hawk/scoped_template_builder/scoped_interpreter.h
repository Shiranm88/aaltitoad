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
#ifndef AALTITOAD_SCOPED_INTERPRETER_H
#define AALTITOAD_SCOPED_INTERPRETER_H
#include <drivers/tree_interpreter.h>
#include <drivers/interpreter.h>

namespace aaltitoad::hawk {
    struct scoped_interpreter : public expr::interpreter {
        scoped_interpreter(std::initializer_list<expr::symbol_table_ref_t> environments);
        ~scoped_interpreter() override = default;
        void add_tree(const std::string& identifier, const expr::syntax_tree_t& tree) override;
        void add_tree(const std::string& access_modifier, const std::string& identifier, const expr::syntax_tree_t& tree) override;
        auto get_symbol(const std::string& identifier) -> expr::syntax_tree_t override;
        expr::symbol_table_t public_result;
        expr::symbol_table_t parameters{};
        std::string identifier_prefix{};
    };

    struct scoped_compiler : public expr::compiler {
        scoped_compiler(expr::symbol_table_t local_symbols, expr::symbol_table_t parameters, std::string local_prefix, std::initializer_list<expr::symbol_table_ref_t> environments);
        ~scoped_compiler() override = default;
        void add_tree(const std::string& identifier, const expr::syntax_tree_t& tree) override;
        auto get_symbol(const std::string &identifier) -> expr::syntax_tree_t override;
        auto get_localized_symbols() -> expr::symbol_table_t;

    private:
        expr::symbol_table_t local_symbols;
        expr::symbol_table_t parameters;
        std::string local_prefix;
    };
}

#endif //AALTITOAD_SCOPED_INTERPRETER_H
