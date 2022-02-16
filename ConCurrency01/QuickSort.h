#pragma once

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <list>
#include <future>
#include <algorithm>
#include <type_traits>

template <typename F, typename... Args>
static auto SpawnTask(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
	using result_type = typename std::invoke_result_t<F, Args...>;
	std::packaged_task<result_type(Args&&...)> task(std::move(f));
	auto ans = task.get_future();
	std::thread t(std::move(task), std::move(args...));
	t.detach();
	return ans;
}

template <typename T>
static std::list<T> QuickSortParallel(std::list<T> input) {
	if (input.empty())
		return input;
	std::list<T> ans;
	ans.splice(ans.begin(), input, input.begin());
	const T& point = *ans.begin();
	auto dividePoint = std::partition(input.begin(), input.end(), [&](const T& iter) { return iter < point; });
	std::list<T> lowerPart;
	lowerPart.splice(lowerPart.end(), input, input.begin(), dividePoint);
	std::future<std::list<T>> lower = std::async(&QuickSortParallel<T>, std::move(lowerPart)); // 不小于中间值的部分启用另一线程进行排序
	std::future<std::list<T>> higher = SpawnTask(&QuickSortParallel<T>, std::move(input));
	ans.splice(ans.end(), higher.get());
	ans.splice(ans.begin(), lower.get());
	return ans;
} 

