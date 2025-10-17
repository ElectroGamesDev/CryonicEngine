#pragma once

#include "Event.h"
#include <functional>
#include <string>

namespace EventSystem
{
    /// Subscribes to an event by name. Returns a callback ID that can be used for reliable unsubscription.
    size_t Subscribe(const std::string eventName, std::function<void()> callback);

    /// Unsubscribes a callback using direct comparison.
    /// - This method does not work with lambdas that capture variables.
    /// - For those, store the ID from Subscribe() and call Unsubscribe(eventName, id) instead.
    void Unsubscribe(const std::string eventName, std::function<void()> callback);

    /// Unsubscribes a callback by its unique ID.
    /// - Use this for any lambdas with captures or complex callables.
    /// - The ID must be the one returned from the corresponding Subscribe() call.
    void Unsubscribe(const std::string eventName, size_t callbackId);

    /// Invokes all callbacks registered to the event.
    void Invoke(const std::string eventName);
};