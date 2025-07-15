#pragma once

#include "toml_serde/trait.h"

namespace data
{
    struct Project
    {
        std::string name;
        std::string version;
        std::string type;
        std::optional<std::string> license;
    };

    TOML_DESERIALIZE(Project, {
        TOML_REQUIRE(name);
        TOML_REQUIRE(version);
        TOML_REQUIRE(type);
        TOML_OPTIONS(license);
    });
} // namespace data
