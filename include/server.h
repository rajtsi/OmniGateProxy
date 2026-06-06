#pragma once
#include <vector>
#include "thread_pool.h"
class OmniGate
{

    int port;
    ThreadPool *pool;
    std::vector<int> backend_ports;
    int current_index;
    void handle_client(int client_fd);
    int getNextPort();

public:
    void start_server();
    OmniGate(int port, std::vector<int> vt, int thread_count);
    ~OmniGate();
};