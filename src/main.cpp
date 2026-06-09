#include "../include/server.h"
#include <iostream>

int main(int argc, char *argv[])
{
    std::string config_path = (argc > 1) ? argv[1] : "../config.json";
    OmniGate &myserver = OmniGate::getInstance();
    myserver.load_config(config_path);
    myserver.start_server();

    return 0;
}