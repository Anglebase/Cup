#include "tools.h"
#include "build.h"
#include <fstream>
#include "git.h"

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
    LOG_INFO("Create new project: ", this->name);
}

CleanCmd::CleanCmd(const SysArgs &args)
{
    if (args.hasConfig("dir") && !args.getConfigs().at("dir").empty())
        this->at = args.getConfigs().at("dir")[0];
    LOG_INFO("Begin Clean...");
}

bool BuildCmd::auto_download = false;
BuildCmd::BuildCmd(const SysArgs &args)
    : args(args)
{
    if (args.hasConfig("dir") && !args.getConfigs().at("dir").empty())
        this->at = args.getConfigs().at("dir")[0];

    this->project_dir = this->at.value_or(fs::current_path());
    if (this->project_dir.is_relative())
        this->project_dir = fs::current_path() / this->project_dir;
    this->project_dir = project_dir.lexically_normal();
    this->config = args.hasFlag("r") || args.hasFlag("release")
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
    if (args.hasFlag("auto"))
        BuildCmd::auto_download = true;
}

RunCmd::RunCmd(const SysArgs &args) : BuildCmd(args)
{
    if (args.hasConfig("args"))
        this->run_args = args.getConfigs().at("args");
    if (args.getArguments().size() <= 1)
        throw std::runtime_error("No target provided.");
    auto target = args.getArguments()[1];
    auto dir = this->target_dir / target;
    dir.replace_extension();
    this->run_target = dir.lexically_normal();
}

std::optional<std::string> RunCmd::suffix = std::nullopt;
int RunCmd::exec()
{
    auto res = 0;
    if ((res = this->run()) != 0)
        return res;
    if (RunCmd::suffix.has_value())
    {
        this->run_target.replace_extension(RunCmd::suffix.value());
    }
    else
    {
#ifdef _WIN32
        this->run_target.replace_extension(".exe");
#endif
    }
    if (!fs::exists(this->run_target))
        this->run_target = this->run_target.parent_path() /
                           (this->config == BuildType::Release ? "Release" : "Debug") /
                           this->run_target.filename();
    auto dir = this->at.value_or(fs::current_path());
    if (dir.is_relative())
        dir = fs::current_path() / dir;
    auto dll_at = dir / "build" / "lib";
    if (fs::exists(dll_at))
        for (auto &dll : fs::directory_iterator(dll_at))
            if (dll.is_regular_file() && dll.path().extension() == ".dll")
                fs::copy_file(dll.path(), this->run_target.parent_path() / dll.path().filename(),
                              fs::copy_options::overwrite_existing);
    dll_at = dll_at / (this->config == BuildType::Release ? "Release" : "Debug");
    if (fs::exists(dll_at))
        for (auto &dll : fs::directory_iterator(dll_at))
            if (dll.is_regular_file() && dll.path().extension() == ".dll")
                fs::copy_file(dll.path(), this->run_target.parent_path() / dll.path().filename(),
                              fs::copy_options::overwrite_existing);
    LOG_INFO("\033[32mRun: ", this->run_target.string(), "\033[0m");
    std::cout << std::endl;
    res = std::system((this->run_target.string() + " " + join(this->run_args, " ")).c_str());
    std::cout << std::endl;
    std::ostringstream oss;
    oss << std::hex << res;
    LOG_INFO(res == 0 ? "\033[32m" : "\033[33m", "Exit Code: ", res, " (0x", oss.str(), ")", "\033[0m");
    return 0;
}

