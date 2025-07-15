#pragma once

#include "toml_serde/trait.h"

namespace data
{
    struct Dependency
    {
        std::optional<std::string> version;
        std::optional<fs::path> path;
        std::optional<std::string> url;
        std::optional<std::vector<std::string>> features;
        std::optional<std::vector<std::string>> optional;
    };

    TOML_DESERIALIZE(Dependency, {
        TOML_OPTIONS(version);
        TOML_OPTIONS(path);
        TOML_OPTIONS(url);
        TOML_OPTIONS(features);
        TOML_OPTIONS(optional);
    });
}