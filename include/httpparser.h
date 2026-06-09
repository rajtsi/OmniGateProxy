#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

class HttpRequest
{
public:
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    HttpRequest(const char *raw_buffer);
};
