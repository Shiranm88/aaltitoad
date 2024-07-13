#ifndef AALTITOAD_HAWK_COMPILER_H
#define AALTITOAD_HAWK_COMPILER_H
#include "ntta/tta.h"
#include "parser/hawk/diagnostics.h"
#include "symbol_table.h"
#include <string>

namespace aaltitoad::hawk {
    class compiler;

    struct error {
        diagnostic diagnostic;
    };
    template<typename ok>
    using result = std::variant<error, ok>; // TODO: Consider making this a simple yalibs thing

    namespace frontend {
        struct scanner {
            virtual auto scan(compiler& ctx, const std::string& path) const noexcept -> result<std::string> = 0;  // TODO: probably shouldnt be a string output...
        };

        struct parser { 
            virtual auto parse(compiler& ctx, const std::string& stream) const noexcept -> result<int> = 0;  // TODO: Should be an AST output
        };
    }

    namespace middleend {
        struct semantic_analyzer {
            virtual auto analyze(compiler& ctx, const int& ast) const noexcept -> result<int> = 0; // TODO: Should be an ast output
        };

        struct optimizer {
            // Note that optimizers are not allowed to return errors, but can throw exceptions.
            virtual void optimize(compiler& ctx, int& ast) const = 0;
        };
    }

    namespace backend {
        struct generator {
            virtual auto generate(compiler& ctx, const int& ast) const noexcept -> result<ntta_t> = 0;
        };
    }

    class compiler {
    public:
        compiler(const frontend::scanner& scanner,
                const frontend::parser& parser,
                const middleend::semantic_analyzer& analyzer,
                const middleend::optimizer& optimizer,
                const backend::generator& generator);
        void add_symbols(const expr::symbol_table_t& symbols);
        void clear_symbols();
        auto get_diagnostic_factory() -> diagnostic_factory&;
        auto compile(const std::string& path) -> result<ntta_t>;

    private:
        const frontend::scanner& scanner;
        const frontend::parser& parser;
        const middleend::semantic_analyzer& analyzer;
        const middleend::optimizer& optimizer;
        const backend::generator& generator;
        expr::symbol_table_t symbols;
        diagnostic_factory diagnostic_factory;
    };
}

#endif // AALTITOAD_HAWK_COMPILER_H
