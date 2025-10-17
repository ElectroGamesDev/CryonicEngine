#pragma once

#include <functional>
#include <mutex>
#include <queue>

namespace MainThreadQueue {
    static std::queue<std::function<void()>> tasks;
    static std::mutex tasksMutex;

    inline void Add(std::function<void()> task)
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        tasks.push(task);
    }

    inline void Process()
    {
        std::lock_guard<std::mutex> lock(tasksMutex);
        while (!tasks.empty())
        {
            tasks.front()();
            tasks.pop();
        }
    }
}