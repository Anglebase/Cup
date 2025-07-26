#pragma once

#include "toml_serde/trait.h"
#include "toml/default/part.h"
#include "toml/export.h"

namespace data
{
    struct Build
    {
        std::optional<std::string> generator;
        std::optional<std::vector<std::string>> defines;
        std::optional<std::vector<fs::path>> includes;
        std::optional<std::vector<fs::path>> link_dirs;
        std::optional<std::vector<std::string>> link_libs;
        std::optional<std::vector<std::string>> compile_options;
        std::optional<std::vector<std::string>> link_options;
        std::optional<std::vector<fs::path>> sources;
        std::optional<std::vector<std::string>> compiler_features;
        std::optional<Part> debug;
        std::optional<Part> release;

        std::optional<Integer> stdc;
        std::optional<Integer> stdcxx;
        std::optional<Integer> jobs;
        std::optional<Array<std::string>> features;
        std::optional<Export> export_data;
        std::optional<Array<std::string>> languages;
    };

    TOML_DESERIALIZE(Build, {
        TOML_OPTIONS(generator);
        TOML_OPTIONS(defines);
        TOML_OPTIONS(includes);
        TOML_OPTIONS(link_dirs);
        TOML_OPTIONS(link_libs);
        TOML_OPTIONS(compile_options);
        TOML_OPTIONS(link_options);
        TOML_OPTIONS(sources);
        TOML_OPTIONS(compiler_features);
        TOML_OPTIONS(stdc);
        TOML_OPTIONS(stdcxx);
        TOML_OPTIONS(jobs);
        TOML_OPTIONS(debug);
        TOML_OPTIONS(release);
        TOML_OPTIONS(features);
        _TOML_OPTIONS(export_data, "export");
        TOML_OPTIONS(languages);
    });
}