int NewCmd::run()
{
    if (this->at.has_value())
        fs::current_path(this->at.value());
    auto project_path = fs::current_path() / this->name;
    if (fs::exists(project_path))
        throw std::runtime_error("Project already exists: " + project_path.string());
    std::vector<std::string> sub_dir = {"include", "bin"};
    if (this->target != "header")
        sub_dir.push_back("src");
    for (auto &dir : sub_dir)
        fs::create_directories(project_path / dir);
    LOG_INFO("Generator template project...");
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
    else if (target == "header")
    {
        std::ofstream lib_h(project_path / "include" / (this->name + ".h"));
        lib_h << "#pragma once" << std::endl
              << std::endl;
        lib_h << "#include <iostream>" << std::endl
              << std::endl;
        lib_h << "inline void " << this->name << "()" << std::endl;
        lib_h << "{" << std::endl;
        lib_h << "    std::cout << \"" << this->name << "() called!\" << std::endl;" << std::endl;
        lib_h << "}" << std::endl;
        lib_h.close();

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
    LOG_INFO("Finised.");
    return 0;
}

ListCmd::ListCmd(const SysArgs &args)
{
    if (args.hasConfig("dir") && !args.getConfigs().at("dir").empty())
        this->at = args.getConfigs().at("dir")[0];
    if (args.getArguments().size() <= 1)
        throw std::runtime_error("No option provided.");
    this->option = args.getArguments()[1];
}

void load_all_includes(std::unordered_set<std::string> &includes, const fs::path &root)
{
    includes.insert((root / "include").lexically_normal().string());
    Config config(root);
    for (const auto &include : config.config->build.include)
    {
        if (include.is_relative())
            includes.insert((root / include).lexically_normal().string());
        else
            includes.insert(include.lexically_normal().string());
    }
    for (const auto &[name, path] : config.config->dependencies)
    {
        auto dir = path.get_path();
        dir.lexically_normal();
        load_all_includes(includes, dir);
    }
}

Version compare(Version a, Version b)
{
    if (a.x != b.x)
        throw std::runtime_error("Simultaneously having incompatible versions is not allowed!");
    if (a.y != b.y)
        return a.y < b.y ? b : a;
    if (a.z != b.z)
        return a.z < b.z ? b : a;
    return a;
}

void load_all_dependecies(std::unordered_map<std::string, Version> &dependencies, const fs::path &root)
{
    Config config(root);
    if (dependencies.find(config.config->name) == dependencies.end())
        dependencies[config.config->name] = config.config->version;
    else
    {
        auto result = compare(dependencies[config.config->name], config.config->version);
        if (result == config.config->version)
            return;
        dependencies[config.config->name] = result;
    }
    for (const auto &[name, path] : config.config->dependencies)
    {
        auto dir = path.get_path();
        dir.lexically_normal();
        load_all_dependecies(dependencies, dir);
    }
}

int ListCmd::run()
{
    auto project_dir = this->at.value_or(fs::current_path());
    if (project_dir.is_relative())
        project_dir = fs::current_path() / project_dir;
    project_dir = project_dir.lexically_normal();
    if (this->option == "include")
    {
        LOG_INFO("Include directories:");
        std::unordered_set<std::string> includes;
        load_all_includes(includes, project_dir);
        for (const auto &include : includes)
            std::cout << "    " << include << std::endl;
    }
    else if (this->option == "deps")
    {
        LOG_INFO("Dependencies:");
        std::unordered_map<std::string, Version> dependencies;
        load_all_dependecies(dependencies, project_dir);
        for (const auto &[name, version] : dependencies)
            std::cout << "    " << name << " v" << version << std::endl;
    }
    else if (this->option == "installed")
    {
        LOG_MSG("Installed packages:");
        auto cup_dir = get_user_dir() / ".cup";
        for (const auto &author_dir : fs::directory_iterator(cup_dir))
        {
            if (!author_dir.is_directory())
                continue;
            const auto &author = author_dir.path().filename().string();
            for (const auto &libary_dir : fs::directory_iterator(author_dir))
            {
                if (!libary_dir.is_directory())
                    continue;
                const auto &libary = libary_dir.path().filename().string();
                for (const auto &tag_dir : fs::directory_iterator(libary_dir))
                {
                    if (!tag_dir.is_directory())
                        continue;
                    const auto &version = tag_dir.path().filename().string();
                    std::cout << "    @" << author << "/" << libary << "\t" << version << std::endl;
                }
            }
        }
    }
    return 0;
}

InstallCmd::InstallCmd(const SysArgs &args)
{
    if (args.getArguments().size() <= 1)
        throw std::runtime_error("No package name provided.");
    auto package = args.getArguments()[1];
    if (package.starts_with("@"))
    {
        auto package_ = package.substr(1);
        auto info = split(package_, "/");
        if (info.size() != 2)
            throw std::runtime_error("Invalid package name: " + package);
        auto author = info[0];
        auto libary = info[1];
        this->url = "https://github.com/" + author + "/" + libary + ".git";
    }
    else
    {
        this->url = package;
    }
    if (args.hasConfig("version") && !args.getConfigs().at("version").empty())
        this->version = args.getConfigs().at("version")[0];
}

int InstallCmd::run()
{
    auto git = Git{};
    auto tags = git.get_tags(this->url);
    if (tags.empty())
        throw std::runtime_error("No tags found in repository: " + this->url);
    if (!this->version)
    {
        this->version = tags.back().substr(1);
        LOG_MSG("Not specified version, use latest version: ", *this->version);
    }
    else
    {
        LOG_DEBUG("Specified version: ", *this->version);
        if (std::find(tags.begin(), tags.end(), "v" + *this->version) == tags.end())
            throw std::runtime_error("Connot find version: " + *this->version);
    }
    LOG_MSG("Installing package: ", this->url, " v", *this->version);
    git.download(this->url, "v" + *this->version);
    LOG_MSG("Package installed.");
    return 0;
}

UninstallCmd::UninstallCmd(const SysArgs &args)
{
    if (args.getArguments().size() <= 1)
        throw std::runtime_error("No package name provided.");
    auto package = args.getArguments()[1];
    if (package.starts_with("@"))
    {
        auto package_ = package.substr(1);
        auto info = split(package_, "/");
        if (info.size() != 2)
            throw std::runtime_error("Invalid package name: " + package);
        author = info[0];
        libary = info[1];
    }
    else
    {
        const auto &[author_, libary_] = get_author_libary(package);
        author = author_;
        libary = libary_;
    }
    if (args.hasConfig("version") && !args.getConfigs().at("version").empty())
        this->version = args.getConfigs().at("version")[0];
}

int UninstallCmd::run()
{
    auto cup_dir = get_user_dir() / ".cup";
    auto package_dir = cup_dir / author / libary;
    if (this->version.has_value())
    {
        package_dir /= "v" + *this->version;
        if (fs::exists(package_dir))
        {
            fs::remove_all(package_dir);
            LOG_INFO("Package @" + author + "/" + libary + " is uninstalled.");
        }
        else
            LOG_INFO("Package @" + author + "/" + libary + " v" + *this->version + " is not installed.");
        return 0;
    }
    if (!fs::exists(package_dir))
    {
        LOG_INFO("Package @" + author + "/" + libary + " is not installed.");
        return 0;
    }
    LOG_WARN("Are you sure to uninstall all versions of package @" + author + "/" + libary + "? (y/N)");
    std::string input;
    std::getline(std::cin, input);
    if (input == "y" || input == "Y")
    {
        fs::remove_all(package_dir);
        LOG_INFO("Package @" + author + "/" + libary + " is uninstalled.");
    }
    return 0;
}

int CleanCmd::run()
{
    if (this->at.has_value())
        fs::current_path(this->at.value());
    bool finished = false;
    if (fs::exists("build"))
    {
        LOG_INFO("Cleaning build directory...");
        fs::remove_all("build");
        finished = true;
    }
    finished
        ? LOG_INFO("Finised.")
        : LOG_INFO("Nothing to clean.");
    return 0;
}

int BuildCmd::run()
{
    auto build_info = BuildInfo(project_dir, this->args);
    auto config_info = ConfigInfo(Config(project_dir));
    auto build = Build(build_info, config_info);
    if (!fs::exists(build_info.build_dir))
        fs::create_directories(build_info.build_dir);
    LOG_INFO("Generating cmake...");
    std::ofstream ofs(build_info.build_dir / "CMakeLists.txt");
    build.generate_build(ofs);
    ofs.close();
    LOG_INFO("Building...");
    return build.build();
}