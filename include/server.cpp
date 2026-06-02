#include "../include/server.h"
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
using namespace std;
std::mutex mtx;
OmniGate::OmniGate(int port, vector<int> vt)
{
    this->port = port;
    this->current_index = 0;
    this->backend_ports = vt;
}

int OmniGate::getNextPort()
{

    mtx.lock();
    int port = backend_ports[current_index];
    current_index++;
    current_index %= 3;
    mtx.unlock();
    return port;
}

void OmniGate::handel_client(int client_fd)
{

    char buffer[4096];
    int rec_bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (rec_bytes > 0)
    {
        int backend_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (backend_fd < 0)
        {
            cout << "unable to connect to backend end server";
            close(backend_fd);
            return;
        }

        struct sockaddr_in backend_config;
        memset(&backend_config, 0, sizeof(backend_config));
        backend_config.sin_family = AF_INET;
        inet_pton(AF_INET, "127.0.0.1", &backend_config.sin_addr);
        int initial_port = getNextPort();
        backend_config.sin_port = htons(initial_port);
        int backend_connect = connect(backend_fd, (struct sockaddr *)&backend_config, sizeof(backend_config));

        while (backend_connect < 0)
        {
            close(backend_fd);
            int new_port = getNextPort();
            if (new_port == initial_port)
            {
                cout << "backend connection failed ";
                cout << "backend connection failed";
                string response = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/html\r\n"
                                  "Connection: close\r\n\r\n"
                                  "<h1>Unable to connect to any servers!</h1>"
                                  "<p> This response is from  load Ballancer</p>";
                send(client_fd, response.c_str(), response.length(), 0);
                close(backend_fd);
                close(client_fd);
                return;
            }
            backend_fd = socket(AF_INET, SOCK_STREAM, 0);
            backend_config.sin_port = htons(new_port);
            backend_connect = connect(backend_fd, (struct sockaddr *)&backend_config, sizeof(backend_config));
        }

        buffer[rec_bytes] = '\0';
        cout << " Receaved " << buffer << "buffer recieved from browser ends here" << endl;
        send(backend_fd, buffer, rec_bytes, 0);
        while (true)
        {
            char backend_buffer[4096];
            int backend_res = recv(backend_fd, backend_buffer, sizeof(backend_buffer) - 1, 0);
            if (backend_res <= 0)
                break;
            backend_buffer[backend_res] = '\0';
            cout << "backend responded with " << backend_buffer << "backend response end here " << endl;
            send(client_fd, backend_buffer, backend_res, 0);
        }
        close(backend_fd);
    }
    else
    {
        cout << "no bytes recieved from browser request ";
    }
    close(client_fd);
}

void OmniGate::start_server()
{
    int proxy_fd = socket(AF_INET, SOCK_STREAM, 0); // Asked for a socket from OS
    if (proxy_fd == -1)                             // less than 0 means falied
    {
        perror("Failed to create proxy server socket");
        return;
    }
    cout << "Server socket created with proxy FD: " << proxy_fd << endl;
    struct sockaddr_in listner_config;                  // creating sockaddr config for listening socket
    memset(&listner_config, 0, sizeof(listner_config)); // making sure that the object that we made is clear and has no garbage values
    listner_config.sin_family = AF_INET;                // ipv4
    listner_config.sin_port = htons(port);              // using port for listeninga nd also htons(host to network short for converting from Little-Endian to Big-Endian)
    listner_config.sin_addr.s_addr = INADDR_ANY;        // 0.0.0.0 = INADDR_ANY for setting that no ip restriction for request

    if (::bind(proxy_fd, (struct sockaddr *)&listner_config, sizeof(listner_config)) == -1)
    {
        perror("Bind failed (Port may be in use)");
        close(proxy_fd);
        return;
    }

    if (listen(proxy_fd, 10) == -1)
    {
        perror("Listen failed");
        close(proxy_fd);
        return;
    }
    cout << "Proxy is listening on port " << port << endl;
    while (true)
    {
        int client_fd = accept(proxy_fd, nullptr, nullptr);
        if (client_fd > 0)
        {

            cout << "[+] New Browser Connected! Client FD: " << client_fd << " -> Assigned to Thread." << endl;
            thread t(&OmniGate::handel_client, this, client_fd);
            t.detach();
        }
    }
    close(proxy_fd);
}
