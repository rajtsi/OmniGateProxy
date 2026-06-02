#pragma once
#include <vector>
class OmniGate
{

    int port;
    std::vector<int> backend_ports;
    int current_index;
    void handel_client(int client_fd);
    int getNextPort();

public:
    void start_server();
    OmniGate(int port, std::vector<int> vt);
};