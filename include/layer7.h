#pragma once
#include <unordered_map>
#include <stdio.h>
#include <vector>
#include <atomic>
struct config
{
    std::atomic<size_t> next_ind{0};
    std::vector<std::pair<std::string, int>> ipPortList;
};

class Layer7
{
public:
    std::unordered_map<std::string, struct config> pathToDetails;
    Layer7(std::vector<std::string> &paths, std::vector<std::vector<std::pair<std::string, int>>> details);
    std::pair<std::string, int> nextPort(const std::string &path);
    bool Layer7check(const std::string &path);
};