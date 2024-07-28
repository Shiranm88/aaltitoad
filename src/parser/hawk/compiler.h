#ifndef AALTITOAD_HAWK_COMPILER_H
#define AALTITOAD_HAWK_COMPILER_H
#include "ntta/tta.h"
#include "parser/diagnostics.h"
#include "parser/hawk/model.h"
#include "symbol_table.h"
#include <expected>
#include <string>
#include <vector>

namespace aaltitoad::hawk {
    class compiler;

    auto not_implemented_yet() -> diagnostic;

    struct error_t {
        std::vector<Diagnostic> diagnostics;

        error_t(); // TODO: maybe these aren't needed
        error_t(const std::vector<Diagnostic>& diagnostics);
        // error_t(const Diagnostic& diagnostic); // TODO: <--
        // error_t(const diagnostic& diagnostic, compiler& ctx); // TODO: <--
    };

    struct scanner {
        struct ok {
            std::vector<scanning::template_t> templates;
            std::vector<Diagnostic> diagnostics;
        };

        scanner() = default;
        virtual ~scanner() = default;
        virtual auto scan(compiler& ctx,
                const std::vector<std::string>& filepaths,
                const std::vector<std::string>& ignore_list) const noexcept -> std::expected<ok, error_t> = 0;
    };

    struct parser { 
        struct ok {
            std::vector<parsing::template_t> templates;
            std::vector<Diagnostic> diagnostics;
        };

        parser() = default;
        virtual ~parser() = default;
        virtual auto parse(compiler& ctx, const scanner::ok& scanner_result) const noexcept -> std::expected<ok, error_t> = 0;
    };

    struct semantic_analyzer {
        semantic_analyzer() = default;
        virtual ~semantic_analyzer() = default;
        virtual auto analyze(compiler& ctx, const parser::ok& ast) const noexcept -> std::expected<parser::ok, error_t> = 0;
    };

    struct optimizer {
        optimizer() = default;
        virtual ~optimizer() = default;
        virtual void optimize(compiler& ctx, parser::ok& ast) const = 0;
    };

    struct generator {
        generator() = default;
        virtual ~generator() = default;
        virtual auto generate(compiler& ctx, const parser::ok& ast) const noexcept -> std::expected<ntta_t, error_t> = 0;
    };

    class compiler {
    public:
        struct ok {
            ntta_t ntta;
            std::vector<Diagnostic> diagnostics;
        };

        compiler(
                std::unique_ptr<scanner>&& scanner,
                std::unique_ptr<parser>&& parser,
                std::unique_ptr<semantic_analyzer>&& analyzer,
                std::unique_ptr<optimizer>&& optimizer,
                std::unique_ptr<generator>&& generator);
        void add_symbols(const expr::symbol_table_t& symbols);
        void clear_symbols();
        auto get_diagnostic_factory() -> diagnostic_factory&;
        auto diag(const diagnostic& d) -> Diagnostic;
        auto compile(const std::vector<std::string>& paths, const std::vector<std::string>& ignore_list) -> std::expected<ok, error_t>;

    private:
        std::unique_ptr<scanner> _scanner;
        std::unique_ptr<parser> _parser;
        std::unique_ptr<semantic_analyzer> _analyzer;
        std::unique_ptr<optimizer> _optimizer;
        std::unique_ptr<generator> _generator;
        expr::symbol_table_t symbols;
        diagnostic_factory _diagnostic_factory;
    };
}

#endif // AALTITOAD_HAWK_COMPILER_H
