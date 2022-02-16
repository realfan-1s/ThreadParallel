// ConCurrency01.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "BackgroundTask.h"
#include <shared_mutex>
#include <memory>
#include <utility>
#include <future>
#include <deque>
#include <list>
#include <random>
#include "QuickSort.h"
#include "MTStack.hpp"
#include "MTDictionary.hpp"

void ProcessData(std::unique_ptr<BackgroundTask<int>>) {
    printf_s("Unique_ptr test!");
}

bool ShutDownMessage() {
    return false;
}
std::mutex mtx;
std::deque<std::packaged_task<void()>> tasks;

static void GuiProcess() {
    while (!ShutDownMessage()) {
        std::packaged_task<void()> task;
        {
            std::scoped_lock guard(mtx);
            if (!tasks.empty())
                continue;
            task = std::move(tasks.front());
            tasks.pop_front();
        }
        task();
    }
}

template <typename Func>
static std::future<void> PoskTaskToGuiThread(const Func& f) { // 发出一条正确的信息给相应的线程进行界面更新
    std::packaged_task<void()> task(f);
    auto res = task.get_future();
    std::unique_lock guard(mtx);
    tasks.emplace_back(std::move(task));
    return res; 
}

template <typename T>
BackgroundTask<T> sequence() {
    int i = 0;
    while (true) {
        i++;
        co_yield i * i;
    }

    co_return;
}

template <typename T>
MTStack<T> Sequence() {
    int i = 0;
    while (true) {
        ++i;
        co_yield i * i;
    }

    co_return;
}

int main()
{
    BackgroundTask<int> bt;
    int num = 0;
    // 传递成员函数指针作为线程函数，并提供一个合适的对象指针作为第一个参数
    // std::thread可移动但不可复制
    std::thread t(&BackgroundTask<int>::DoSomething, std::ref(bt), num); 
    // 提供参数若仅支持移动而不能拷贝，
    BackgroundTask<int>* bt2 = new BackgroundTask<int>();
    std::unique_ptr<BackgroundTask<int>> test(std::move(bt2)); // 没有拷贝构造函数，只有移动构造函数
    std::thread t2(ProcessData, std::move(test)); //若是临时变量，自动进行移动，但命名对象需要显示移动
    std::cout << "Hello World!\n";
    t.detach();
    t2.join();

    int count = 1 << 10;
    srand((int)time(0));
    std::list<int> random;
    for (int i = 0; i < count; ++i) {
        random.emplace_back(rand());
    }

    auto ans = QuickSortParallel(std::move(random));
    for (auto iter = ans.begin(); iter != ans.end(); ++iter) {
        printf_s("%d\n", *iter);
    }

    auto gen = sequence<int>();
    for (int i = 0; i < 10; ++i) {
        gen.handle.resume(); // 将函数的执行权由main转给协程
        std::cout << gen.handle.promise().value << std::endl;
    }

    auto gen1 = Sequence<int>();
    for (int i = 0; i < 20; ++i) {
        gen1.handler.resume();
        std::cout << gen1.handler.promise().value << std::endl;
    }

    MTDictionary<int, int> dict;
    std::thread dict1([&]() {
        for (int i = 0; i < 100; ++i)
            dict.Emplace(i, i);
        });
    dict1.join();
    std::thread dict2([&]() {
        for (int i = 0; i < 100; ++i)
            dict.Remove(i);
        });
    dict2.join();
    std::cout << dict[1] << " dict size = " << dict.size() << std::endl;

    return 0;
}