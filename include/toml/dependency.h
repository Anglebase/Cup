#pragma once

#include "toml/trait.h"

namespace data
{
    struct Dependency
    {
        std::optional<std::string> version;
        std::optional<fs::path> path;
        std::optional<std::string> url;
        std::optional<std::vector<std::string>> features;
    };

    TOML_DESERIALIZE(Dependency, {
        TOML_OPTIONS(version);
        TOML_OPTIONS(path);
        TOML_OPTIONS(url);
        TOML_OPTIONS(features);
    });
}