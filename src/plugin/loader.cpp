#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include "plugin/loader.h"
#include "plugin/built-in/binary.h"
#include "plugin/built-in/shared.h"
#include "plugin/built-in/static.h"
#include "plugin/built-in/module.h"
#include "res.h"

PluginLoader::PluginLoader(const std::string &type)
{
    std::unordered_map<std::string, std::function<IPlugin *()>> built_in_plugins = {
        {
            "binary",
            []
            { return new BinaryPlugin(); },
        },
        {
            "shared",
            []
            { return new SharedPlugin(); },
        },
        {
            "static",
            []
            { return new StaticPlugin(); },
        },
        {
            "module",
            []
            { return new ModulePlugin(); },
        },
    };
    if (!built_in_plugins.contains(type))
    {
        auto path = (Resource::home() / "plugins" /
                     (type +
#ifdef _WIN32
                      ".dll"
#else
                      ".so"
#endif
                      ))
                        .lexically_normal();
        if (!fs::exists(path))
            throw std::runtime_error("Failed to find plugin '" + type +
                                     "' at: " + path.parent_path().string());
        if (!DLL_LOAD(this->dll, path.string()))
            throw std::runtime_error("Failed to load plugin '" + type +
                                     "' at: " + path.string());
        if (!DLL_GET_FUNC(this->dll, createPlugin))
            throw std::runtime_error("Failed to get 'createPlugin' function from plugin '" +
                                     type + "'");
        if (!DLL_GET_FUNC(this->dll, destroyPlugin))
            throw std::runtime_error("Failed to get 'destroyPlugin' function from plugin '" +
                                     type + "'");
        this->plugin = this->createPlugin();
    }
    else
    {
        this->plugin = built_in_plugins[type]();
        this->dll = nullptr;
        this->destroyPlugin = [](IPlugin *plugin)
        { delete plugin; };
    }
}

PluginLoader::~PluginLoader()
{
    this->destroyPlugin(this->plugin);
    if (this->dll)
        DLL_UNLOAD(this->dll);
}