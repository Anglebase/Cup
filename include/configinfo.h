#pragma once

#include "deserde.hpp"
#include "utils.h"
#include <iostream>

class Config;

struct Version
{
    int x;
    int y;
    int z;

    Version &operator=(const Version &other) = default;
    inline bool operator==(const Version &other) const { return x == other.x && y == other.y && z == other.z; }
};

inline std::ostream &operator<<(std::ostream &os, const Version &version)
{
    os << version.x << "." << version.y << "." << version.z;
    return os;
}

template <>
struct Deserde<Version>
{
    static inline Version from(const toml::node &node)
    {
        auto version = Deserde<std::string>::from(node);
        std::vector<std::string> a_parts = split(version, ".");
        if (a_parts.size() != 3)
            throw std::runtime_error("Invalid version number: " + version);
        std::vector<int> a_nums;
        try
        {
            a_nums = {std::stoi(a_parts[0]), std::stoi(a_parts[1]), std::stoi(a_parts[2])};
        }
        catch (...)
        {
            throw std::runtime_error("Invalid version number: " + version);
        }
        return {
            .x = a_nums[0],
            .y = a_nums[1],
            .z = a_nums[2],
        };
    }
    static inline std::optional<Version> try_from(const toml::node &node) noexcept
    {
        auto version = Deserde<std::string>::try_from(node);
        if (!version)
            return std::nullopt;
        std::vector<std::string> a_parts = split(*version, ".");
        if (a_parts.size() != 3)
            return std::nullopt;
        std::vector<int> a_nums;
        try
        {
            a_nums = {std::stoi(a_parts[0]), std::stoi(a_parts[1]), std::stoi(a_parts[2])};
        }
        catch (...)
        {
            return std::nullopt;
        }
        return Version{
            .x = a_nums[0],
            .y = a_nums[1],
            .z = a_nums[2],
        };
    }
};

struct Options
{
    std::vector<std::string> compile;
    std::vector<std::string> link;
};

template <>
struct Deserde<Options>
{
    static inline Options from(const toml::node &node)
    {
        auto table = require(node.as_table());
        Options options;

        options.compile = option<std::vector<std::string>>(table.get("compile"))
                              .value_or(std::vector<std::string>{});
        options.link = option<std::vector<std::string>>(table.get("link"))
                           .value_or(std::vector<std::string>{});

        return options;
    }
    static inline std::optional<Options> try_from(const toml::node &node) noexcept
    {
        auto table = node.as_table();
        if (!table)
            return std::nullopt;
        Options options;

        options.compile = option<std::vector<std::string>>(table->get("compile"))
                              .value_or(std::vector<std::string>{});
        options.link = option<std::vector<std::string>>(table->get("link"))
                           .value_or(std::vector<std::string>{});

        return options;
    }
};

struct ToolChain
{
    std::optional<std::string> cc;
    std::optional<std::string> cxx;
    std::optional<std::string> asm_;
    std::optional<std::string> ld;
};

template <>
struct Deserde<ToolChain>
{
    static inline ToolChain from(const toml::node &node)
    {
        auto table = require(node.as_table());
        ToolChain tool_chain;

        tool_chain.cc = option<std::string>(table.get("cc"));
        tool_chain.cxx = option<std::string>(table.get("cxx"));
        tool_chain.asm_ = option<std::string>(table.get("asm"));
        tool_chain.ld = option<std::string>(table.get("ld"));

        return tool_chain;
    }
    static inline std::optional<ToolChain> try_from(const toml::node &node) noexcept
    {
        auto table = node.as_table();
        if (!table)
            return std::nullopt;
        ToolChain tool_chain;

        tool_chain.cc = option<std::string>(table->get("cc"));
        tool_chain.cxx = option<std::string>(table->get("cxx"));
        tool_chain.asm_ = option<std::string>(table->get("asm"));
        tool_chain.ld = option<std::string>(table->get("ld"));

        return tool_chain;
    }
};

struct System
{
    std::optional<std::string> name;
    std::optional<std::string> processor;
};

template <>
struct Deserde<System>
{
    static inline System from(const toml::node &node)
    {
        auto table = require(node.as_table());
        System system;

        system.name = option<std::string>(table.get("name"));
        system.processor = option<std::string>(table.get("processor"));

        return system;
    }
    static inline std::optional<System> try_from(const toml::node &node) noexcept
    {
        auto table = node.as_table();
        if (!table)
            return std::nullopt;
        System system;

        system.name = option<std::string>(table->get("name"));
        system.processor = option<std::string>(table->get("processor"));

        return system;
    }
};

