#pragma once

#include "toml++/toml.hpp"
#include "utils.h"
#include "log.h"
#include <filesystem>
namespace fs = std::filesystem;

class Config
{
    toml::table table_;
    fs::path path;
#ifdef DEBUG_CONFIG
public:
#endif
    // 断言配置项必定存在并获取，若不存在则抛出异常
    template <class T>
    T get(const std::string &key_dir, std::string_view error_message_view = "") const
    {
        std::string error_message;
        if (error_message_view.empty())
        {
            std::ostringstream oss;
            oss << "Error: Unable to find configuration item: '" << key_dir
                << "' in file: '" << this->path << "'.";
            error_message = oss.str();
        }
        else
            error_message = std::string(error_message_view);
        auto keys = split(key_dir, ".");
        auto table_keys = std::vector<std::string>(keys.begin(), keys.end() - 1);
        auto table = &this->table_;
        for (auto key : table_keys)
        {
            if (!table->contains(key) || !table->at(key).is_table())
                throw std::runtime_error(error_message);
            table = table->at(key).as_table();
        }
        if (!table->contains(keys.back()))
            throw std::runtime_error(error_message);
        toml::v3::optional<T> value = table->at(keys.back()).value<T>();
        if (!value.has_value())
            throw std::runtime_error(error_message);
        return value.value();
    }

    // 查询配置项是否存在，若不存在则返回默认值
    template <class T>
    T need(const std::string &key_dir, T default_, bool show_warning = true, std::string_view warning_message_view = "") const
    {
        std::string warning_message;
        if (warning_message_view.empty())
        {
            std::ostringstream oss;
            oss << "Warning: Unable to find configuration item: '" << key_dir
                << "' in file: '" << this->path
                << "'. It will use the default value: '" << default_ << "'.";
            warning_message = oss.str();
        }
        else
            warning_message = std::string(warning_message_view);
        auto keys = split(key_dir, ".");
        auto table_keys = std::vector<std::string>(keys.begin(), keys.end() - 1);
        auto table = &this->table_;
        for (auto key : table_keys)
        {
            if (!table->contains(key) || !table->at(key).is_table())
            {
                if (show_warning)
                    LOG_WARN(warning_message);
                return default_;
            }
            table = table->at(key).as_table();
        }
        if (!table->contains(keys.back()))
        {
            if (show_warning)
                LOG_WARN(warning_message);
            return default_;
        }
        toml::v3::optional<T> value = table->at(keys.back()).value<T>();
        if (!value.has_value())
        {
            if (show_warning)
                LOG_WARN(warning_message);
            return default_;
        }
        return value.value();
    }

public:
    Config(const fs::path &project);
};