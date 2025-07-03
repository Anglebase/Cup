#include "template.h"
#include "log.h"

std::string Template::replace(const std::unordered_map<std::string, std::string> &replaceMap)
{
    std::string result = this->getTemplate();
    for (const auto &[key, value] : replaceMap)
    {
        auto replaceKey = "${%" + key + "%}";
        size_t pos = 0;
        while ((pos = result.find(replaceKey)) != std::string::npos)
            result.replace(pos, replaceKey.length(), value);
    }
    return result;
}

FileTemplate::FileTemplate(
    const std::string &content,
    const std::unordered_map<std::string, std::string> &replace_map)
    : content(content), replace_map(replace_map) {}

std::string FileTemplate::getTemplate()
{
    return this->content;
}

std::string FileTemplate::getContent()
{
    return this->replace(this->replace_map);
}