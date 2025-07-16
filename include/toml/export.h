#pragma once

#include "toml_serde/trait.h"

namespace data
{
    struct Export
    {
        std::optional<fs::path> compile_commands;
    };

    TOML_DESERIALIZE_W(Export, {
        TOML_OPTIONS(compile_commands);
    });
}