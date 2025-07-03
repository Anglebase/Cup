#pragma once

#include "toml/trait.h"
#include "toml/project.h"
#include "toml/build.h"
#include "toml/dependency.h"
#include <map>

namespace data
{
    struct Default
    {
        Project project;
        std::optional<Build> build;
        std::optional<std::map<std::string, Dependency>> dependencies;
    };

    TOML_DESERIALIZE(Default, {
        TOML_REQUIRE(project);
        TOML_OPTIONS(build);
        TOML_OPTIONS(dependencies);
    });
}