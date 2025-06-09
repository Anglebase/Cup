#include "build.h"
#include "md5.h"
#include <thread>

std::vector<fs::path> find_all_source(const fs::path &root)
{
    const std::unordered_set<std::string> ext = {".c", ".cxx", ".cc", ".cpp"};
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
                          ? args.getConfigs().at("build").at(0)
                          : this->project_dir / "build";
    this->target_dir = args.hasConfig("target") && args.getConfigs().at("target").size() > 0
                           ? args.getConfigs().at("target").at(0)
                           : this->project_dir / "target";
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
    for (const auto &[name, cup] : this->config.dependencies)
    {
        if (this->build_depends.find(name) == this->build_depends.end())
        {
            this->generate_cmake_sub(cup, gen);
            this->build_depends.insert(name);
        }
    }
    auto src_files = fs::exists(this->info.project_dir / "src")
                         ? find_all_source(this->info.project_dir / "src")
                         : std::vector<fs::path>{};
    auto main_files = fs::exists(this->info.project_dir / "bin")
                          ? find_all_source(this->info.project_dir / "bin")
                          : std::vector<fs::path>{};
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
            gen.target_include_directories(item, cmake::Visual::Public, {(this->info.project_dir / "include").lexically_normal()});
            gen.set_target_output_directory(item, std::nullopt, replace_finally_name(main_file.parent_path(), "bin", "target"));
            gen.set_target_c_standard(item, this->config.build.stdc);
            gen.set_target_cxx_standard(item, this->config.build.stdcxx);
            gen.target_include_directories(item, cmake::Visual::Public, this->config.build.include);
            gen.target_compile_options(item, cmake::Visual::Private, this->config.build.options.compile);
            gen.target_link_options(item, cmake::Visual::Private, this->config.build.options.link);
            std::vector<fs::path> libs_dir;
            std::vector<std::string> libs;
            for (const auto &[name, dir] : this->config.link)
            {
                libs.push_back(name);
                libs_dir.push_back(dir);
            }
            for (const auto &[name, cup] : this->config.dependencies)
            {
                MD5 lhash(cup.path.is_relative() ? (this->info.project_dir / cup.path).string() : cup.path);
                const auto lib_name = name + "_" + lhash.toStr();
                libs.push_back(lib_name);
            }
            if (!libs.empty())
                gen.target_link_libraries(item, cmake::Visual::Public, libs);
            if (!libs_dir.empty())
                gen.target_link_directories(item, cmake::Visual::Public, libs_dir);
        }
    }
    else if (this->config.build.target == STATIC)
    {
        MD5 hash(this->info.project_dir);
        auto item = this->config.name + "_" + hash.toStr();
        gen.add_library(item, cmake::LibaryType::Static, src_files);
        gen.set_target_output_name(item, this->config.name);
        gen.target_include_directories(item, cmake::Visual::Public, {(this->info.project_dir / "include").lexically_normal()});
        gen.set_target_c_standard(item, this->config.build.stdc);
        gen.set_target_cxx_standard(item, this->config.build.stdcxx);
        gen.target_include_directories(item, cmake::Visual::Public, this->config.build.include);
        gen.target_compile_options(item, cmake::Visual::Private, this->config.build.options.compile);
        gen.target_link_options(item, cmake::Visual::Public, this->config.build.options.link);
        std::vector<fs::path> libs_dir;
        std::vector<std::string> libs;
        for (const auto &[name, dir] : this->config.link)
        {
            libs.push_back(name);
            libs_dir.push_back(dir);
        }
        for (const auto &[name, cup] : this->config.dependencies)
        {
            MD5 lhash(cup.path.is_relative() ? (this->info.project_dir / cup.path).string() : cup.path);
            const auto lib_name = name + "_" + lhash.toStr();
            libs.push_back(lib_name);
        }
        if (!libs.empty())
            gen.target_link_libraries(item, cmake::Visual::Public, libs);
        if (!libs_dir.empty())
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
            gen.set_target_output_directory(demo, std::nullopt, replace_finally_name(main_file.parent_path(), "bin", "target"));
            gen.set_target_c_standard(demo, this->config.build.stdc);
            gen.set_target_cxx_standard(demo, this->config.build.stdcxx);
        }
    }
    else if (this->config.build.target == SHARED)
    {
        MD5 hash(this->info.project_dir);
        auto item = this->config.name + "_" + hash.toStr();
        gen.add_library(item, cmake::LibaryType::Shared, src_files);
        gen.set_target_output_name(item, this->config.name);
        gen.target_include_directories(item, cmake::Visual::Public, {(this->info.project_dir / "include").lexically_normal()});
        gen.set_target_c_standard(item, this->config.build.stdc);
        gen.set_target_cxx_standard(item, this->config.build.stdcxx);
        gen.target_include_directories(item, cmake::Visual::Public, this->config.build.include);
        gen.target_compile_options(item, cmake::Visual::Private, this->config.build.options.compile);
        gen.target_link_options(item, cmake::Visual::Public, this->config.build.options.link);
        std::vector<fs::path> libs_dir;
        std::vector<std::string> libs;
        for (const auto &[name, dir] : this->config.link)
        {
            libs.push_back(name);
            libs_dir.push_back(dir);
        }
        for (const auto &[name, cup] : this->config.dependencies)
        {
            MD5 lhash(cup.path.is_relative() ? (this->info.project_dir / cup.path).string() : cup.path);
            const auto lib_name = name + "_" + lhash.toStr();
            libs.push_back(lib_name);
        }
        if (!libs.empty())
            gen.target_link_libraries(item, cmake::Visual::Public, libs);
        if (!libs_dir.empty())
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
            gen.set_target_output_directory(demo, std::nullopt, replace_finally_name(main_file.parent_path(), "bin", "target"));
            gen.set_target_c_standard(demo, this->config.build.stdc);
            gen.set_target_cxx_standard(demo, this->config.build.stdcxx);
        }
    }
    else
        throw std::runtime_error("Unknown target type: '" + this->config.build.target + "'");
}

