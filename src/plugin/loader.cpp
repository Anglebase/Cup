#include "plugin/loader.h"

plugin::Loader::Loader(const fs::path &path)
{
#ifdef _WIN32
    HMODULE handle = LoadLibraryA(path.string().c_str());
    if (!handle)
    {
        throw std::runtime_error("Failed to load library: " + path.string());
    }
#else
    void *handle = dlopen(path.string().c_str(), RTLD_NOW);
    if (!handle)
    {
        throw std::runtime_error("Failed to load library: " + path.string());
    }
#endif
    this->handle_ = handle;
}

plugin::Loader::~Loader()
{
#ifdef _WIN32
    FreeLibrary(this->handle_);
#else
    dlclose(this->handle_);
#endif
}

void *plugin::Loader::get_symbol(const std::string &name)
{
    void *symbol = nullptr;
#ifdef _WIN32
    symbol = GetProcAddress(this->handle_, name.c_str());
#else
    symbol = dlsym(this->handle_, name.c_str());
#endif
    if (!symbol)
    {
        throw std::runtime_error("Failed to get symbol: " + name);
    }
    return symbol;
}
