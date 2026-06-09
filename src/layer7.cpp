#include "../include/layer7.h"
Layer7::Layer7(std::vector<std::string> &paths, std::vector<std::vector<std::pair<std::string, int>>> details)
{
    int len = paths.size();
    for (int i = 0; i < len; i++)
    {
        int n = details[i].size();
        for (int j = 0; j < n; j++)
            this->pathToDetails[paths[i]].ipPortList.push_back(details[i][j]);
    }
};

std::pair<std::string, int> Layer7::nextPort(const std::string &path)
{
    std::pair<std::string, int> toReturn = {"", -1};
    int len = pathToDetails[path].ipPortList.size();
    if (len == 0)
    {
        return toReturn;
    }
    size_t ind = pathToDetails[path].next_ind.fetch_add(1);
    toReturn = pathToDetails[path].ipPortList[ind % len];
    return toReturn;
}

bool Layer7::Layer7check(const std::string &path)
{

    if (this->pathToDetails.find(path) != this->pathToDetails.end())
        return 1;
    return 0;
}
