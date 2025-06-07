#include "cmake.h"
#include "utils.h"
#include <unordered_set>
#include <sstream>
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
    std::ostringstream oss;
    oss << "add_definitions(";
    for (const auto &define : defines)
        oss << "-D" << define << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::add_executable(const std::string &name, const std::vector<fs::path> &sources)
{
    std::ostringstream oss;
    oss << "add_executable(" << name << " ";
    for (const auto &source : sources)
        oss << replace_all(source.string(), "\\", "/") << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::add_library(const std::string &name, LibaryType type, const std::vector<fs::path> &sources)
{
    std::ostringstream oss;
    oss << "add_library(" << name << " " << (type == LibaryType::Static ? "STATIC" : "SHARED") << " ";
    for (const auto &source : sources)
        oss << replace_all(source.string(), "\\", "/") << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::include_directories(const std::vector<fs::path> &dirs)
{
    std::ostringstream oss;
    oss << "include_directories(";
    for (const auto &dir : dirs)
        oss << replace_all(dir.string(), "\\", "/") << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::link_directories(const std::vector<fs::path> &dirs)
{
    std::ostringstream oss;
    oss << "link_directories(";
    for (const auto &dir : dirs)
        oss << replace_all(dir.string(), "\\", "/") << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::link_libraries(const std::vector<std::string> &libs)
{
    std::ostringstream oss;
    oss << "link_libraries(";
    for (const auto &lib : libs)
        oss << lib << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::add_complie_options(const std::vector<std::string> &options)
{
    std::ostringstream oss;
    oss << "add_compile_options(";
    for (const auto &option : options)
        oss << option << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::add_link_options(const std::vector<std::string> &options)
{
    std::ostringstream oss;
    oss << "add_link_options(";
    for (const auto &option : options)
        oss << option << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::target_link_libraries(const std::string &name, Visual visual, const std::vector<std::string> &libs)
{
    std::ostringstream oss;
    oss << "target_link_libraries(" << name << " " << visual_to_string(visual) << " ";
    for (const auto &lib : libs)
        oss << lib << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::target_link_directories(const std::string &name, Visual visual, const std::vector<fs::path> &dirs)
{
    std::ostringstream oss;
    oss << "target_link_directories(" << name << " " << visual_to_string(visual) << " ";
    for (const auto &dir : dirs)
        oss << replace_all(dir.string(), "\\", "/") << " ";
    oss << ")";
    this->commands.push_back(oss.str());
}

void cmake::Generator::target_include_directories(const std::string &name, Visual visual, const std::vector<fs::path> &dirs)
{
    std::ostringstream oss;
    oss << "target_include_directories(" << name << " " << visual_to_string(visual) << " ";
    for (const auto &dir : dirs)
        oss << replace_all(dir.string(), "\\", "/") << " ";
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

void cmake::Generator::else_()
{
    this->commands.push_back("else()");
}

void cmake::Generator::endif_()
{
    this->commands.push_back("endif()");
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
