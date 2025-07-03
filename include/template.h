#pragma once

#include <string>
#include <unordered_map>

class Template
{
public:
    virtual std::string getTemplate() = 0;
    virtual std::string getContent() = 0;
    std::string replace(const std::unordered_map<std::string, std::string> &replaceMap);
};

class FileTemplate : public Template
{
    std::string content;
    std::unordered_map<std::string, std::string> replace_map;

public:
    FileTemplate(const std::string &content, const std::unordered_map<std::string, std::string> &raplace_map);
    std::string getTemplate() override;
    std::string getContent() override;
};