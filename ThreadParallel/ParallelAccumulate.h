#pragma once

#include <thread>
#include <numeric>
#include <vector>
#include <iterator>
#include <variant>

template <typename iter, typename T>
struct ParallelAccumulateBlock
{
	void operator() (iter start, iter end, T& ans) {
		ans = std::accumulate(start, end, ans);
	}
};

template <typename iter, typename T, typename size_t min_task_pre_thread = 25>
static T ParallelAccumulate(iter start, iter end, T init) {
	const size_t len = std::distance(start, end);
	if (!len) // 任务数为零
		return init;
	const size_t max_threads = (len + min_task_pre_thread - 1) / min_task_pre_thread; // 若范围内元素多余一个时，用范围内元素总数除以线程块中最小的任务数
	const size_t hardware_threads = std::thread::hardware_concurrency();
	const size_t real_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
	const size_t block_size = len / real_threads;

	std::vector<T> results(real_threads);
	std::vector<std::thread> threads(real_threads - 1); // 因为已经存在了一个主线程，所以创建线程比总线程数少1

	iter block_start = start;
	for (size_t i = 0; i < real_threads - 1; ++i) {
		iter block_end = block_start;
		std::advance(block_end, block_size); // 增加给定的迭代器 it 以 n 个元素的步长。
		threads[i] = std::thread(ParallelAccumulateBlock<iter, T>(), block_start, block_end, std::ref(results[i])); // 在模板传参的时候传入引用，否则无法传递
		block_start = block_end;	
	}
	ParallelAccumulateBlock<iter, T>()(block_start, end, results[real_threads - 1]);
	for (auto& thread : threads) {
		if (thread.joinable())
			thread.join();
	}

	return std::accumulate(results.begin(), results.end(), init);
}

static std::variant<std::true_type, std::false_type> BoolVariant(bool x) {
	if (x)
		return std::true_type{};
	return std::false_type{};
}