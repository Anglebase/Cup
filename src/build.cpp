#include "build.h"
#include "md5.h"
#include <thread>
#include <string_view>

std::vector<fs::path> find_all_source(const fs::path &root)
{
    const std::unordered_set<std::string> ext = {".c", ".cxx", ".cc", ".cpp", ".c++", ".s"};
    std::vector<fs::path> result;
    for (const auto &entry : fs::recursive_directory_iterator(root))
    {
        auto file_ext = entry.path().extension().string();
        if (entry.is_regular_file() && ext.find(file_ext) != ext.end())
            result.push_back(entry.path().lexically_normal());
    }
    return result;
}

BuildInfo::BuildInfo(const fs::path &project_dir, const SysArgs &args)
    : project_dir(project_dir)
{
    this->type = args.hasFlag("r") || args.hasFlag("release")
                     ? BuildType::Release
                     : BuildType::Debug;
    this->build_dir = args.hasConfig("build") && args.getConfigs().at("build").size() > 0
                          ? fs::path(args.getConfigs().at("build").at(0))
                          : this->project_dir / "build";
    this->target_dir = args.hasConfig("target") && args.getConfigs().at("target").size() > 0
                           ? fs::path(args.getConfigs().at("target").at(0))
                           : this->project_dir / "target";
    if (this->build_dir.is_relative())
        this->build_dir = fs::current_path() / this->build_dir;
    if (this->target_dir.is_relative())
        this->target_dir = fs::current_path() / this->target_dir;
    this->build_dir = this->build_dir.lexically_normal();
    this->target_dir = this->target_dir.lexically_normal();
    if (args.getArguments().size() <= 1)
    {
        this->build_target = std::nullopt;
        return;
    }
    fs::path target = args.getArguments().at(1);
    if (target.is_relative())
        target = this->project_dir / "bin" / target;
    if (!fs::exists(target))
        throw std::runtime_error("Invalid target path: " + target.string());
    this->build_target = target.lexically_normal();
}

Build::Build(const BuildInfo &info, const ConfigInfo &config)
    : info(info), config(config) {}

