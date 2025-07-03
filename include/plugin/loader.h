#pragma once

#ifdef _WIN32
#include <Windows.h>
using DLLPtr = HMODULE;
#define DLL_LOAD(dll, path) (dll = LoadLibrary(path.c_str()))
#define DLL_UNLOAD(dll) FreeLibrary(dll)
#define DLL_GET_FUNC(dll, func) (func = (decltype(func))GetProcAddress(dll, #func))
#else
#include <dlfcn.h>
using DLLPtr = void *;
#define DLL_LOAD(dll, path) (dll = dlopen(path.c_str(), RTLD_NOW))
#define DLL_UNLOAD(dll) dlclose(dll)
#define DLL_GET_FUNC(dll, func) (func = (decltype(func))dlsym(dll, #func))
#endif
#include <string>
#include "plugin/interface.h"

class PluginLoader
{
    using CreatePluginFunc = IPlugin *(*)();
    using DestoryPluginFunc = void (*)(IPlugin *);
    DLLPtr dll{nullptr};
    IPlugin *plugin{nullptr};
    CreatePluginFunc createPlugin{nullptr};
    DestoryPluginFunc destroyPlugin{nullptr};

public:
    PluginLoader(const std::string &type);
    ~PluginLoader();

    IPlugin *operator->() { return plugin; }
};