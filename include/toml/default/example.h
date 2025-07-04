#pragma once
#include "toml/trait.h"

namespace data
{
    struct Example
    {
        std::optional<std::vector<fs::path>> includes;
        std::optional<std::vector<std::string>> defines;
    };

    TOML_DESERIALIZE(Example, {
        TOML_OPTIONS(includes);
        TOML_OPTIONS(defines);
    });
}