void Build::generate_cmake_root(cmake::Generator &gen)
{
    const char *BINARY = "binary";
    const char *STATIC = "static";
    const char *SHARED = "shared";
    if (!this->config.build.generator.empty())
        this->cmake_gen = this->config.build.generator;
    else
    {
#ifdef _WIN32
        LOG_WARN("No generator specified, use default generator \"Visual Studio 17 2022\".");
        this->cmake_gen = "Visual Studio 17 2022";
#else
        LOG_WARN("No generator specified, use default generator \"Unix Makefiles\".");
        this->cmake_gen = "Unix Makefiles";
#endif
    }
    for (const auto &[name, cup] : this->config.dependencies)
    {
        if (this->build_depends.find(std::string(name)) == this->build_depends.end())
        {
            this->generate_cmake_sub(cup, gen);
            this->build_depends.insert(std::string(name));
        }
    }
    auto src_files = fs::exists(this->info.project_dir / "src")
                         ? find_all_source(this->info.project_dir / "src")
                         : std::vector<fs::path>{};
    auto main_files = fs::exists(this->info.project_dir / "bin")
                          ? find_all_source(this->info.project_dir / "bin")
                          : std::vector<fs::path>{};
    auto config_gen = [&](const std::string &item, const std::string &type)
    {
        auto link_visual = type == BINARY ? cmake::Visual::Private : cmake::Visual::Public;
        gen.target_include_directories(item, cmake::Visual::Public, {(this->info.project_dir / "include").lexically_normal()});
        gen.set_target_c_standard(item, this->config.build.stdc);
        gen.set_target_cxx_standard(item, this->config.build.stdcxx);
        gen.target_compile_definitions(
            item,
            cmake::Visual::Private,
            {
                std::string("_VER_X=") + std::to_string(this->config.version.x),
                std::string("_VER_Y=") + std::to_string(this->config.version.y),
                std::string("_VER_Z=") + std::to_string(this->config.version.z),
            });
        gen.target_include_directories(item, cmake::Visual::Public, this->config.build.include);
        gen.target_compile_options(item, cmake::Visual::Private, this->config.build.options.compile);
        gen.target_link_options(item, link_visual, this->config.build.options.link);
        gen.target_compile_definitions(item, cmake::Visual::Private, this->config.build.define);
        if (this->config.qt.has_value())
        {
            auto qt = this->config.qt.value();
            gen.target_link_qt_libraries(item, cmake::Visual::Private, qt.version, qt.modules);
        }
        if (this->info.type == BuildType::Debug)
        {
            gen.target_compile_options(item, cmake::Visual::Private, this->config.build.debug.options.compile);
            gen.target_link_options(item, link_visual, this->config.build.debug.options.link);
            gen.target_compile_definitions(item, cmake::Visual::Private, this->config.build.debug.define);
        }
        else if (this->info.type == BuildType::Release)
        {
            gen.target_compile_options(item, cmake::Visual::Private, this->config.build.release.options.compile);
            gen.target_link_options(item, link_visual, this->config.build.release.options.link);
            gen.target_compile_definitions(item, cmake::Visual::Private, this->config.build.release.define);
        }

        bool has_config;
        for (const auto &[gener, config_gen] : this->config.generators)
        {
            if (gener == this->cmake_gen)
            {
                gen.target_link_options(item, link_visual, config_gen.default_.options.link);
                gen.target_compile_options(item, cmake::Visual::Private, config_gen.default_.options.compile);
                gen.target_compile_definitions(item, cmake::Visual::Private, config_gen.default_.define);
                if (this->info.type == BuildType::Debug)
                {
                    gen.target_compile_options(item, cmake::Visual::Private, config_gen.debug.options.compile);
                    gen.target_link_options(item, link_visual, config_gen.debug.options.link);
                    gen.target_compile_definitions(item, cmake::Visual::Private, config_gen.debug.define);
                }
                else if (this->info.type == BuildType::Release)
                {
                    gen.target_compile_options(item, cmake::Visual::Private, config_gen.release.options.compile);
                    gen.target_link_options(item, link_visual, config_gen.release.options.link);
                    gen.target_compile_definitions(item, cmake::Visual::Private, config_gen.release.define);
                }
                has_config = true;
                break;
            }
        }
        if (!has_config && !this->config.generators.empty())
        {
            LOG_WARN("No generator config for generator \"", *this->cmake_gen, "\" in \"", this->config.name, "\".");
            LOG_WARN("It may not support this generator.");
        }
    };
    LOG_DEBUG("Target:", this->info.target_dir);
    if (this->config.build.target == BINARY)
    {
        for (const auto &main_file : main_files)
        {
            auto source = src_files;
            auto hash = MD5(main_file);
            auto raw_path = main_file;
            const auto raw_name = raw_path.replace_extension().filename().string();
            const auto item = raw_name + "_" + hash.toStr();
            source.push_back(main_file);
            gen.add_executable(item, source);
            gen.set_target_output_name(item, raw_name);
            gen.set_target_output_directory(
                item, std::nullopt,
                replace_finally_name(main_file.parent_path(), "bin", "target", this->info.target_dir));
            config_gen(item, BINARY);
            std::vector<fs::path> libs_dir;
            std::vector<std::string> libs;
            for (const auto &[name, dir] : this->config.link)
            {
                libs.push_back(std::string(name));
                libs_dir.push_back(dir);
            }
            for (const auto &[name, cup] : this->config.dependencies)
            {
                MD5 lhash(cup.path->is_relative() ? this->info.project_dir / *cup.path : *cup.path);
                const auto lib_name = std::string(name) + "_" + lhash.toStr();
                libs.push_back(lib_name);
            }
            gen.target_link_libraries(item, cmake::Visual::Public, libs);
            gen.target_link_directories(item, cmake::Visual::Public, libs_dir);
        }
    }
    else if (this->config.build.target == STATIC || this->config.build.target == SHARED)
    {
        MD5 hash(this->info.project_dir);
        auto item = this->config.name + "_" + hash.toStr();
        gen.add_library(
            item,
            this->config.build.target == STATIC
                ? cmake::LibaryType::Static
                : cmake::LibaryType::Shared,
            src_files);
        gen.set_target_output_name(item, this->config.name);
        config_gen(item, this->config.build.target);
        std::vector<fs::path> libs_dir;
        std::vector<std::string> libs;
        for (const auto &[name, dir] : this->config.link)
        {
            libs.push_back(std::string(name));
            libs_dir.push_back(dir);
        }
        for (const auto &[name, cup] : this->config.dependencies)
        {
            MD5 lhash(cup.path->is_relative() ? this->info.project_dir / *cup.path : *cup.path);
            const auto lib_name = std::string(name) + "_" + lhash.toStr();
            libs.push_back(lib_name);
        }
        gen.target_link_libraries(item, cmake::Visual::Public, libs);
        gen.target_link_directories(item, cmake::Visual::Public, libs_dir);
        for (auto &main_file : main_files)
        {
            auto main_hash = MD5(main_file);
            auto raw_path = main_file;
            const auto raw_name = raw_path.replace_extension().filename().string();
            const auto demo = raw_name + "_" + main_hash.toStr();
            gen.add_executable(demo, {main_file});
            gen.set_target_output_name(demo, raw_name);
            gen.target_link_libraries(demo, cmake::Visual::Public, {item});
            gen.set_target_output_directory(
                demo, std::nullopt,
                replace_finally_name(main_file.parent_path(), "bin", "target", this->info.target_dir));
            gen.set_target_c_standard(demo, this->config.build.stdc);
            gen.set_target_cxx_standard(demo, this->config.build.stdcxx);
        }
    }
    else
        throw std::runtime_error("Unknown target type: '" + this->config.build.target + "'");
}

