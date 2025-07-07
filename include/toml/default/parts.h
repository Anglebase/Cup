#pragma once

#include "toml/default/part.h"

namespace data
{
    struct Parts
    {
        std::optional<std::vector<fs::path>> includes;
        std::optional<std::vector<std::string>> defines;
        std::optional<std::vector<fs::path>> link_dirs;
        std::optional<std::vector<std::string>> link_libs;
        std::optional<std::vector<std::string>> compiler_options;
        std::optional<std::vector<std::string>> link_options;
        std::optional<Part> debug;
        std::optional<Part> release;
    };

    TOML_DESERIALIZE(Parts, {
        TOML_OPTIONS(includes);
        TOML_OPTIONS(defines);
        TOML_OPTIONS(link_dirs);
        TOML_OPTIONS(link_libs);
        TOML_OPTIONS(compiler_options);
        TOML_OPTIONS(link_options);
        TOML_OPTIONS(debug);
        TOML_OPTIONS(release);
    });

    using Generator = Parts;
    using Test = Parts;
    using Examples = Parts;
}