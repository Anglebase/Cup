#pragma once

#include "toml/trait.h"
#include "toml/project.h"
#include "toml/build.h"
#include "toml/dependency.h"
#include "toml/default/example.h"
#include <map>

namespace data
{
    struct Static
    {
        Project project;
        std::optional<Build> build;
        std::optional<std::map<std::string, Dependency>> dependencies;
        std::optional<Example> examples;
    };

    TOML_DESERIALIZE(Static, {
        TOML_REQUIRE(project);
        TOML_OPTIONS(build);
        TOML_OPTIONS(dependencies);
        TOML_OPTIONS(examples);
    });
}