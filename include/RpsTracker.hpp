#pragma once
#include <atomic>
#include <chrono>

class RpsTracker
{
private:
    std::atomic<uint64_t> current_epoch_second{0};
    std::atomic<uint64_t> current_second_counter{0};
    std::atomic<uint64_t> last_full_second_rps{0};

    std::atomic_flag time_lock = ATOMIC_FLAG_INIT;

    // Singleton constructor
    RpsTracker() = default;

public:
    static RpsTracker &getInstance()
    {
        static RpsTracker instance;
        return instance;
    }

    void recordRequest()
    {
        // current time in seconds
        auto now = std::chrono::system_clock::now();
        uint64_t current_time = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        // if in same second
        if (current_time == current_epoch_second.load(std::memory_order_relaxed))
        {

            current_second_counter.fetch_add(1, std::memory_order_relaxed);
        }
        // In a NEW second!
        else
        {
            // Try to grab the lock (the doorknob)
            if (!time_lock.test_and_set(std::memory_order_acquire))
            {

                // Double-check: Did another thread already update the time while we were grabbing the lock?
                if (current_time != current_epoch_second.load(std::memory_order_relaxed))
                {

                    // WE are the thread responsible for the rollover!
                    // Shift the counter to the "last full second"
                    last_full_second_rps.store(current_second_counter.load(std::memory_order_relaxed), std::memory_order_relaxed);

                    // Reset the counter to 1 (counting this exact request)
                    current_second_counter.store(1, std::memory_order_relaxed);

                    // Update the epoch time so other threads know the new second started
                    current_epoch_second.store(current_time, std::memory_order_relaxed);
                }
                else
                {
                    // Someone else beat us to it, just increment.
                    current_second_counter.fetch_add(1, std::memory_order_relaxed);
                }

                // Release the lock
                time_lock.clear(std::memory_order_release);
            }
            else
            {
                // We didn't get the lock. Another thread is doing the math right now.
                // Just increment, it will safely apply to the newly reset counter.
                current_second_counter.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

    // This is what your AI Tool will read
    uint64_t getRps()
    {
        return last_full_second_rps.load(std::memory_order_relaxed);
    }
};