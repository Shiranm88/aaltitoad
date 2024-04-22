#ifndef AALTITOAD_PARSER_H
#define AALTITOAD_PARSER_H
#include "lsp.pb.h"
#include "ntta/tta.h"
#include <memory>

namespace aaltitoad::plugin {
    struct parse_result {
        std::vector<Diagnostic> diagnostics;
        std::optional<std::unique_ptr<ntta_t>> result;
    };

    class parser {
    public:
        virtual ~parser() = default;
        virtual auto parse_files(const std::vector<std::string>& files, const std::vector<std::string>& ignore_patterns) -> parse_result = 0 ;
        virtual auto parse_model(const Buffer& buffer) -> parse_result = 0;
    };
}

#endif
