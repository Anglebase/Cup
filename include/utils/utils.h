#pragma once

#include <vector>
#include <string>

std::vector<std::string> split(const std::string& str, const std::string& delimiter);
std::string join(const std::vector<std::string>& strs, const std::string& delimiter);