void Build::generate_cmake_sub(const Dependency &root_cup, cmake::Generator &gen)
{
    const fs::path path = root_cup.path.value();
    const char *BINARY = "binary";
    const char *STATIC = "static";
    const char *SHARED = "shared";
    auto project_dir = path.is_relative() ? this->info.project_dir / path : path;
    Config config(project_dir);
    if (!config.config->build.generator.empty() && config.config->build.generator != this->cmake_gen)
    {
        LOG_WARN("Generator of dependency \"" +
                 config.config->name +
                 "\" is different from the root project. ");
        LOG_DEBUG("This may result in the inability to build.");
    }
    for (const auto &[name, cup] : config.config->dependencies)
    {
        if (this->build_depends.find(std::string(name)) == this->build_depends.end())
        {
            this->generate_cmake_sub(cup, gen);
            this->build_depends.insert(std::string(name));
        }
    }
    auto src_files = fs::exists(project_dir / "src")
                         ? find_all_source(project_dir / "src")
                         : std::vector<fs::path>{};
    auto main_files = fs::exists(project_dir / "bin")
                          ? find_all_source(project_dir / "bin")
                          : std::vector<fs::path>{};
    if (config.config->build.target == BINARY)
    {
        throw std::runtime_error("Binary target(" + config.config->name + ") cannot be used as a dependency.");
    }
    else if (config.config->build.target == STATIC || config.config->build.target == SHARED)
    {
        MD5 hash(project_dir);
        auto item = config.config->name + "_" + hash.toStr();
        gen.add_library(
            item,
            config.config->build.target == STATIC
                ? cmake::LibaryType::Static
                : cmake::LibaryType::Shared,
            src_files);
        gen.set_target_output_name(item, config.config->name);
        gen.target_include_directories(item, cmake::Visual::Public, {(project_dir / "include").lexically_normal()});
        gen.set_target_c_standard(item, config.config->build.stdc);
        gen.set_target_cxx_standard(item, config.config->build.stdcxx);
        gen.target_compile_definitions(
            item,
            cmake::Visual::Private,
            {
                std::string("_VER_X=") + std::to_string(config.config->version.x),
                std::string("_VER_Y=") + std::to_string(config.config->version.y),
                std::string("_VER_Z=") + std::to_string(config.config->version.z),
            });
        gen.target_include_directories(item, cmake::Visual::Public, config.config->build.include);
        gen.target_compile_options(item, cmake::Visual::Private, config.config->build.options.compile);
        gen.target_link_options(item, cmake::Visual::Public, config.config->build.options.link);
        gen.target_compile_definitions(item, cmake::Visual::Private, root_cup.features);
        gen.target_compile_definitions(item, cmake::Visual::Private, config.config->build.define);
        if (config.config->qt.has_value())
        {
            auto qt = config.config->qt.value();
            gen.target_link_qt_libraries(item, cmake::Visual::Public, qt.version, qt.modules);
        }
        if (this->info.type == BuildType::Debug)
        {
            gen.target_compile_options(item, cmake::Visual::Private, config.config->build.debug.options.compile);
            gen.target_link_options(item, cmake::Visual::Private, config.config->build.debug.options.link);
            gen.target_compile_definitions(item, cmake::Visual::Private, config.config->build.debug.define);
        }
        else if (this->info.type == BuildType::Release)
        {
            gen.target_compile_options(item, cmake::Visual::Private, config.config->build.release.options.compile);
            gen.target_link_options(item, cmake::Visual::Private, config.config->build.release.options.link);
            gen.target_compile_definitions(item, cmake::Visual::Private, config.config->build.release.define);
        }
        std::vector<fs::path> libs_dir;
        std::vector<std::string> libs;
        for (const auto &[name, dir] : config.config->link)
        {
            libs.push_back(std::string(name));
            libs_dir.push_back(dir);
        }
        for (const auto &[name, cup] : config.config->dependencies)
        {
            MD5 lhash(cup.path->is_relative() ? project_dir / *cup.path : *cup.path);
            const auto lib_name = std::string(name) + "_" + lhash.toStr();
            libs.push_back(lib_name);
        }
        gen.target_link_libraries(item, cmake::Visual::Public, libs);
        gen.target_link_directories(item, cmake::Visual::Public, libs_dir);

        bool has_config;
        for (const auto &[gener, config_gen] : this->config.generators)
        {
            if (gener == this->cmake_gen)
            {
                gen.target_link_options(item, cmake::Visual::Private, config_gen.default_.options.link);
                gen.target_compile_options(item, cmake::Visual::Private, config_gen.default_.options.compile);
                gen.target_compile_definitions(item, cmake::Visual::Private, config_gen.default_.define);
                if (this->info.type == BuildType::Debug)
                {
                    gen.target_compile_options(item, cmake::Visual::Private, config_gen.debug.options.compile);
                    gen.target_link_options(item, cmake::Visual::Private, config_gen.debug.options.link);
                    gen.target_compile_definitions(item, cmake::Visual::Private, config_gen.debug.define);
                }
                else if (this->info.type == BuildType::Release)
                {
                    gen.target_compile_options(item, cmake::Visual::Private, config_gen.release.options.compile);
                    gen.target_link_options(item, cmake::Visual::Private, config_gen.release.options.link);
                    gen.target_compile_definitions(item, cmake::Visual::Private, config_gen.release.define);
                }
                has_config = true;
                break;
            }
        }
        if (!has_config && !this->config.generators.empty())
        {
            LOG_WARN("No generator config for generator \"", *this->cmake_gen, "\" in \"", this->config.name, "\".");
            LOG_WARN("It may not support this generator.");
        }
    }
    else
        throw std::runtime_error("Unknown target type: '" + config.config->build.target + "'");
}

