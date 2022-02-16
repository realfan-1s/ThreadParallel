#include "ThreadPool.h"
#include <algorithm>

ThreadPool::~ThreadPool() {
	{
		unique_lock<std::mutex> lock(task_lock);
		stop = true;
	}
	condition.notify_all();
	for_each(pool.begin(), pool.end(), [](std::thread& t)
	{
		if (t.joinable())
			t.join();
	});
}

ThreadPool::ThreadPool(size_t maxThread) : stop(false)
{
	for (size_t i = 0; i < maxThread; ++i)
	{
		pool.emplace_back([this]() {
				for (;;)
				{
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(this->task_lock);
						this->condition.wait(lock, [this] { return this->stop || !this->tasks.GetQueue().Empty(); });
						if (this->stop || tasks.GetQueue().Empty())
							return;
						tasks.GetQueue().TryDequeue(task);
					}
					task();
				}
			});
	}
}
