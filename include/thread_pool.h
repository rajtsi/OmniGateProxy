#pragma once
#include <functional>
#include <vector>
#include <iostream>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
class ThreadPool
{
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::condition_variable cv;
    std::mutex mtx;
    void worker_loop();
    bool stop = false;

public:
    ThreadPool(int num_threads);
    void add_job(std::function<void()> task);
    ~ThreadPool();
};