void Build::generate_build(std::ofstream &ofs)
{
    cmake::Generator generator;
    if (this->config.build.system.name.has_value())
        generator.set_system_name(this->config.build.system.name.value());
    if (this->config.build.system.processor.has_value())
        generator.set_system_processor(this->config.build.system.processor.value());
    generator.project(this->config.name);
    generator.set_execute_output_path(this->info.target_dir);
    generator.set_library_output_path(this->info.build_dir / "lib");

    if (this->info.type == BuildType::Debug)
        generator.add_defines({"DEBUG", "_DEBUG"});
    else
        generator.add_defines({"NDEBUG", "_NDEBUG"});

    generator.if_("MSVC");
    if (this->info.type == BuildType::Debug)
        generator.add_complie_options({"/Zi"});
    else
        generator.add_complie_options({"/O2"});
    generator.else_();
    if (this->info.type == BuildType::Debug)
        generator.add_complie_options({"-g"});
    else
        generator.add_complie_options({"-O2"});
    generator.endif_();

    if (this->config.build.toolchain.cc.has_value())
        generator.set_c_compiler(this->config.build.toolchain.cc.value());
    if (this->config.build.toolchain.cxx.has_value())
        generator.set_cxx_compiler(this->config.build.toolchain.cxx.value());
    if (this->config.build.toolchain.ld.has_value())
        generator.set_linker(this->config.build.toolchain.ld.value());
    if (this->config.build.toolchain.asm_.has_value())
    {
        generator.enable_language("ASM");
        generator.set_asm_compiler(this->config.build.toolchain.asm_.value());
    }
    if (this->config.build.suffix.has_value())
        generator.set_executable_suffix(this->config.build.suffix.value());

    if (this->config.qt.has_value())
        generator.add_prefix_path(*this->config.qt->path);
    if (this->config.qt.has_value())
    {
        auto qt = this->config.qt.value();
        for (auto &&flag : qt.flags)
            generator.set("CMAKE_" + flag, "ON");
        generator.find_package(qt.version, qt.modules);
    }

    if (!this->config.build.flags.c.empty())
        generator.set("CMAKE_C_FLAGS", '"' + join(this->config.build.flags.c, " ") + '"');
    if (!this->config.build.flags.cxx.empty())
        generator.set("CMAKE_CXX_FLAGS", '"' + join(this->config.build.flags.cxx, " ") + '"');
    if (!this->config.build.flags.asm_.empty())
        generator.set("CMAKE_ASM_FLAGS", '"' + join(this->config.build.flags.asm_, " ") + '"');
    if (!this->config.build.flags.ld_c.empty())
        generator.set("CMAKE_C_LINK_FLAGS", '"' + join(this->config.build.flags.ld_c, " ") + '"');
    if (!this->config.build.flags.ld_cxx.empty())
        generator.set("CMAKE_CXX_LINK_FLAGS", '"' + join(this->config.build.flags.ld_cxx, " ") + '"');

    this->generate_cmake_root(generator);
    generator.write_to(ofs);
}

