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
            std::vector<std::string> modifiers; // e.g. "IMMEDIATE", "FINAL"

            struct {
                std::optional<std::string> name;
                std::optional<position> position;
            } debug;
        };

        struct edge {
            std::string identifier;
            std::string source;
            std::optional<std::string> guard;
            std::optional<std::string> update;
            std::string target;

            struct {
                std::optional<std::string> name;
            } debug;
        };

        struct template_t {
            std::string identifier;
            std::string signature;
            std::optional<std::string> declarations;
            std::vector<vertex> vertices;
            std::vector<edge> edges;
            std::vector<std::string> modifiers; // e.g. "MAIN"

            struct {
                std::optional<std::string> name;
                std::optional<std::string> filepath;
            } debug;
        };
    }

    namespace parsing {
        enum class location_mod {
            IMMEDATE,
            INITIAL,
            FINAL
        };

        enum class vertex_type {
            LOCATION,
            INTERMEDIATE,
            TEMPLATE_INSTANCE
        };
    }
}

#endif // AALTITOAD_HAWK_MODEL_H
