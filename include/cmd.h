#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

class SysArgs
{
    std::vector<std::string> args;

    std::vector<std::string> arguments;
    std::unordered_set<std::string> flags;
    std::unordered_map<std::string, std::vector<std::string>> configs;
public:
    SysArgs(int argc, char **argv);
    const std::vector<std::string>& getArguments() const;
    const std::unordered_set<std::string>& getFlags() const;
    const std::unordered_map<std::string, std::vector<std::string>>& getConfigs() const;

    bool hasFlag(const std::string& flag) const;
    bool hasConfig(const std::string& config) const;
};