int Build::build()
{
    const auto cmake_build = "cmake";
    cmake::Execute make;
    make.source(this->info.build_dir);
    make.build_dir(this->info.build_dir / cmake_build);
    if (!this->config.build.generator.empty())
        make.generator(this->config.build.generator);
    LOG_DEBUG("Build command: ", make.as_command());
    int res = 0;
    if ((res = system(make.as_command().c_str())) != 0)
    {
        throw std::runtime_error("Build failed with error code: " + std::to_string(res));
    }

    cmake::Execute bud;
    bud.build(this->info.build_dir / cmake_build);
    if (this->info.build_target.has_value())
    {
        auto target = this->info.build_target.value();
        MD5 hash(target);
        const auto item = target.filename().replace_extension().string() + "_" + hash.toStr();
        bud.target(item);
    }
    bud.jobs(this->config.build.jobs == 0 ? std::thread::hardware_concurrency() : this->config.build.jobs);
    LOG_DEBUG("Build command: ", bud.as_command());
    if (this->info.type == BuildType::Release)
        bud.config(cmake::Config::Release);
    else
        bud.config(cmake::Config::Debug);
    if ((res = system(bud.as_command().c_str())) != 0)
    {
        throw std::runtime_error("Build failed with error code: " + std::to_string(res));
    }
    return 0;
}
