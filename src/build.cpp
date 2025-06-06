#include "build.h"
#include "md5.h"
#include <thread>

std::vector<fs::path> find_all_source(const fs::path &root)
{
    const std::unordered_set<std::string> ext = {"c", "cxx", "cc", "cpp"};
    std::vector<fs::path> result;
    for (const auto &entry : fs::recursive_directory_iterator(root))
    {
        auto file_ext = entry.path().extension().string();
        if (entry.is_regular_file() && ext.find(file_ext) != ext.end())
            result.push_back(entry.path());
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
                          : fs::current_path() / "build";
    this->target_dir = args.hasConfig("target") && args.getConfigs().at("target").size() > 0
                           ? args.getConfigs().at("target").at(0)
                           : fs::current_path() / "target";
}

Build::Build(const BuildInfo &info, const ConfigInfo &config)
    : info(info), config(config) {}

void Build::generate_cmake_root(cmake::Generator &gen)
{
    const char *BINARY = "binary";
    const char *STATIC = "static";
    const char *SHARED = "shared";
    for (const auto &[name, cup] : this->config.dependencies)
        this->generate_cmake_sub(cup.path, gen);
    MD5 hash{this->info.project_dir};
    const auto item = this->config.name + hash.toStr();
    auto src_files = find_all_source(this->info.project_dir / "src");
    if (this->config.build.target == BINARY)
        gen.add_executable(item, src_files);
    else if (this->config.build.target == STATIC)
        gen.add_library(item, cmake::LibaryType::Static, src_files);
    else if (this->config.build.target == SHARED)
        gen.add_library(item, cmake::LibaryType::Shared, src_files);
    else
        throw std::runtime_error("Unknown target type: '" + this->config.build.target + "'");
    gen.set_target_output_name(item, this->config.name);
    gen.target_include_directories(item, cmake::Visual::Public, {this->info.project_dir / "include"});
    std::vector<std::string> libs;
    for (const auto &[name, cup] : this->config.dependencies)
    {
        MD5 lhash(cup.path);
        const auto lib_name = name + lhash.toStr();
        libs.push_back(lib_name);
    }
    gen.target_link_libraries(item, cmake::Visual::Public, libs);
}

void Build::generate_cmake_sub(const fs::path &path, cmake::Generator &gen)
{
    const char *BINARY = "binary";
    const char *STATIC = "static";
    const char *SHARED = "shared";
    Config config(path);
    for (const auto &[name, cup] : config.config->dependencies)
        this->generate_cmake_sub(cup.path, gen);
    MD5 hash(path);
    const auto item = config.config->name + hash.toStr();
    auto src_files = find_all_source(path / "src");
    if (config.config->build.target == BINARY)
        gen.add_executable(item, src_files);
    else if (config.config->build.target == STATIC)
        gen.add_library(item, cmake::LibaryType::Static, src_files);
    else if (config.config->build.target == SHARED)
        gen.add_library(item, cmake::LibaryType::Shared, src_files);
    else
        throw std::runtime_error("Unknown target type: '" + config.config->build.target + "'");
    gen.set_target_output_name(item, config.config->name);
    gen.target_include_directories(item, cmake::Visual::Public, {path / "include"});
    std::vector<std::string> libs;
    for (const auto &[name, cup] : config.config->dependencies)
    {
        MD5 lhash(cup.path);
        const auto lib_name = name + lhash.toStr();
        libs.push_back(lib_name);
    }
    gen.target_link_libraries(item, cmake::Visual::Public, libs);
}

void Build::generate_build(std::ofstream &ofs)
{
    cmake::Generator generator;
    generator.project(this->config.name);
    generator.set_execute_output_path(this->info.target_dir);
    generator.set_library_output_path(this->info.build_dir / "lib");
    this->generate_cmake_root(generator);
    generator.write_to(ofs);
}

int Build::build()
{
    cmake::Execute make;
    make.source(this->info.project_dir);
    make.build(this->info.build_dir);
    make.generator("MinGW Makefiles");
    int res = 0;
    if ((res = system(make.as_command().c_str())) != 0)
        return res;

    cmake::Execute bud;
    make.build_dir(this->info.build_dir);
    make.jobs(std::thread::hardware_concurrency());
    if (this->info.type == BuildType::Release)
        make.config(cmake::Config::Release);
    else
        make.config(cmake::Config::Debug);
    if ((res = system(bud.as_command().c_str())) != 0)
        return res;
    return 0;
}
