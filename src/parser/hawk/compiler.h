#ifndef AALTITOAD_HAWK_COMPILER_H
#define AALTITOAD_HAWK_COMPILER_H
#include "ntta/tta.h"
#include "parser/hawk/diagnostics.h"
#include "parser/hawk/model.h"
#include "symbol_table.h"
#include <expected>
#include <string>
#include <vector>

namespace aaltitoad::hawk {
    class compiler;

    struct error {
        std::vector<diagnostic> diagnostics;
    };

    struct scanner {
        struct result {
            std::vector<scanning::template_t> templates;
            std::vector<diagnostic> diagnostics;
        };

        scanner() = default;
        virtual ~scanner() = default;
        virtual auto scan(compiler& ctx,
                const std::vector<std::string>& filepaths,
                const std::vector<std::string>& ignore_list) const noexcept -> std::expected<result, error> = 0;
    };

    struct parser { 
        struct result {
            std::vector<parsing::template_t> templates;
            std::vector<diagnostic> diagnostics;
        };

        parser() = default;
        virtual ~parser() = default;
        virtual auto parse(compiler& ctx, const scanner::result& scanner_result) const noexcept -> std::expected<result, error> = 0;
    };

    struct semantic_analyzer {
        semantic_analyzer() = default;
        virtual ~semantic_analyzer() = default;
        virtual auto analyze(compiler& ctx, const parser::result& ast) const noexcept -> std::expected<parser::result, error> = 0;
    };

    struct optimizer {
        optimizer() = default;
        virtual ~optimizer() = default;
        virtual void optimize(compiler& ctx, parser::result& ast) const = 0;
    };

    struct generator {
        generator() = default;
        virtual ~generator() = default;
        virtual auto generate(compiler& ctx, const parser::result& ast) const noexcept -> std::expected<ntta_t, error> = 0;
    };

    class compiler {
    public:
        compiler(
                std::unique_ptr<scanner>&& scanner,
                std::unique_ptr<parser>&& parser,
                std::unique_ptr<semantic_analyzer>&& analyzer,
                std::unique_ptr<optimizer>&& optimizer,
                std::unique_ptr<generator>&& generator);
        void add_symbols(const expr::symbol_table_t& symbols);
        void clear_symbols();
        auto get_diagnostic_factory() -> diagnostic_factory&;
        auto compile(const std::vector<std::string>& paths, const std::vector<std::string>& ignore_list) -> std::expected<ntta_t, error>;

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
