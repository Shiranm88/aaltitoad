#include "compiler.h"
#include <overload>
#include <variant>

namespace aaltitoad::hawk {
    compiler::compiler(const frontend::scanner& scanner, const frontend::parser& parser, const middleend::semantic_analyzer& analyzer, const middleend::optimizer& optimizer, const backend::generator& generator)
        : scanner{scanner}, parser{parser}, analyzer{analyzer}, optimizer{optimizer}, generator{generator}, symbols{}, diagnostic_factory{}
    {

    }

    void compiler::add_symbols(const expr::symbol_table_t& symbols) {
        this->symbols.put(symbols);
    }

    void compiler::clear_symbols() {
        symbols.clear();
    }

    auto compiler::compile(const std::string& path) -> result<ntta_t> {
        auto stream = scanner.scan(*this, path);
        std::visit(ya::overload(
            [](const std::string& stream){},
            [](const error& err){}
        ), stream);
        auto ast = parser.parse(*this, stream);
        auto dast = analyzer.analyze(*this, ast);
        optimizer.optimize(*this, dast);
        return generator.generate(*this, dast);
    }
}
