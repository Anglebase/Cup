#pragma once

class Git
{
public:
    Git();

    std::vector<std::string> get_tags(const std::string &url);
    void download(const std::string &url, const std::string &tag);
};