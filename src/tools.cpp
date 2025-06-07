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
    if (args.hasConfig("target") && !args.getConfigs().at("target").empty())
        this->target = args.getConfigs().at("target")[0];
    if (args.hasConfig("config") && !args.getConfigs().at("config").empty())
        this->config = args.getConfigs().at("config")[0];
}

RunCmd::RunCmd(const SysArgs &args) : BuildCmd(args)
{
}

int RunCmd::exec()
{
    auto res = 0;
    if ((res = this->run()) != 0)
        return res;
    auto dir = this->at.value_or(fs::current_path());
    Config config(dir);
    auto exe_name = this->target.value_or(config.config->name);
    auto exe_path = dir / "target" / (exe_name);
    res = std::system((exe_path.string()).c_str());
    LOG_INFO("RunCmd: {}", res);
    return 0;
}

int NewCmd::run()
{
    if (this->at.has_value())
        fs::current_path(this->at.value());
    auto project_path = fs::current_path() / this->name;
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

    std::ofstream main_cpp(project_path / "bin" / (this->name + ".cpp"));
    main_cpp << "#include <iostream>" << std::endl;
    main_cpp << "int main() {" << std::endl;
    main_cpp << "    std::cout << \"Hello, world!\" << std::endl;" << std::endl;
    main_cpp << "    return 0;" << std::endl;
    main_cpp << "}" << std::endl;
    main_cpp.close();

    std::ofstream gitignore(project_path / ".gitignore");
    gitignore << "/build" << std::endl;
    gitignore << "/target" << std::endl;
    gitignore.close();
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
    std::ofstream ofs(dir / "CMakeLists.txt");
    build.generate_build(ofs);
    ofs.close();
    return build.build();
}