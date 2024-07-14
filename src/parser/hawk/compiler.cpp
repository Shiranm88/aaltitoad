#include "compiler.h"
#include <overload>

namespace aaltitoad::hawk {
    compiler::compiler(
            std::unique_ptr<scanner_t>&& scanner,
            std::unique_ptr<parser_t>&& parser,
            std::unique_ptr<semantic_analyzer_t>&& analyzer,
            std::unique_ptr<optimizer_t>&& optimizer,
            std::unique_ptr<generator_t>&& generator)
        : 
            scanner{std::move(scanner)},
            parser{std::move(parser)},
            analyzer{std::move(analyzer)},
            optimizer{std::move(optimizer)},
            generator{std::move(generator)},
            symbols{},
            diagnostic_factory{} {
    }

    void compiler::add_symbols(const expr::symbol_table_t& symbols) {
        this->symbols.put(symbols);
    }

    void compiler::clear_symbols() {
        symbols.clear();
    }

    auto compiler::compile(const std::vector<std::string>& paths, const std::vector<std::string>& ignore_list) -> std::expected<ntta_t, error> {
        auto stream = scanner->scan(*this, paths, ignore_list);
        if(!stream)
            return std::unexpected{stream.error()};
        auto ast = parser->parse(*this, stream.value());
        if(!ast)
            return std::unexpected{ast.error()};
        auto dast_r = analyzer->analyze(*this, ast.value());
        if(!dast_r)
            return std::unexpected{dast_r.error()};
        auto dast = dast_r.value();
        optimizer->optimize(*this, dast);
        return generator->generate(*this, dast);
    }
}
