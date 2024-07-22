#ifndef AALTITOAD_HAWK_MODEL_H
#define AALTITOAD_HAWK_MODEL_H
#include <optional>
#include <string>
#include <vector>

namespace aaltitoad::hawk {
    struct position {
        double x;
        double y;
    };

    namespace scanning {
        struct vertex {
            std::string identifer;
            std::string type;

            struct {
                std::optional<std::string> name;
                std::optional<position> position;
                std::optional<std::vector<std::string>> original_identifiers;
            } debug;
        };

        struct edge {
            std::string identifier;
            std::string source;
            std::string guard;
            std::string update;
            std::string target;

            struct {
                std::optional<std::string> name;
                std::optional<std::vector<std::string>> original_identifiers;
            } debug;
        };

        struct template_t {
            std::string identifier;
            std::string signature;
            std::optional<std::string> declarations;
            std::vector<vertex> vertices;
            std::vector<edge> edges;

            struct {
                std::optional<std::string> name;
                std::optional<std::string> filepath;
            } debug;
        };
    }

    namespace parsing {
        enum class vertex_type {
            LOCATION,
            COMMENT_NAIL,
            GUARD_NAIL,
            UPDATE_NAIL
        };

        struct vertex {
            std::string identifer;
            std::string type;

            struct {
                std::optional<std::string> name;
                std::optional<position> position;
            } debug;
        };
    }
}

#endif // AALTITOAD_HAWK_MODEL_H
