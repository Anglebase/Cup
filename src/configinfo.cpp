#include "configinfo.h"
#include "config.h"
#include <unordered_set>
#include "tools.h"
#include "git.h"

ConfigInfo::ConfigInfo(const Config &config)
{
    *this = Deserde<ConfigInfo>::from(config.table_);
    auto iter = this->dependencies.end();
    for (auto dep = this->dependencies.begin(); dep != this->dependencies.end(); ++dep)
    {
        auto &[name, dependency] = *dep;
        if (name.starts_with("Qt"))
        {
            if (this->qt.has_value())
                throw std::runtime_error("Multiple Qt dependencies found.");
            std::vector<std::string> flags;
            std::vector<std::string> modules;
            for (auto &feature : dependency.features)
            {
                static const std::unordered_set<std::string> flags_set{"AUTORCC", "AUTOUIC", "AUTOMOC"};
                if (flags_set.find(feature) != flags_set.end())
                    flags.push_back(feature);
                else
                    modules.push_back(feature);
            }
            this->qt = Qt{
                .path = dependency.path,
                .version = name,
                .modules = std::move(modules),
                .flags = std::move(flags),
            };
            iter = dep;
        }
    }
    if (iter != this->dependencies.end())
        this->dependencies.erase(iter);
    for (auto &[name, dependency] : this->dependencies)
    {
        if (!dependency.path.has_value())
            throw std::runtime_error("Connot finc dependency '" + name + "' because it has no path.");
    }
}

fs::path Dependency::get_path() const
{
    if (this->path.has_value())
        return *this->path;
    if (!this->git)
        throw std::runtime_error("Invalid dependency '" + name + "'.");
    std::string url;
    if (this->git->starts_with("@"))
    {
        auto package_ = this->git->substr(1);
        auto info = split(package_, "/");
        if (info.size() != 2)
            throw std::runtime_error("Invalid package name: " + *this->git);
        auto author = info[0];
        auto libary = info[1];
        url = "https://github.com/" + author + "/" + libary + ".git";
    }
    else
    {
        url = *this->git;
    }
    auto git = Git{};
    std::string tag;
    if (!this->version)
    {
        auto tags = git.get_tags(url);
        if (tags.empty())
            throw std::runtime_error("Cannot find any tags for dependency '" + name + "'.");
        tag = tags.back();
    }
    else
    {
        tag = "v" + *this->version;
    }
    const auto &[author, library] = get_author_libary(url);
    const auto cup_dir = get_user_dir() / ".cup";
    const auto dir = cup_dir / author / library / tag;
    if (fs::exists(dir))
        return dir;
    if (BuildCmd::auto_download)
    {
        LOG_MSG("Installing dependency '" + name + "' from git repository '" +
                url + "' with tag '" + tag + "'.");
        git.download(url, tag);
        LOG_MSG("Dependency '" + name + "' installed.");
    }
    else
    {
        throw std::runtime_error("Cannot find dependency '" + name + "'.");
    }
    return dir;
}
