#pragma once
#include <vector>
#include <string>
#include <utility>
#include "thread_pool.h"
#include "layer7.h"
#include "httpparser.h"

class OmniGate
{
private:
    // Server Configuration
    int port;
    int worker_threads;
    bool is_layer7_enabled;

    // Layer 4 Variables
    std::vector<int> backend_ports;
    size_t current_index = 0;

    // Components
    ThreadPool *pool = nullptr;
    Layer7 *layer7_balancer = nullptr;

    // Static members for Signal Handling
    static bool start_flag;
    static int proxy_fd;

    // Singleton Setup
    OmniGate() = default;

    OmniGate(const OmniGate &) = delete;
    OmniGate &operator=(const OmniGate &) = delete;

    // Internal Methods
    std::pair<std::string, int> get_backend_target(const std::string &path);
    void handle_client(int client_fd);

public:
    static OmniGate &getInstance()
    {
        static OmniGate instance;
        return instance;
    }

    // Destructor
    ~OmniGate();

    // Public API
    void load_config(const std::string &filepath);
    void start_server();

    static void server_handler(int sigint);
};




