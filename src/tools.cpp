#include "tools.h"
#include "build.h"
#include <fstream>

NewCmd::NewCmd(const SysArgs &args)
    : at(std::nullopt), target(std::nullopt)
{
    if (args.getArguments().size() < 2)
        throw std::runtime_error("No project name provided.");
    this->name = args.getArguments()[1];
    if (args.hasConfig("dir") && !args.getConfigs().at("dir").empty())
        this->at = args.getConfigs().at("dir")[0];
    if (args.hasConfig("target") && !args.getConfigs().at("target").empty())
        this->target = args.getConfigs().at("target")[0];
}

CleanCmd::CleanCmd(const SysArgs &args)
{
    if (args.hasConfig("dir") && !args.getConfigs().at("dir").empty())
        this->at = args.getConfigs().at("dir")[0];
}

BuildCmd::BuildCmd(const SysArgs &args)
    : args(args)
{
    if (args.hasConfig("dir") && !args.getConfigs().at("dir").empty())
        this->at = args.getConfigs().at("dir")[0];
    this->config = args.hasFlag("r") || args.hasFlag("release")
                       ? BuildType::Release
                       : BuildType::Debug;
}

RunCmd::RunCmd(const SysArgs &args) : BuildCmd(args)
{
    if (args.hasConfig("args"))
        this->run_args = args.getConfigs().at("args");
    if (args.getArguments().size() <= 1)
        throw std::runtime_error("No target provided.");
    auto target = args.getArguments()[1];
    auto dir = this->at.value_or(fs::current_path());
    if (dir.is_relative())
        dir = fs::current_path() / dir;
    dir = dir / "target" / target;
    dir.replace_extension();
    this->run_target = dir.lexically_normal();
}

int RunCmd::exec()
{
    auto res = 0;
    if ((res = this->run()) != 0)
        return res;
#ifdef _WIN32
    this->run_target.replace_extension(".exe");
#endif
    if (!fs::exists(this->run_target))
        this->run_target = this->run_target.parent_path() /
                           (this->config == BuildType::Release ? "Release" : "Debug") /
                           this->run_target.filename();
    auto dir = this->at.value_or(fs::current_path());
    if (dir.is_relative())
        dir = fs::current_path() / dir;
    auto dll_at = dir / "build" / "lib";
    for (auto &dll : fs::directory_iterator(dll_at))
        if (dll.is_regular_file() && dll.path().extension() == ".dll")
            fs::copy_file(dll.path(), this->run_target.parent_path() / dll.path().filename(),
                          fs::copy_options::overwrite_existing);
    dll_at = dll_at / (this->config == BuildType::Release ? "Release" : "Debug");
    for (auto &dll : fs::directory_iterator(dll_at))
        if (dll.is_regular_file() && dll.path().extension() == ".dll")
            fs::copy_file(dll.path(), this->run_target.parent_path() / dll.path().filename(),
                          fs::copy_options::overwrite_existing);
    LOG_INFO("RunCmd: ", this->run_target.string());
    res = std::system((this->run_target.string() + " " + join(this->run_args, " ")).c_str());
    LOG_INFO("RunCmd: ", res);
    return 0;
}

