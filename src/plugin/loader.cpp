#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include "plugin/loader.h"
#include "plugin/built-in/binary.h"
#include "plugin/built-in/shared.h"
#include "plugin/built-in/static.h"
#include "plugin/built-in/module.h"
#include "plugin/built-in/interface.h"
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
        {
            "interface",
            []
            { return new InterfacePlugin(); },
        },
    };
    if (!built_in_plugins.contains(type))
    {
        auto path = (Resource::cup() / "plugins" /
                     (type +
#ifdef _WIN32
                      ".dll"
#else
                      ".so"
#endif
                      ))
                        .lexically_normal();
        try
        {
            if (!fs::exists(path))
                throw std::runtime_error("Plugin '" + type + "' not installed.");
            if (!DLL_LOAD(this->dll, path.string()))
                throw std::runtime_error("Plugin '" + type + "' failed to load.");
            if (!DLL_GET_FUNC(this->dll, createPlugin) || !DLL_GET_FUNC(this->dll, destroyPlugin))
                throw std::runtime_error("Plugin '" + type + "' not a valid plugin.");
        }
        catch (const std::exception &e)
        {
            DLL_UNLOAD(this->dll);
            throw e;
        }
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