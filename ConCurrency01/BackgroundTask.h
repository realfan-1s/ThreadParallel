#pragma once

#include <thread>
#include <coroutine>

template <typename T>
class BackgroundTask
{
private:
public:
	struct promise_type
	{
		T value;
		std::suspend_always initial_suspend() { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		std::suspend_always yield_value(T val) { 
			this->value = val;
			return {}; 
		}
		BackgroundTask<T> get_return_object() {
			return BackgroundTask<T> { std::coroutine_handle<promise_type>::from_promise(*this) };
		}
		void unhandled_exception() {}
		void return_void() {}
	};
	std::coroutine_handle<promise_type> handle;
	void DoSomething(T) const {}
	void operator() () const {}
};