struct Flags
{
    std::vector<std::string> c;
    std::vector<std::string> cxx;
    std::vector<std::string> asm_;
    std::vector<std::string> ld_c;
    std::vector<std::string> ld_cxx;
};

template <>
struct Deserde<Flags>
{
    static inline Flags from(const toml::node &node)
    {
        auto table = require(node.as_table());
        Flags flags;

        flags.c = option<std::vector<std::string>>(table.get("c"))
                      .value_or(std::vector<std::string>{});
        flags.cxx = option<std::vector<std::string>>(table.get("cxx"))
                        .value_or(std::vector<std::string>{});
        flags.asm_ = option<std::vector<std::string>>(table.get("asm"))
                         .value_or(std::vector<std::string>{});
        flags.ld_c = option<std::vector<std::string>>(table.get("ld_c"))
                         .value_or(std::vector<std::string>{});
        flags.ld_cxx = option<std::vector<std::string>>(table.get("ld_cxx"))
                           .value_or(std::vector<std::string>{});

        return flags;
    }
    static inline std::optional<Flags> try_from(const toml::node &node) noexcept
    {
        auto table = node.as_table();
        if (!table)
            return std::nullopt;
        Flags flags;

        flags.c = option<std::vector<std::string>>(table->get("c"))
                      .value_or(std::vector<std::string>{});
        flags.cxx = option<std::vector<std::string>>(table->get("cxx"))
                        .value_or(std::vector<std::string>{});
        flags.asm_ = option<std::vector<std::string>>(table->get("asm"))
                         .value_or(std::vector<std::string>{});
        flags.ld_c = option<std::vector<std::string>>(table->get("ld_c"))
                         .value_or(std::vector<std::string>{});
        flags.ld_cxx = option<std::vector<std::string>>(table->get("ld_cxx"))
                           .value_or(std::vector<std::string>{});

        return flags;
    }
};

struct ModeSettings
{
    Options options;
    Flags flags;
    std::vector<std::string> define;
};

template <>
struct Deserde<ModeSettings>
{
    static inline ModeSettings from(const toml::node &node)
    {
        auto table = require(node.as_table());
        ModeSettings mode_settings;

        mode_settings.options = option<Options>(table.get("options")).value_or(Options{});
        mode_settings.flags = option<Flags>(table.get("flags")).value_or(Flags{});
        mode_settings.define = option<std::vector<std::string>>(table.get("define"))
                                   .value_or(std::vector<std::string>{});

        return mode_settings;
    }
    static inline std::optional<ModeSettings> try_from(const toml::node &node) noexcept
    {
        auto table = node.as_table();
        if (!table)
            return std::nullopt;
        ModeSettings mode_settings;

        mode_settings.options = option<Options>(table->get("options")).value_or(Options{});
        mode_settings.flags = option<Flags>(table->get("flags")).value_or(Flags{});
        mode_settings.define = option<std::vector<std::string>>(table->get("define"))
                                   .value_or(std::vector<std::string>{});

        return mode_settings;
    }
};

struct BuildSettings
{
    std::string target;
    std::string generator;
    int64_t stdc;
    int64_t stdcxx;
    int64_t jobs;
    std::vector<fs::path> include;
    std::vector<std::string> define;
    std::optional<std::string> suffix;
    Options options;
    ToolChain toolchain;
    System system;
    Flags flags;
    ModeSettings debug;
    ModeSettings release;
};

