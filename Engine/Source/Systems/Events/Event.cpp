#include "Event.h"
#include "Utilities/ConsoleLogger.h"
#include <algorithm>

size_t Event::Subscribe(std::function<void()> callback)
{
    size_t id = nextId++;
    callbacks.push_back({ id, std::move(callback) });
    return id;
}

void Event::Unsubscribe(std::function<void()> callback)
{
    auto ptr = callback.target<void(*)()>();
    if (!ptr)
    {
        ConsoleLogger::ErrorLog("Failed to unsubscribe callback. Either this callback does not exist or you will need to unsubscribe it using the ID returned from Subscribe(). You can use Unsubscribe(id).");
        return;
    }

    callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(),
        [&](const CallbackEntry& entry) {
            auto entryPtr = entry.callback.target<void(*)()>();
            return entryPtr && *entryPtr == *ptr;
        }),
        callbacks.end());

}

void Event::Unsubscribe(size_t id)
{
    size_t oldSize = callbacks.size();

    callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(),
        [=](const CallbackEntry& entry) { return entry.id == id; }),
        callbacks.end()
    );

    if (callbacks.size() == oldSize)
        ConsoleLogger::ErrorLog("Failed to unsubscribe callback: no callback found with id = " + std::to_string(id));
}

void Event::Invoke()
{
    for (auto& entry : callbacks)
    {
        if (entry.callback)
            entry.callback();
    }
}

int Event::CallbackCount()
{
    return static_cast<int>(callbacks.size());
}
