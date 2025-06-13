#include "cmake.h"
#include "utils.h"
#include <unordered_set>
#include <sstream>
#include <fstream>
#include "command.h"
using namespace cmake;

std::string visual_to_string(cmake::Visual visual)
{
    switch (visual)
    {
    case cmake::Private:
        return "PRIVATE";
    case cmake::Public:
        return "PUBLIC";
    case cmake::Interface:
        return "INTERFACE";
    default:
        throw std::invalid_argument("Invalid visual: " + std::to_string(static_cast<int>(visual)));
    }
}

cmake::Generator::Generator(const std::string &version)
{
    this->commands.push_back("cmake_minimum_required(VERSION " + version + ")");
}

void cmake::Generator::project(const std::string &name)
{
    this->commands.push_back("project(" + name + ")");
}

void cmake::Generator::enable_language(const std::string &language)
{
    this->commands.push_back("enable_language(" + language + ")");
}

void cmake::Generator::set_system_name(const std::string &name)
{
    this->commands.push_back("set(CMAKE_SYSTEM_NAME " + name + ")");
}

void cmake::Generator::set_system_processor(const std::string &processor)
{
    this->commands.push_back("set(CMAKE_SYSTEM_PROCESSOR " + processor + ")");
}

void cmake::Generator::set_c_compiler(const std::string &compiler)
{
    this->commands.push_back("set(CMAKE_C_COMPILER " + compiler + ")");
}

void cmake::Generator::set_cxx_compiler(const std::string &compiler)
{
    this->commands.push_back("set(CMAKE_CXX_COMPILER " + compiler + ")");
}

void cmake::Generator::set_executable_suffix(const std::string &suffix)
{
    this->commands.push_back("set(CMAKE_EXECUTABLE_SUFFIX_ASM " + suffix + ")");
    this->commands.push_back("set(CMAKE_EXECUTABLE_SUFFIX_C " + suffix + ")");
    this->commands.push_back("set(CMAKE_EXECUTABLE_SUFFIX_CXX " + suffix + ")");
}

void cmake::Generator::set_asm_compiler(const std::string &compiler)
{
    this->commands.push_back("set(CMAKE_ASM_COMPILER " + compiler + ")");
}

void cmake::Generator::set_linker(const std::string &linker)
{
    this->commands.push_back("set(CMAKE_LINKER " + linker + ")");
}

void cmake::Generator::set_c_standard(int standard)
{
    static std::unordered_set<int> valid_standards = {89, 95, 99, 11, 17, 23};
    if (valid_standards.count(standard) == 0)
        throw std::invalid_argument("Invalid C++ standard: c++" + std::to_string(standard));
    this->commands.push_back("set(CMAKE_C_STANDARD " + std::to_string(standard) + ")");
    this->commands.push_back("set(CMAKE_C_STANDARD_REQUIRED ON)");
}

void cmake::Generator::set_cxx_standard(int standard)
{
    static std::unordered_set<int> valid_standards = {98, 03, 11, 14, 17, 20, 23, 26};
    if (valid_standards.count(standard) == 0)
        throw std::invalid_argument("Invalid C++ standard: c++" + std::to_string(standard));
    this->commands.push_back("set(CMAKE_CXX_STANDARD " + std::to_string(standard) + ")");
    this->commands.push_back("set(CMAKE_CXX_STANDARD_REQUIRED ON)");
}

