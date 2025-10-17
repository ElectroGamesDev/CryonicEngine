#include "EventSystem.h"
#include "Utilities/ConsoleLogger.h"
#include <unordered_map>

namespace EventSystem
{
    static std::unordered_map<std::string, Event> events;

    size_t Subscribe(std::string eventName, std::function<void()> callback)
    {
        auto& event = events[eventName];
        return event.Subscribe(callback);
    }

    void Unsubscribe(std::string eventName, std::function<void()> callback)
    {
        auto it = events.find(eventName);
        if (it != events.end())
        {
            it->second.Unsubscribe(callback);

            if (it->second.CallbackCount() < 1)
                events.erase(it);
        }
        else
            ConsoleLogger::ErrorLog("Failed to unsubscribe callback: the event '" + eventName + "' does not exist.");

    }

    void Unsubscribe(const std::string eventName, size_t callbackId)
    {
        auto it = events.find(eventName);
        if (it != events.end())
        {
            it->second.Unsubscribe(callbackId);

            if (it->second.CallbackCount() < 1)
                events.erase(it);
        }
        else
            ConsoleLogger::ErrorLog("Failed to unsubscribe: the event '" + eventName + "' does not exist.");
    }


    void Invoke(std::string eventName)
    {
        auto it = events.find(eventName);
        if (it != events.end())
            it->second.Invoke();
        else
            ConsoleLogger::ErrorLog("Failed to invoke callbacks: the event '" + eventName + "' does not exist.");

        events[eventName].Invoke();
    }
}