int NewCmd::run()
{
    if (this->at.has_value())
        fs::current_path(this->at.value());
    auto project_path = fs::current_path() / this->name;
    if(fs::exists(project_path))
        throw std::runtime_error("Project already exists: " + project_path.string());
    std::vector<std::string> sub_dir = {"include", "src", "bin"};
    for (auto &dir : sub_dir)
        fs::create_directories(project_path / dir);

    std::ofstream cup_config(project_path / "cup.toml");
    cup_config << "name = \"" << this->name << "\"" << std::endl;
    cup_config << "version = \"0.1.0\"" << std::endl
               << std::endl;
    cup_config << "[build]" << std::endl;
    cup_config << "target = \"" << this->target.value_or("binary") << "\"" << std::endl;
    cup_config.close();

    std::ofstream gitignore(project_path / ".gitignore");
    gitignore << "/build" << std::endl;
    gitignore << "/target" << std::endl;
    gitignore.close();

    auto target = this->target.value_or("binary");
    if (target == "binary")
    {
        std::ofstream main_cpp(project_path / "bin" / (this->name + ".cpp"));
        main_cpp << "#include <iostream>" << std::endl;
        main_cpp << "int main() {" << std::endl;
        main_cpp << "    std::cout << \"Hello, world!\" << std::endl;" << std::endl;
        main_cpp << "    return 0;" << std::endl;
        main_cpp << "}" << std::endl;
        main_cpp.close();
    }
    else if (target == "static")
    {
        std::ofstream lib_h(project_path / "include" / (this->name + ".h"));
        lib_h << "#pragma once" << std::endl
              << std::endl;
        lib_h << "void " << this->name << "();" << std::endl;
        lib_h.close();

        std::ofstream lib_cpp(project_path / "src" / (this->name + ".cpp"));
        lib_cpp << "#include <iostream>" << std::endl;
        lib_cpp << "#include \"" << this->name << ".h\"" << std::endl
                << std::endl;
        lib_cpp << "void " << this->name << "() {" << std::endl;
        lib_cpp << "    std::cout << \"" << this->name << "() called!\" << std::endl;" << std::endl;
        lib_cpp << "}" << std::endl;
        lib_cpp.close();

        std::ofstream main_cpp(project_path / "bin" / (this->name + ".cpp"));
        main_cpp << "#include \"" << this->name << ".h\"" << std::endl
                 << std::endl;
        main_cpp << "int main() {" << std::endl;
        main_cpp << "    " << this->name << "();" << std::endl;
        main_cpp << "    return 0;" << std::endl;
        main_cpp << "}" << std::endl;
        main_cpp.close();
    }
    else if (target == "shared")
    {
        std::ofstream lib_h(project_path / "include" / (this->name + ".h"));
        lib_h << "#pragma once" << std::endl
              << std::endl;
        lib_h << "#ifdef _MSC_VER" << std::endl;
        lib_h << "    #define " << "DLLEXPORT __declspec(dllexport)" << std::endl;
        lib_h << "#else" << std::endl;
        lib_h << "    #define " << "DLLEXPORT" << std::endl;
        lib_h << "#endif" << std::endl
              << std::endl;
        lib_h << "DLLEXPORT void " << this->name << "();" << std::endl;
        lib_h.close();

        std::ofstream lib_cpp(project_path / "src" / (this->name + ".cpp"));
        lib_cpp << "#include <iostream>" << std::endl;
        lib_cpp << "#include \"" << this->name << ".h\"" << std::endl
                << std::endl;
        lib_cpp << "DLLEXPORT void " << this->name << "() {" << std::endl;
        lib_cpp << "    std::cout << \"" << this->name << "() called!\" << std::endl;" << std::endl;
        lib_cpp << "}" << std::endl;
        lib_cpp.close();

        std::ofstream main_cpp(project_path / "bin" / (this->name + ".cpp"));
        main_cpp << "#include \"" << this->name << ".h\"" << std::endl
                 << std::endl;
        main_cpp << "int main() {" << std::endl;
        main_cpp << "    " << this->name << "();" << std::endl;
        main_cpp << "    return 0;" << std::endl;
        main_cpp << "}" << std::endl;
        main_cpp.close();
    }
    else
    {
        fs::remove_all(project_path);
        throw std::runtime_error("Invalid target: " + target);
    }
    return 0;
}

int CleanCmd::run()
{
    if (this->at.has_value())
        fs::current_path(this->at.value());
    fs::remove_all("build");
    return 0;
}

int BuildCmd::run()
{
    auto dir = this->at.value_or(fs::current_path());
    if (dir.is_relative())
        dir = fs::current_path() / dir;
    dir = dir.lexically_normal();
    auto build_info = BuildInfo(dir, this->args);
    auto config_info = ConfigInfo(Config(dir));
    auto build = Build(build_info, config_info);
    if (!fs::exists(build_info.build_dir))
        fs::create_directories(build_info.build_dir);
    std::ofstream ofs(build_info.build_dir / "CMakeLists.txt");
    build.generate_build(ofs);
    ofs.close();
    return build.build();
}