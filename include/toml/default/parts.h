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
        std::optional<std::vector<std::string>> compile_options;
        std::optional<std::vector<std::string>> link_options;
        std::optional<std::vector<fs::path>> sources;
        std::optional<std::vector<std::string>> compiler_features;
        std::optional<Part> debug;
        std::optional<Part> release;
    };

    TOML_DESERIALIZE_W(Parts, {
        TOML_OPTIONS(includes);
        TOML_OPTIONS(defines);
        TOML_OPTIONS(link_dirs);
        TOML_OPTIONS(link_libs);
        TOML_OPTIONS(compile_options);
        TOML_OPTIONS(link_options);
        TOML_OPTIONS(sources);
        TOML_OPTIONS(compiler_features);
        TOML_OPTIONS(debug);
        TOML_OPTIONS(release);
    });

    using Generator = Parts;
    using Test = Parts;
    using Example = Parts;
    using Feature = Parts;
    using Target = Parts;
}