void Build::generate_cmake_sub(const CupProject &root_cup, cmake::Generator &gen)
{
    const fs::path path = root_cup.path;
    const char *BINARY = "binary";
    const char *STATIC = "static";
    const char *SHARED = "shared";
    auto project_dir = path.is_relative() ? this->info.project_dir / path : path;
    Config config(project_dir);
    for (const auto &[name, cup] : config.config->dependencies)
    {
        if (this->build_depends.find(name) == this->build_depends.end())
        {
            this->generate_cmake_sub(cup, gen);
            this->build_depends.insert(name);
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
            gen.target_include_directories(item, cmake::Visual::Public, {(project_dir / "include").lexically_normal()});
            gen.set_target_output_directory(item, std::nullopt, replace_finally_name(main_file.parent_path(), "bin", "target"));
            gen.set_target_c_standard(item, config.config->build.stdc);
            gen.set_target_cxx_standard(item, config.config->build.stdcxx);
            gen.target_include_directories(item, cmake::Visual::Public, config.config->build.include);
            gen.target_compile_options(item, cmake::Visual::Private, this->config.build.options.compile);
            gen.target_link_options(item, cmake::Visual::Private, this->config.build.options.link);
            gen.target_compile_definitions(item, cmake::Visual::Private, root_cup.define);
            std::vector<fs::path> libs_dir;
            std::vector<std::string> libs;
            for (const auto &[name, dir] : config.config->link)
            {
                libs.push_back(name);
                libs_dir.push_back(dir);
            }
            for (const auto &[name, cup] : config.config->dependencies)
            {
                MD5 lhash(cup.path.is_relative() ? (project_dir / cup.path).string() : cup.path);
                const auto lib_name = name + "_" + lhash.toStr();
                libs.push_back(lib_name);
            }
            if (!libs.empty())
                gen.target_link_libraries(item, cmake::Visual::Public, libs);
            if (!libs_dir.empty())
                gen.target_link_directories(item, cmake::Visual::Public, libs_dir);
        }
    }
    else if (config.config->build.target == STATIC)
    {
        MD5 hash(project_dir);
        auto item = config.config->name + "_" + hash.toStr();
        gen.add_library(item, cmake::LibaryType::Static, src_files);
        gen.set_target_output_name(item, config.config->name);
        gen.target_include_directories(item, cmake::Visual::Public, {(project_dir / "include").lexically_normal()});
        gen.set_target_c_standard(item, config.config->build.stdc);
        gen.set_target_cxx_standard(item, config.config->build.stdcxx);
        gen.target_include_directories(item, cmake::Visual::Public, config.config->build.include);
        gen.target_compile_options(item, cmake::Visual::Private, this->config.build.options.compile);
        gen.target_link_options(item, cmake::Visual::Public, this->config.build.options.link);
        gen.target_compile_definitions(item, cmake::Visual::Private, root_cup.define);
        std::vector<fs::path> libs_dir;
        std::vector<std::string> libs;
        for (const auto &[name, dir] : config.config->link)
        {
            libs.push_back(name);
            libs_dir.push_back(dir);
        }
        for (const auto &[name, cup] : config.config->dependencies)
        {
            MD5 lhash(cup.path.is_relative() ? (project_dir / cup.path).string() : cup.path);
            const auto lib_name = name + "_" + lhash.toStr();
            libs.push_back(lib_name);
        }
        if (!libs.empty())
            gen.target_link_libraries(item, cmake::Visual::Public, libs);
        if (!libs_dir.empty())
            gen.target_link_directories(item, cmake::Visual::Public, libs_dir);
    }
    else if (config.config->build.target == SHARED)
    {
        MD5 hash(project_dir);
        auto item = config.config->name + "_" + hash.toStr();
        gen.add_library(item, cmake::LibaryType::Shared, src_files);
        gen.set_target_output_name(item, config.config->name);
        gen.target_include_directories(item, cmake::Visual::Public, {(project_dir / "include").lexically_normal()});
        gen.set_target_c_standard(item, config.config->build.stdc);
        gen.set_target_cxx_standard(item, config.config->build.stdcxx);
        gen.target_include_directories(item, cmake::Visual::Public, config.config->build.include);
        gen.target_compile_options(item, cmake::Visual::Private, this->config.build.options.compile);
        gen.target_link_options(item, cmake::Visual::Public, this->config.build.options.link);
        gen.target_compile_definitions(item, cmake::Visual::Private, root_cup.define);
        std::vector<fs::path> libs_dir;
        std::vector<std::string> libs;
        for (const auto &[name, dir] : config.config->link)
        {
            libs.push_back(name);
            libs_dir.push_back(dir);
        }
        for (const auto &[name, cup] : config.config->dependencies)
        {
            MD5 lhash(cup.path.is_relative() ? (project_dir / cup.path).string() : cup.path);
            const auto lib_name = name + "_" + lhash.toStr();
            libs.push_back(lib_name);
        }
        if (!libs.empty())
            gen.target_link_libraries(item, cmake::Visual::Public, libs);
        if (!libs_dir.empty())
            gen.target_link_directories(item, cmake::Visual::Public, libs_dir);
    }
    else
        throw std::runtime_error("Unknown target type: '" + config.config->build.target + "'");
}

void Build::generate_build(std::ofstream &ofs)
{
    cmake::Generator generator;
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
    if (!this->config.build.define.empty())
        generator.add_defines(this->config.build.define);
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
        return res;

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
        return res;
    return 0;
}