template <>
struct Deserde<BuildSettings>
{
    static inline BuildSettings from(const toml::node &node)
    {
        auto table = require(node.as_table());
        BuildSettings build_settings;

        build_settings.target = require<std::string>(table.get("target"));
        build_settings.generator = option<std::string>(table.get("generator")).value_or("");
        build_settings.stdc = option<int64_t>(table.get("stdc")).value_or(17);
        build_settings.stdcxx = option<int64_t>(table.get("stdcxx")).value_or(20);
        build_settings.jobs = option<int64_t>(table.get("jobs")).value_or(1);
        build_settings.include = option<std::vector<fs::path>>(table.get("include"))
                                     .value_or(std::vector<fs::path>{});
        build_settings.define = option<std::vector<std::string>>(table.get("define"))
                                    .value_or(std::vector<std::string>{});
#ifdef _DEBUG
        std::cout << "Debug mode: [include]" << std::endl;
        for (auto &p : build_settings.include)
            std::cout << p << std::endl;
#endif
        build_settings.suffix = option<std::string>(table.get("suffix"));
        build_settings.options = option<Options>(table.get("options")).value_or(Options{});
        build_settings.toolchain = option<ToolChain>(table.get("toolchain")).value_or(ToolChain{});
        build_settings.system = option<System>(table.get("system")).value_or(System{});
        build_settings.flags = option<Flags>(table.get("flags")).value_or(Flags{});
        build_settings.debug = option<ModeSettings>(table.get("debug")).value_or(ModeSettings{});
        build_settings.release = option<ModeSettings>(table.get("release")).value_or(ModeSettings{});

        return build_settings;
    }
    static inline std::optional<BuildSettings> try_from(const toml::node &node) noexcept
    {
        auto table = node.as_table();
        if (!table)
            return std::nullopt;
        BuildSettings build_settings;

        auto target = option<std::string>(table->get("target"));
        if (!target)
            return std::nullopt;
        build_settings.target = *target;
        build_settings.generator = option<std::string>(table->get("generator")).value_or("");
        build_settings.stdc = option<int64_t>(table->get("stdc")).value_or(17);
        build_settings.stdcxx = option<int64_t>(table->get("stdcxx")).value_or(20);
        build_settings.include = option<std::vector<fs::path>>(table->get("include"))
                                     .value_or(std::vector<fs::path>{});
#ifdef _DEBUG
        std::cout << "Debug mode: [include]" << std::endl;
        for (auto &p : build_settings.include)
            std::cout << p << std::endl;
#endif
        build_settings.define = option<std::vector<std::string>>(table->get("define"))
                                    .value_or(std::vector<std::string>{});
        build_settings.suffix = option<std::string>(table->get("suffix"));
        build_settings.options = option<Options>(table->get("options")).value_or(Options{});
        build_settings.toolchain = option<ToolChain>(table->get("toolchain")).value_or(ToolChain{});
        build_settings.system = option<System>(table->get("system")).value_or(System{});
        build_settings.flags = option<Flags>(table->get("flags")).value_or(Flags{});
        build_settings.debug = option<ModeSettings>(table->get("debug")).value_or(ModeSettings{});
        build_settings.release = option<ModeSettings>(table->get("release")).value_or(ModeSettings{});

        return build_settings;
    }
};

struct Dependency
{
    std::optional<fs::path> path;
    std::optional<std::string> git;
    std::optional<std::string> version;
    std::vector<std::string> features;

    std::string name;
    fs::path get_path() const;
};

template <>
struct Deserde<Dependency>
{
    static inline Dependency from(const toml::node &node)
    {
        auto table = require(node.as_table());
        Dependency dependency;

        dependency.path = option<fs::path>(table.get("path"));
        dependency.git = option<std::string>(table.get("git"));
        dependency.version = option<std::string>(table.get("version"));
        dependency.features = option<std::vector<std::string>>(table.get("features"))
                                  .value_or(std::vector<std::string>{});
        return dependency;
    }
    static inline std::optional<Dependency> try_from(const toml::node &node) noexcept
    {
        auto table = node.as_table();
        if (!table)
            return std::nullopt;
        Dependency dependency;

        dependency.path = option<fs::path>(table->get("path"));
        dependency.git = option<std::string>(table->get("git"));
        dependency.version = option<std::string>(table->get("version"));
        dependency.features = option<std::vector<std::string>>(table->get("features"))
                                  .value_or(std::vector<std::string>{});

        return dependency;
    }
};

struct Qt
{
    std::optional<fs::path> path;
    std::string version;
    std::vector<std::string> modules;
    std::vector<std::string> flags;
};

struct Link
{
    std::vector<std::string> libs;
    std::vector<fs::path> paths;
};

template <>
struct Deserde<Link>
{
    static inline Link from(const toml::node &node)
    {
        auto table = require(node.as_table());
        Link link;

        link.libs = option<std::vector<std::string>>(table.get("libs"))
                        .value_or(std::vector<std::string>{});
        link.paths = option<std::vector<fs::path>>(table.get("paths"))
                         .value_or(std::vector<fs::path>{});

        return link;
    }
    static inline std::optional<Link> try_from(const toml::node &node) noexcept
    {
        auto table = node.as_table();
        if (!table)
            return std::nullopt;
        Link link;

        link.libs = option<std::vector<std::string>>(table->get("libs"))
                        .value_or(std::vector<std::string>{});
        link.paths = option<std::vector<fs::path>>(table->get("paths"))
                         .value_or(std::vector<fs::path>{});

        return link;
    }
};

struct GeneratorInfo
{
    Link link;
    Options options;
    std::vector<std::string> define;
};

