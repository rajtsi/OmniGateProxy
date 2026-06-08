#include "../include/thread_pool.h"

ThreadPool::ThreadPool(int num_threads)
{
    while (num_threads--) // made the number of threads required and pushed them in a vector
    {
        workers.emplace_back(&ThreadPool::worker_loop, this);
    }
}

void ThreadPool::add_job(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        tasks.push(task);
    }
    cv.notify_one();
}

void ThreadPool::worker_loop()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx);

            cv.wait(lock, [this]
                    { return !tasks.empty() || stop; });

            if (stop && tasks.empty())
                return;

            task = tasks.front();
            tasks.pop();
        }

        task();
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();
    for (std::thread &worker : workers)
    {
        worker.join();
    }
    std::cout << "Thread pool distructor was called ";
}