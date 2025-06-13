#pragma once

#include <string>
#include <vector>
#include <utility>

class Git
{
public:
    Git();

    std::vector<std::string> get_tags(const std::string &url);
    void download(const std::string &url, const std::string &tag);
};

std::pair<std::string, std::string> get_author_libary(const std::string &url);