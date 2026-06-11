#include "../include/ProxyServer.h"
#include "../include/httpparser.h"
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <csignal>
#include <fstream>
#include "../include/json.hpp"

using json = nlohmann::json;
using namespace std;

std::mutex l4_mtx;
bool OmniGate::start_flag = true;
int OmniGate::proxy_fd = -1;

OmniGate::~OmniGate()
{
    if (this->pool)
        delete this->pool;
    if (this->layer7_balancer)
        delete this->layer7_balancer;
}

void OmniGate::server_handler(int sigint)
{
    OmniGate::start_flag = false;
    if (OmniGate::proxy_fd != -1)
    {
        shutdown(OmniGate::proxy_fd, SHUT_RD);
        close(OmniGate::proxy_fd);
    }
}

std::pair<std::string, int> OmniGate::get_backend_target(const std::string &path)
{
    if (this->is_layer7_enabled && this->layer7_balancer)
    {

        if (this->layer7_balancer->Layer7check(path))
        {
            auto target = this->layer7_balancer->nextPort(path);

            if (target.second != -1)
            {
                return target;
            }
            else
            {

                std::cout << "CRITICAL: All Layer 7 servers for path " << path << " are dead!" << std::endl;
                return {"", -2};
            }
        }
    }

    // Layer 4
    l4_mtx.lock();
    int b_port = backend_ports[current_index];
    current_index = (current_index + 1) % backend_ports.size();
    l4_mtx.unlock();

    return {"127.0.0.1", b_port};
}

void OmniGate::handle_client(int client_fd)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(client_fd, (struct sockaddr *)&client_addr, &addr_len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

    char buffer[4096];
    int rec_bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (rec_bytes <= 0)
    {
        close(client_fd);
        return;
    }

    std::string req_path = "";
    std::string raw_request(buffer, rec_bytes);

    if (this->is_layer7_enabled)
    {
        buffer[rec_bytes] = '\0';
        HttpRequest req(buffer);
        req_path = req.path;

        std::cout << "\n--- Layer 7 Request Details ---" << std::endl;
        std::cout << "Path: " << req_path << std::endl;

        size_t header_end = raw_request.find("\r\n\r\n");
        if (header_end != std::string::npos)
        {
            std::string header_to_inject = "X-Forwarded-For: " + std::string(client_ip) + "\r\n";
            raw_request.insert(header_end + 2, header_to_inject);
        }
    }

    std::cout << "\n--- Request Details ---" << std::endl;
    std::cout << "Path: " << req_path << std::endl;
    std::cout << "Real Client IP: " << client_ip << std::endl;

    auto target = get_backend_target(req_path);

    if (target.second == -2)
    {
        std::cout << "Service Unavailable for path: " << req_path << std::endl;
        std::string response = "HTTP/1.1 503 Service Unavailable\r\nConnection: close\r\n\r\n<h1>503: Service Unavailable</h1>";
        send(client_fd, response.c_str(), response.length(), 0);
        close(client_fd);
        return;
    }

    std::cout << "Routing to -> " << target.first << ":" << target.second << std::endl;

    int backend_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in backend_config;
    memset(&backend_config, 0, sizeof(backend_config));
    backend_config.sin_family = AF_INET;

    inet_pton(AF_INET, target.first.c_str(), &backend_config.sin_addr);
    backend_config.sin_port = htons(target.second);

    if (connect(backend_fd, (struct sockaddr *)&backend_config, sizeof(backend_config)) < 0)
    {
        std::cout << "Backend Connection Failed!" << std::endl;
        string response = "HTTP/1.1 502 Bad Gateway\r\nConnection: close\r\n\r\n<h1>502: Bad Gateway</h1>";
        send(client_fd, response.c_str(), response.length(), 0);
        close(backend_fd);
        close(client_fd);
        return;
    }

    send(backend_fd, raw_request.c_str(), raw_request.length(), 0);

    while (true)
    {
        char backend_buffer[4096];
        int backend_res = recv(backend_fd, backend_buffer, sizeof(backend_buffer) - 1, 0);
        if (backend_res <= 0)
            break;
        send(client_fd, backend_buffer, backend_res, 0);
    }

    close(backend_fd);
    close(client_fd);
}

void OmniGate::load_config(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Config file (" << filepath << ") nahi mili bhaisahab!" << std::endl;
        exit(1);
    }

    json config = json::parse(file);

    this->port = config["server_port"];
    this->worker_threads = config["worker_threads"];
    this->is_layer7_enabled = config["enable_layer7"];
    this->pool = new ThreadPool(this->worker_threads);

    std::vector<int> l4_ports = config["layer4_ports"];
    this->backend_ports = l4_ports;
    this->current_index = 0;

    if (this->is_layer7_enabled)
    {
        std::cout << "Starting in Layer 7 (Smart Routing) Mode..." << std::endl;
        std::vector<std::string> paths;
        std::vector<std::vector<std::pair<std::string, int>>> details;

        for (auto &[path, servers] : config["layer7_routes"].items())
        {
            paths.push_back(path);
            std::vector<std::pair<std::string, int>> server_list;
            for (auto &s : servers)
            {
                server_list.push_back({s["ip"], s["port"]});
            }
            details.push_back(server_list);
        }
        this->layer7_balancer = new Layer7(paths, details);
    }
}

void OmniGate::start_server()
{
    std::signal(SIGINT, server_handler);
    OmniGate::proxy_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (OmniGate::proxy_fd == -1)
    {
        perror("Failed to create proxy server socket");
        return;
    }

    struct sockaddr_in listner_config;
    memset(&listner_config, 0, sizeof(listner_config));
    listner_config.sin_family = AF_INET;
    listner_config.sin_port = htons(this->port);
    listner_config.sin_addr.s_addr = INADDR_ANY;

    if (::bind(OmniGate::proxy_fd, (struct sockaddr *)&listner_config, sizeof(listner_config)) == -1)
    {
        perror("Bind failed");
        close(OmniGate::proxy_fd);
        return;
    }

    if (listen(OmniGate::proxy_fd, 10) == -1)
    {
        perror("Listen failed");
        close(OmniGate::proxy_fd);
        return;
    }

    cout << "OmniGate Proxy is successfully running on port " << this->port << endl;

    while (OmniGate::start_flag)
    {
        int client_fd = accept(OmniGate::proxy_fd, nullptr, nullptr);

        if (!OmniGate::start_flag)
        {
            if (client_fd > 0)
                close(client_fd);
            break;
        }

        if (client_fd < 0)
            continue;

        pool->add_job([this, client_fd]
                      { this->handle_client(client_fd); });
    }
    close(OmniGate::proxy_fd);
}