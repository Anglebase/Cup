#pragma once

#include "toml/trait.h"
#include "toml/project.h"
#include "toml/build.h"
#include "toml/dependency.h"
#include "toml/default/part.h"
#include "toml/default/parts.h"
#include <map>

namespace data
{
    struct Shared
    {
        Project project;
        std::optional<Build> build;
        std::optional<std::map<std::string, Dependency>> dependencies;
        std::optional<Test> tests;
        std::optional<Example> examples;
        std::optional<Table<Generator>> generator;
    };

    TOML_DESERIALIZE(Shared, {
        TOML_REQUIRE(project);
        TOML_OPTIONS(build);
        TOML_OPTIONS(dependencies);
        TOML_OPTIONS(tests);
        TOML_OPTIONS(examples);
        TOML_OPTIONS(generator);
    });
}