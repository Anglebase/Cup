#include "configinfo.h"
#include "config.h"
#include <unordered_set>

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
}