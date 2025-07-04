#pragma once

#include "toml/trait.h"

namespace data
{
    struct Export
    {
        std::optional<fs::path> compile_commands;
    };

    TOML_DESERIALIZE(Export, {
        TOML_OPTIONS(compile_commands);
    });
}