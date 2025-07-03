#pragma once

#include "toml/trait.h"
#include "toml/export.h"

namespace data
{
    struct Build
    {
        std::optional<std::string> generator;
        std::optional<std::vector<std::string>> defines;
        std::optional<std::vector<fs::path>> includes;
        std::optional<std::vector<fs::path>> link_dirs;
        std::optional<std::vector<std::string>> link_libs;
        std::optional<std::vector<std::string>> compiler_options;
        std::optional<std::vector<std::string>> link_options;
        std::optional<Export> export_data;
    };

    TOML_DESERIALIZE(Build, {
        TOML_OPTIONS(generator);
        TOML_OPTIONS(defines);
        TOML_OPTIONS(includes);
        TOML_OPTIONS(link_dirs);
        TOML_OPTIONS(link_libs);
        TOML_OPTIONS(compiler_options);
        TOML_OPTIONS(link_options);
        TOML_OPTIONS(export_data);
    });
}