template <>
struct Deserde<GeneratorInfo>
{
    static inline GeneratorInfo from(const toml::node &node)
    {
        auto table = require(node.as_table());
        GeneratorInfo generator;
        generator.link = option<Link>(table.get("link"))
                             .value_or(Link{});
        generator.options = option<Options>(table.get("options")).value_or(Options{});
        generator.define = option<std::vector<std::string>>(table.get("define"))
                               .value_or(std::vector<std::string>{});

        return generator;
    }
    static inline std::optional<GeneratorInfo> try_from(const toml::node &node) noexcept
    {
        auto table = node.as_table();
        if (!table)
            return std::nullopt;
        GeneratorInfo generator;
        generator.link = option<Link>(table->get("link"))
                             .value_or(Link{});
        generator.options = option<Options>(table->get("options")).value_or(Options{});
        generator.define = option<std::vector<std::string>>(table->get("define"))
                               .value_or(std::vector<std::string>{});

        return generator;
    }
};

struct GeneratorSettings
{
    GeneratorInfo default_;
    GeneratorInfo debug;
    GeneratorInfo release;
};

template <>
struct Deserde<GeneratorSettings>
{
    static inline GeneratorSettings from(const toml::node &node)
    {
        auto table = require(node.as_table());
        GeneratorSettings generator_settings;
        generator_settings.default_ = Deserde<GeneratorInfo>::from(table);
        generator_settings.debug = option<GeneratorInfo>(table.get("debug")).value_or(GeneratorInfo{});
        generator_settings.release = option<GeneratorInfo>(table.get("release"))
                                         .value_or(GeneratorInfo{});

        return generator_settings;
    }
    static inline std::optional<GeneratorSettings> try_from(const toml::node &node) noexcept
    {
        auto table = node.as_table();
        if (!table)
            return std::nullopt;
        GeneratorSettings generator_settings;
        generator_settings.default_ = Deserde<GeneratorInfo>::try_from(*table).value();
        generator_settings.debug = option<GeneratorInfo>(table->get("debug")).value_or(GeneratorInfo{});
        generator_settings.release = option<GeneratorInfo>(table->get("release"))
                                         .value_or(GeneratorInfo{});

        return generator_settings;
    }
};

struct ConfigInfo
{
    std::string name;
    Version version;
    std::string description;
    std::vector<std::string> authors;
    std::string license;
    BuildSettings build;
    Link link;
    std::map<std::string, Dependency> dependencies;
    std::map<std::string, GeneratorSettings> generators;

    std::optional<Qt> qt;

public:
    ConfigInfo() = default;
    ConfigInfo(const Config &config);
};

template <>
struct Deserde<ConfigInfo>
{
    static inline ConfigInfo from(const toml::node &node)
    {
        auto table = require(node.as_table());
        ConfigInfo config_info;

        config_info.name = require<std::string>(table.get("name"));
        config_info.version = require<Version>(table.get("version"));
        config_info.description = option<std::string>(table.get("description")).value_or("");
        config_info.authors = option<std::vector<std::string>>(table.get("authors"))
                                  .value_or(std::vector<std::string>{});
        config_info.license = option<std::string>(table.get("license")).value_or("");
        config_info.build = require<BuildSettings>(table.get("build"));
        config_info.link = option<Link>(table.get("link"))
                               .value_or(Link{});
        config_info.dependencies = option<std::map<std::string, Dependency>>(table.get("dependencies"))
                                       .value_or(std::map<std::string, Dependency>{});
        config_info.generators = option<std::map<std::string, GeneratorSettings>>(table.get("generator"))
                                     .value_or(std::map<std::string, GeneratorSettings>{});

        for (auto &[name, dep] : config_info.dependencies)
            dep.name = name;
        return config_info;
    }

    static inline std::optional<ConfigInfo> try_from(const toml::node &node) noexcept
    {
        auto table = node.as_table();
        if (!table)
            return std::nullopt;
        ConfigInfo config_info;

        auto name = option<std::string>(table->get("name"));
        if (!name)
            return std::nullopt;
        config_info.name = *name;
        config_info.version = option<Version>(table->get("version")).value_or(Version{});
        config_info.description = option<std::string>(table->get("description")).value_or("");
        config_info.authors = option<std::vector<std::string>>(table->get("authors"))
                                  .value_or(std::vector<std::string>{});
        config_info.license = option<std::string>(table->get("license")).value_or("");
        auto build = option<BuildSettings>(table->get("build"));
        if (!build)
            return std::nullopt;
        config_info.build = *build;
        config_info.link = option<Link>(table->get("link"))
                               .value_or(Link{});
        config_info.dependencies = option<std::map<std::string, Dependency>>(table->get("dependencies"))
                                       .value_or(std::map<std::string, Dependency>{});
        config_info.generators = option<std::map<std::string, GeneratorSettings>>(table->get("generator"))
                                     .value_or(std::map<std::string, GeneratorSettings>{});

        for (auto &[name, dep] : config_info.dependencies)
            dep.name = name;
        return config_info;
    }
};