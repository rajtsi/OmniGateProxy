#pragma once
#include <atomic>
#include <string>

class MetricsTracker
{
public:
    // Using atomic variables for lock-free, ultra-fast thread safety
    std::atomic<uint64_t> total_requests{0};
    std::atomic<uint64_t> active_connections{0};
    std::atomic<uint64_t> status_2xx{0};
    std::atomic<uint64_t> status_4xx{0};
    std::atomic<uint64_t> status_5xx{0};

    // Singleton instance so both Proxy and MCP threads access the exact same numbers
    static MetricsTracker &getInstance()
    {
        static MetricsTracker instance;
        return instance;
    }

    void incrementRequest() { total_requests++; }
    void incrementActive() { active_connections++; }
    void decrementActive()
    {
        if (active_connections > 0)
            active_connections--;
    }

    void trackStatusCode(int code)
    {
        if (code >= 200 && code < 300)
            status_2xx++;
        else if (code >= 400 && code < 500)
            status_4xx++;
        else if (code >= 500 && code < 600)
            status_5xx++;
    }
};