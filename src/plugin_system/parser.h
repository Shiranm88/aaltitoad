#ifndef AALTITOAD_PARSER_H
#define AALTITOAD_PARSER_H
#include "lsp.pb.h"
#include "ntta/tta.h"
#include <memory>

namespace aaltitoad::plugin {
    class parser {
    public:
        virtual ~parser() = default;
        virtual std::unique_ptr<ntta_t> parse_files(const std::vector<std::string>& files, const std::vector<std::string>& ignore_patterns) = 0;
        virtual std::unique_ptr<ntta_t> parse_model(const Buffer& buffer) = 0;
    };
}

#endif
