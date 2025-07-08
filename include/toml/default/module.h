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
    struct Module
    {
        Project project;
        std::optional<Build> build;
        std::optional<std::map<std::string, Dependency>> dependencies;
        std::optional<Test> tests;
        std::optional<Table<Generator>> generator;
        std::optional<Table<Array<std::string>>> features;
    };

    TOML_DESERIALIZE(Module, {
        TOML_REQUIRE(project);
        TOML_OPTIONS(build);
        TOML_OPTIONS(dependencies);
        TOML_OPTIONS(tests);
        TOML_OPTIONS(generator);
        TOML_OPTIONS(features);
    });
}