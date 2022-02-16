#pragma once
#include "MTQueue.hpp"
#include <thread>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <future>
#include <vector>

class ThreadPool {
private:
	MTQueue<std::function<void()>> tasks;
	std::vector<std::thread> pool;
	std::condition_variable condition;
	std::mutex task_lock;
	bool stop;
public:
	ThreadPool(size_t maxThread = std::thread::hardware_concurrency());
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(ThreadPool&) = delete;
	ThreadPool& operator=(ThreadPool&&) = delete;
	~ThreadPool();
	template <class F, class... Args>
	auto Submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
	{
		using return_type = typename std::invoke_result_t<F, Args...>;
		auto currTask = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f),
		std::forward<Args>(args)...)); // 获取浅拷贝, 使用package_task包装一个可调用对象并允许异步获取结果,拓展包(...)必须要在上下文环境中展开
		tasks.GetQueue().Enqueue([currTask]() { return (*currTask)(); });
		condition.notify_one();
		return currTask->get_future();
	}
};

