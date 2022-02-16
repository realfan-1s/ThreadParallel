#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <stack>
#include <coroutine>
#include <initializer_list>
#include <vector>
#include <memory>

template <typename T>
class MTStack
{
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
		MTStack<T> get_return_object() {
			return MTStack<T> { std::coroutine_handle<promise_type>::from_promise(*this) };
		}
		void unhandled_exception() {}
		void return_void() { }
	};
	std::coroutine_handle<promise_type> handler;
	MTStack() {}
	MTStack(const MTStack& other) {
		std::unique_lock lk(mtx);
		this->stk = other.stk;
	}
	explicit MTStack(std::coroutine_handle<promise_type> handle) : handler(std::move(handle)) {}
	MTStack& operator=(const MTStack&) = delete;
	void Emplace(T val) {
		auto res(std::make_shared<T>(std::move(val)));
		std::lock_guard lk(mtx);
		this->stk.emplace(res);
		cv.notify_one();
	}
	std::shared_ptr<T> Pop() {
		std::lock_guard lk(mtx);
		cv.wait(lk, [this]() { return !stk.empty(); });
		const std::shared_ptr<T> ans(std::make_shared<T>(std::move(stk.top())));
		stk.pop();
		return ans;
	}
	void Pop(T& val) {
		std::lock_guard lk(mtx);
		cv.wait(lk, [this]() { return !stk.empty(); });
		val = std::move(*stk.top());
		stk.pop();
	}
	bool empty() {
		std::lock_guard lk(mtx);
		return stk.empty();
	}
	~MTStack() {}

private:
	std::mutex mtx;
	std::stack<std::shared_ptr<T>> stk;
	std::condition_variable cv;
};