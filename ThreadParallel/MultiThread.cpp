// MultiThread.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <future>
#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <ctime>
#include "ThreadPool.h"
#include "JoiningThread.hpp"
#include "ParallelAccumulate.h"
#include "MTQueue.hpp"

//template <int32_t T>
//void Download()
//{
//	for (int32_t i = 0; i < T; ++i)
//	{
//		double precent = static_cast<double>(i) / static_cast<double>(T);
//		std::cout << "DownLoading " << " (" << precent * 100 << "%)..... " << std::endl;
//		std::this_thread::sleep_for(std::chrono::milliseconds(400));
//	}
//	std::cout << "Download Complete" << std::endl;
//}
//
//template <int32_t T>
//std::string DownLoad()
//{
//	for (int32_t i = 0; i < T; ++i)
//	{
//		double precent = static_cast<double>(i) / static_cast<double>(T);
//		std::cout << "DownLoading " << " (" << precent * 100 << "%)..... " << std::endl;
//		std::this_thread::sleep_for(std::chrono::milliseconds(400));
//	}
//	 return "stupid motherfucker";
//}
//
//
//void Interact()
//{
//	std::string name;
//	std::cout << " Print your name";
//	std::cin >> name;
//	std::cout << "Hi, " << name << std::endl;
//}

void MyThreadPool()
{
	ThreadPool pool;
    std::vector< std::future<int> > results;

    for(int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.Submit([i] {
				printf_s("hello %d\n", i);
                std::this_thread::yield();
				printf_s("world %d\n", i);
                return i*i;
            })
        );
    }

    for(auto && result: results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
}

int main()
{
	//std::thread t1([]() {Download<100>(); });
	//t1.join();
	//Interact();

	//std::future<std::string> res = std::async(
	//	[]() {return DownLoad<10>(); });
	//std::cout << res.get() << std::endl;

	//std::vector<int> arr;
	//std::mutex mtx;
	//std::thread t2([&] {
	//	for (int i = 0; i < 1000; ++i) {
	//		std::unique_lock<std::mutex> grd(mtx);
	//		std::cout << "t2 线程执行" << std::endl;
	//		arr.emplace_back(i);
	//	}
	//});
	//std::thread t3([&] {
	//	for (int i = 1000; i < 2000; ++i) {
	//		std::unique_lock<std::mutex> grd(mtx);
	//		std::cout << "t3 线程执行" << std::endl;
	//		arr.emplace_back(i);
	//	}
	//});
	//t2.join(); // 等待子进程
	//t3.join();

	//std::atomic<int> counter;
	//counter.store(0);
	//std::thread t4([&] {
	//	for (int i = 0; i < 1000; ++i) {
	//		counter += 1;
	//		counter -= 2;
	//	}
	//});

	//std::thread t5([&] {
	//	for (int i = 0; i < 1000; ++i) {
	//		counter += 1;
	//		counter -= 2;
	//	}
	//});
	//t4.join();
	//t5.join();

	//std::cout << counter.load() << std::endl;

	cout << "this is my thread pool" << endl;
	MyThreadPool();

	cout << "this is my joining thread" << endl;
	std::thread t1([]() {
		cout << "thread t1 generated!\n";
	});

	cout << "this thread id equals " << t1.get_id() << endl;
	std::thread t2([](){
		cout << "thread t2 generated!\n";
	});
	JoiningThread jt1(std::move(t1));
	JoiningThread jt2 = std::move(t2);
	JoiningThread jt3(std::move(jt1));
	cout << jt1.GetID() << " and " << jt3.GetID() << endl;

	int size = 100;
	vector<int> random(size);
	srand((int)time(0));
	for (int i = 0; i < size; ++i) {
		random[i] = i + 1;
	}
	printf_s("需要计算的随机数已经生成\n");

	cout << ParallelAccumulate(random.begin(), random.end(), static_cast<long long>(0)) << endl;
	bool test = false;
	auto return_type = BoolVariant(test);
	
	std::visit([&](auto returnType) {
		if constexpr (returnType)
			cout << typeid(return_type).name() << endl;
		}, return_type);
	/*
		lock_guard->RAII模板类，在构造时提供已锁的互斥量而在析构时解锁
		scope_guard->可以接受不定数量的互斥量作为参数，互斥量支持构造时上锁，析构时解锁
	*/ 

	MTQueue<int> task;
	task.GetQueue().Enqueue(2);
	int res;
	task.GetQueue().Dequeue(res);
	cout << res << endl;

	return 0;
}
