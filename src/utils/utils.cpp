#include "utils/utils.h"
#include <fstream>

std::vector<std::string> split(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> result;
    size_t pos = 0, last = 0;
    while ((pos = str.find(delimiter, pos)) != std::string::npos)
    {
        result.push_back(str.substr(last, pos - last));
        last = pos + delimiter.length();
        pos += delimiter.length();
    }
    result.push_back(str.substr(last));
    return result;
}

std::string join(const std::vector<std::string> &strs, const std::string &delimiter)
{
    std::string result;
    for (size_t i = 0; i < strs.size(); i++)
    {
        result += strs[i];
        if (i != strs.size() - 1)
        {
            result += delimiter;
        }
    }
    return result;
}

std::string read_file(const fs::path &file_path)
{
    std::vector<std::string> lines;
    std::ifstream file(file_path);
    std::string line;
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }
    return join(lines, "\n");
}

std::string replace(const std::string &str, const std::string &old_str, const std::string &new_str)
{
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(old_str, pos)) != std::string::npos)
    {
        result.replace(pos, old_str.length(), new_str);
        pos += new_str.length();
    }
    return result;
}