void cmake::Generator::set_target_c_standard(const std::string &name, int standard)
{
    std::ostringstream oss;
    oss << "set_target_properties(" << name << " PROPERTIES C_STANDARD " << standard << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::set_target_cxx_standard(const std::string &name, int standard)
{
    std::ostringstream oss;
    oss << "set_target_properties(" << name << " PROPERTIES CXX_STANDARD " << standard << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::set_execute_output_path(const fs::path &path)
{
    this->commands.push_back("set(EXECUTABLE_OUTPUT_PATH " + replace_all(path.string(), "\\", "/") + ")");
}

void cmake::Generator::set_library_output_path(const fs::path &path)
{
    this->commands.push_back("set(LIBRARY_OUTPUT_PATH " + replace_all(path.string(), "\\", "/") + ")");
}

void cmake::Generator::add_defines(const std::vector<std::string> &defines)
{
    if (defines.empty())
        return;
    std::ostringstream oss;
    oss << "add_definitions(";
    for (const auto &define : defines)
        oss << "-D" << define << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::add_executable(const std::string &name, const std::vector<fs::path> &sources)
{
    if (sources.empty())
        return;
    std::ostringstream oss;
    oss << "add_executable(" << name << " ";
    for (const auto &source : sources)
        oss << replace_all(source.string(), "\\", "/") << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::add_library(const std::string &name, LibaryType type, const std::vector<fs::path> &sources)
{
    if (sources.empty())
        return;
    std::ostringstream oss;
    oss << "add_library(" << name << " " << (type == LibaryType::Static ? "STATIC" : "SHARED") << " ";
    for (const auto &source : sources)
        oss << replace_all(source.string(), "\\", "/") << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::include_directories(const std::vector<fs::path> &dirs)
{
    if (dirs.empty())
        return;
    std::ostringstream oss;
    oss << "include_directories(";
    for (const auto &dir : dirs)
        oss << replace_all(dir.string(), "\\", "/") << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::link_directories(const std::vector<fs::path> &dirs)
{
    if (dirs.empty())
        return;
    std::ostringstream oss;
    oss << "link_directories(";
    for (const auto &dir : dirs)
        oss << replace_all(dir.string(), "\\", "/") << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::link_libraries(const std::vector<std::string> &libs)
{
    if (libs.empty())
        return;
    std::ostringstream oss;
    oss << "link_libraries(";
    for (const auto &lib : libs)
        oss << lib << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::add_complie_options(const std::vector<std::string> &options)
{
    if (options.empty())
        return;
    std::ostringstream oss;
    oss << "add_compile_options(";
    for (const auto &option : options)
        oss << option << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::add_link_options(const std::vector<std::string> &options)
{
    if (options.empty())
        return;
    std::ostringstream oss;
    oss << "add_link_options(";
    for (const auto &option : options)
        oss << option << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::target_compile_definitions(const std::string &name, Visual visual, const std::vector<std::string> &defines)
{
    if (defines.empty())
        return;
    std::ostringstream oss;
    oss << "target_compile_definitions(" << name << " " << visual_to_string(visual) << " ";
    for (const auto &define : defines)
        oss << "-D" << define << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::target_link_libraries(const std::string &name, Visual visual, const std::vector<std::string> &libs)
{
    if (libs.empty())
        return;
    std::ostringstream oss;
    oss << "target_link_libraries(" << name << " " << visual_to_string(visual) << " ";
    for (const auto &lib : libs)
        oss << lib << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::target_link_directories(const std::string &name, Visual visual, const std::vector<fs::path> &dirs)
{
    if (dirs.empty())
        return;
    std::ostringstream oss;
    oss << "target_link_directories(" << name << " " << visual_to_string(visual) << " ";
    for (const auto &dir : dirs)
        oss << replace_all(dir.string(), "\\", "/") << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::target_include_directories(const std::string &name, Visual visual, const std::vector<fs::path> &dirs)
{
    if (dirs.empty())
        return;
    std::ostringstream oss;
    oss << "target_include_directories(" << name << " " << visual_to_string(visual) << " ";
    for (const auto &dir : dirs)
        oss << replace_all(dir.string(), "\\", "/") << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::target_compile_options(const std::string &name, Visual visual, const std::vector<std::string> &options)
{
    if (options.empty())
        return;
    std::ostringstream oss;
    oss << "target_compile_options(" << name << " " << visual_to_string(visual) << " ";
    for (const auto &option : options)
        oss << option << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::target_link_options(const std::string &name, Visual visual, const std::vector<std::string> &options)
{
    if (options.empty())
        return;
    std::ostringstream oss;
    oss << "target_link_options(" << name << " " << visual_to_string(visual) << " ";
    for (const auto &option : options)
        oss << option << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::set_target_output_name(const std::string &name, const std::string &output_name)
{
    std::ostringstream oss;
    oss << "set_target_properties(" << name << " PROPERTIES OUTPUT_NAME " << output_name << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::set_target_output_directory(const std::string &name, const std::optional<LibaryType> &type, const fs::path &output_dir)
{
    std::ostringstream oss;
    oss << "set_target_properties(" << name << " PROPERTIES ";
    oss << (!type.has_value() ? "RUNTIME_OUTPUT_DIRECTORY " : "ARCHIVE_OUTPUT_DIRECTORY ") << replace_all(output_dir.string(), "\\", "/");
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::if_(const std::string &condition)
{
    this->commands.push_back("if(" + condition + ")");
}

void cmake::Generator::find_package(const std::string &name, const std::vector<std::string> &components)
{
    std::ostringstream oss;
    oss << "find_package(" << name;
    if (!components.empty())
    {
        oss << " COMPONENTS ";
        for (const auto &component : components)
            oss << component << " ";
    }
    oss << " REQUIRED)";
    this->commands.push_back(oss.str());
}

void cmake::Generator::set(const std::string &flag, const std::string &value)
{
    this->commands.push_back("set(" + flag + " " + value + ")");
}

void cmake::Generator::target_link_qt_libraries(const std::string &name, cmake::Visual visual, const std::string &version, const std::vector<std::string> &libs)
{
    if (libs.empty())
        return;
    std::ostringstream oss;
    oss << "target_link_libraries(" << name << " " << visual_to_string(visual) << " ";
    for (const auto &lib : libs)
        oss << version << "::" << lib << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::else_()
{
    this->commands.push_back("else()");
}

void cmake::Generator::endif_()
{
    this->commands.push_back("endif()");
}

void cmake::Generator::add_prefix_path(const fs::path &path)
{
    this->set("CMAKE_PREFIX_PATH",
              "${CMAKE_PREFIX_PATH};" + replace_all((path).lexically_normal().string(), "\\", "/"));
}

std::string cmake::Generator::generator() const
{
    std::ostringstream oss;
    for (const auto &command : this->commands)
        oss << command << std::endl;
    return oss.str();
}

void cmake::Generator::write_to(std::ofstream &f) const
{
    for (const auto &command : this->commands)
        f << command << std::endl;
}

bool cmake_valid()
{
    Command cmk("cmake");
    cmk.arg("-version");
    auto cache = get_user_dir() / ".cup" / "cache";
    cmk.set_stdout(cache);
    cmk.set_stderr(cache);
    auto result = cmk.execute();
    if (result != 0)
        throw std::runtime_error("CMake not installed or not in PATH.");
    std::ifstream ifs(cache);
    std::string line;
    while (std::getline(ifs, line))
    {
        if (line.starts_with("cmake version"))
        {
            auto version = line.substr(14);
            auto xyz = split(version, ".");
            if (xyz.size() != 3)
                throw std::runtime_error("Invalid cmake version: " + version);
            int major = std::stoi(xyz[0]);
            int minor = std::stoi(xyz[1]);
            if (major < 3 || (major == 3 && minor < 10))
                throw std::runtime_error("CMake version too low: " + version);
            return true;
        }
    }
    throw std::runtime_error("Invalid cmake version.");
}

cmake::Execute::Execute()
{
    this->commands.push_back("cmake");
}

Execute &cmake::Execute::source(const fs::path &source_dir)
{
    this->commands.push_back("-S");
    this->commands.push_back(source_dir.string());
    return *this;
}

Execute &cmake::Execute::build_dir(const fs::path &build_dir)
{
    this->commands.push_back("-B");
    this->commands.push_back(build_dir.string());
    return *this;
}

Execute &cmake::Execute::generator(const std::string &generator)
{
    this->commands.push_back("-G");
    this->commands.push_back('"' + generator + '"');
    return *this;
}

Execute &cmake::Execute::target(const std::string &target)
{
    this->commands.push_back("--target");
    this->commands.push_back(target);
    return *this;
}

Execute &cmake::Execute::build(const fs::path &build_dir)
{
    this->commands.push_back("--build");
    this->commands.push_back(build_dir.string());
    return *this;
}

Execute &cmake::Execute::config(Config config)
{
    this->commands.push_back("--config");
    this->commands.push_back(config == Config::Debug ? "Debug" : "Release");
    return *this;
}

Execute &cmake::Execute::jobs(int num_jobs)
{
    this->commands.push_back("-j");
    this->commands.push_back(std::to_string(num_jobs));
    return *this;
}

std::string cmake::Execute::as_command() const
{
    return join(this->commands, " ");
}
