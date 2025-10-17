#pragma once

#include <vector>
#include <functional>

class Event
{
public:
	/// Subscribes a callback to the event and returns a unique ID.
	/// - If you plan to unsubscribe a lambda with captures, you must unsubscribe it using the returned ID.
	size_t Subscribe(std::function<void()> callback);

	/// Unsubscribes a callback using direct comparison.
	/// - This method does not work with lambdas that capture variables.
	/// - For those, store the ID from Subscribe() and call Unsubscribe(id) instead.
	void Unsubscribe(std::function<void()> callback);

	/// Unsubscribes a callback by its unique ID.
	/// - Use this for any lambdas with captures or complex callables.
	/// - The ID must be the one returned from the corresponding Subscribe() call.
	void Unsubscribe(size_t id);

	void Invoke();
	int CallbackCount();

private:
	struct CallbackEntry {
		size_t id;
		std::function<void()> callback;
	};

	std::vector<CallbackEntry> callbacks;
	size_t nextId = 0;
};