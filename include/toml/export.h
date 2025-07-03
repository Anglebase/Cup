#pragma once

#include "toml/trait.h"

namespace data
{
    struct Export
    {
        std::optional<fs::path> compiler_comand;
    };

    TOML_DESERIALIZE(Export, {
        TOML_OPTIONS(compiler_comand);
    });
}