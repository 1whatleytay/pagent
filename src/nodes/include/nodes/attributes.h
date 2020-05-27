#pragma once

#include <langutil/parser.h>

#include <string>
#include <vector>
#include <unordered_map>

using langutil::Parser;

std::unordered_map<std::string, bool> parseAttributes(Parser &parser, const std::vector<std::string> &names);
