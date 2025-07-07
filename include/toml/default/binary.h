#pragma once

#include "toml/trait.h"
#include "toml/project.h"
#include "toml/build.h"
#include "toml/dependency.h"
#include "toml/default/part.h"
#include <map>

namespace data
{
    struct Binary
    {
        Project project;
        std::optional<Build> build;
        std::optional<std::map<std::string, Dependency>> dependencies;
        std::optional<Part> debug;
        std::optional<Part> release;
        std::optional<Part> tests;
    };

    TOML_DESERIALIZE(Binary, {
        TOML_REQUIRE(project);
        TOML_OPTIONS(build);
        TOML_OPTIONS(dependencies);
        TOML_OPTIONS(tests);
        TOML_OPTIONS(debug);
        TOML_OPTIONS(release);
    });
}