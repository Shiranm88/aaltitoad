#ifndef AALTITOAD_HAWK_COMPILER_H
#define AALTITOAD_HAWK_COMPILER_H
#include "ntta/tta.h"
#include "parser/hawk/diagnostics.h"
#include "symbol_table.h"
#include <expected>
#include <string>
#include <vector>

namespace aaltitoad::hawk {
    class compiler;

    struct error {
        diagnostic diagnostic;
    };

    struct scanner_t {
        scanner_t() = default;
        virtual ~scanner_t() = default;
        virtual auto scan(compiler& ctx,
                const std::vector<std::string>& filepaths,
                const std::vector<std::string>& ignore_list) const noexcept -> std::expected<std::string, error> = 0;
    };

    struct parser_t { 
        parser_t() = default;
        virtual ~parser_t() = default;
        virtual auto parse(compiler& ctx, const std::string& stream) const noexcept -> std::expected<int, error> = 0;  // TODO: Should be an AST output
    };

    struct semantic_analyzer_t {
        semantic_analyzer_t() = default;
        virtual ~semantic_analyzer_t() = default;
        virtual auto analyze(compiler& ctx, const int& ast) const noexcept -> std::expected<int, error> = 0; // TODO: Should be an ast output
    };

    struct optimizer_t {
        optimizer_t() = default;
        virtual ~optimizer_t() = default;
        virtual void optimize(compiler& ctx, int& ast) const = 0;
    };

    struct generator_t {
        generator_t() = default;
        virtual ~generator_t() = default;
        virtual auto generate(compiler& ctx, const int& ast) const noexcept -> std::expected<ntta_t, error> = 0;
    };

    class compiler {
    public:
        compiler(
                std::unique_ptr<scanner_t>&& scanner,
                std::unique_ptr<parser_t>&& parser,
                std::unique_ptr<semantic_analyzer_t>&& analyzer,
                std::unique_ptr<optimizer_t>&& optimizer,
                std::unique_ptr<generator_t>&& generator);
        void add_symbols(const expr::symbol_table_t& symbols);
        void clear_symbols();
        auto get_diagnostic_factory() -> diagnostic_factory&;
        auto compile(const std::vector<std::string>& paths, const std::vector<std::string>& ignore_list) -> std::expected<ntta_t, error>;

    private:
        std::unique_ptr<scanner_t> scanner;
        std::unique_ptr<parser_t> parser;
        std::unique_ptr<semantic_analyzer_t> analyzer;
        std::unique_ptr<optimizer_t> optimizer;
        std::unique_ptr<generator_t> generator;
        expr::symbol_table_t symbols;
        diagnostic_factory diagnostic_factory;
    };
}

#endif // AALTITOAD_HAWK_COMPILER_H
