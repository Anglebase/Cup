#pragma once
#include "toml_serde/trait.h"

namespace data
{
    struct Part
    {
        std::optional<std::vector<fs::path>> includes;
        std::optional<std::vector<std::string>> defines;
        std::optional<std::vector<fs::path>> link_dirs;
        std::optional<std::vector<std::string>> link_libs;
        std::optional<std::vector<std::string>> compile_options;
        std::optional<std::vector<std::string>> link_options;
        std::optional<std::vector<fs::path>> sources;
        std::optional<std::vector<std::string>> compiler_features;
    };

    TOML_DESERIALIZE_W(Part, {
        TOML_OPTIONS(includes);
        TOML_OPTIONS(defines);
        TOML_OPTIONS(link_dirs);
        TOML_OPTIONS(link_libs);
        TOML_OPTIONS(compile_options);
        TOML_OPTIONS(link_options);
        TOML_OPTIONS(sources);
        TOML_OPTIONS(compiler_features);
    });
}