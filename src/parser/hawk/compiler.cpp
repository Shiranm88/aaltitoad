#include "compiler.h"
#include <overload>

namespace aaltitoad::hawk {
    compiler::compiler(
            std::unique_ptr<scanner>&& _scanner,
            std::unique_ptr<parser>&& _parser,
            std::unique_ptr<semantic_analyzer>&& _analyzer,
            std::unique_ptr<optimizer>&& _optimizer,
            std::unique_ptr<generator>&& _generator)
        : 
            _scanner{std::move(_scanner)},
            _parser{std::move(_parser)},
            _analyzer{std::move(_analyzer)},
            _optimizer{std::move(_optimizer)},
            _generator{std::move(_generator)},
            symbols{},
            _diagnostic_factory{} {
    }

    void compiler::add_symbols(const expr::symbol_table_t& symbols) {
        this->symbols.put(symbols);
    }

    void compiler::clear_symbols() {
        symbols.clear();
    }

    auto compiler::compile(const std::vector<std::string>& paths, const std::vector<std::string>& ignore_list) -> std::expected<ntta_t, error> {
        auto stream = _scanner->scan(*this, paths, ignore_list);
        if(!stream)
            return std::unexpected{stream.error()};
        auto ast = _parser->parse(*this, stream.value());
        if(!ast)
            return std::unexpected{ast.error()};
        auto dast_r = _analyzer->analyze(*this, ast.value());
        if(!dast_r)
            return std::unexpected{dast_r.error()};
        auto dast = dast_r.value();
        _optimizer->optimize(*this, dast);
        return _generator->generate(*this, dast);
    }
}
