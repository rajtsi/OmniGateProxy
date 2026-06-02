# OmniGate - Intelligent L4/L7 Proxy & Load Balancer 🚀

OmniGate is a high-performance, multi-threaded reverse proxy and load balancer built from scratch in modern C++ (C++11/C++20 concepts). It is designed to handle concurrent client requests and distribute traffic seamlessly across multiple backend servers.

## ✨ Core Features
- **Multi-threaded Architecture:** Utilizes `std::thread` to handle multiple concurrent clients without blocking the main event loop.
- **Round Robin Load Balancing:** Distributes incoming traffic evenly across a pool of backend servers.
- **Thread Safety:** Implements `std::mutex` to prevent race conditions during backend server selection.
- **Passive Failover (Fault Tolerance):** Automatically detects dead backend servers and reroutes traffic to the next healthy node in real-time, ensuring zero downtime for the client.
- **Raw Socket Programming:** Built entirely using POSIX sockets (`<sys/socket.h>`), eliminating reliance on heavy external networking frameworks.

## 🛠️ Architecture
- **Language:** C++
- **Build System:** Makefile
- **Design Pattern:** Object-Oriented (OOP) with Dispatcher-Worker threading model.

## 🚀 How to Run

1. **Clone the repo and build:**
   ```bash
   make

2.  Start your backend servers (e.g., Python simple servers on ports 9000, 9001, 9002).
3.  Run OmniGate:
    ```bash
    ./bin/omni_gate

4.  Visit http://localhost:8080 in your browser. 
