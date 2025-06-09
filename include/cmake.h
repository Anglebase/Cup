#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <thread>
#include <optional>
namespace fs = std::filesystem;

namespace cmake
{
    enum Visual
    {
        Private,
        Public,
        Interface,
    };

    enum LibaryType
    {
        Static,
        Shared,
    };

    class Generator
    {
        std::vector<std::string> commands;

    public:
        Generator(const std::string &version = "3.10");
        void project(const std::string &name);
        void enable_language(const std::string &language);
        void set_system_name(const std::string &name);
        void set_system_processor(const std::string &processor);
        void set_c_compiler(const std::string &compiler);
        void set_cxx_compiler(const std::string &compiler);
        void set_asm_compiler(const std::string &compiler);
        void set_linker(const std::string &linker);
        void set_c_standard(int standard);
        void set_cxx_standard(int standard);
        void set_target_c_standard(const std::string &name, int standard);
        void set_target_cxx_standard(const std::string &name, int standard);
        void set_execute_output_path(const fs::path &path);
        void set_library_output_path(const fs::path &path);
        void add_defines(const std::vector<std::string> &defines);
        void add_executable(const std::string &name, const std::vector<fs::path> &sources);
        void add_library(const std::string &name, LibaryType type, const std::vector<fs::path> &sources);
        void include_directories(const std::vector<fs::path> &dirs);
        void link_directories(const std::vector<fs::path> &dirs);
        void link_libraries(const std::vector<std::string> &libs);
        void add_complie_options(const std::vector<std::string> &options);
        void add_link_options(const std::vector<std::string> &options);
        void target_compile_definitions(const std::string &name, Visual visual, const std::vector<std::string> &defines);
        void target_link_libraries(const std::string &name, Visual visual, const std::vector<std::string> &libs);
        void target_link_directories(const std::string &name, Visual visual, const std::vector<fs::path> &dirs);
        void target_include_directories(const std::string &name, Visual visual, const std::vector<fs::path> &dirs);
        void target_compile_options(const std::string &name, Visual visual, const std::vector<std::string> &options);
        void target_link_options(const std::string &name, Visual visual, const std::vector<std::string> &options);
        void set_target_output_name(const std::string &name, const std::string &output_name);
        void set_target_output_directory(const std::string &name, const std::optional<LibaryType> &type, const fs::path &output_dir);
        void if_(const std::string &condition);
        void else_();
        void endif_();

    public:
        std::string generator() const;
        void write_to(std::ofstream &f) const;
    };

    enum Config
    {
        Debug,
        Release,
    };

    class Execute
    {
        std::vector<std::string> commands;

    public:
        Execute();

        Execute &source(const fs::path &source_dir);
        Execute &build_dir(const fs::path &build_dir);
        Execute &generator(const std::string &generator);

        Execute &target(const std::string &target);
        Execute &build(const fs::path &build_dir);
        Execute &config(Config config);

        Execute &jobs(int num_jobs = std::thread::hardware_concurrency());

        std::string as_command() const;
    };
} // namespace cmake