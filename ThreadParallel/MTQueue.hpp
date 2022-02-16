#pragma once

#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

using namespace std;

template <typename T>
class MTQueue
{
private:
	queue<T> m_queue;
	mutex mtx;
public:
	class Accessor
	{
	private:
		MTQueue& curr;
		unique_lock<mutex> guard;
		condition_variable cv;
	public:
		Accessor(MTQueue& other) : curr(other), guard(other.mtx) {}
		Accessor(MTQueue&& other) = delete;
		Accessor& operator=(MTQueue&&) = delete;
		void Enqueue(T&& val) const
		{
			curr.m_queue.emplace(forward<T>(val));
		}
		size_t Size() const
		{
			return curr.m_queue.size();
		}
		bool Empty() const
		{
			return curr.m_queue.empty();
		}
		bool TryDequeue(T& t) noexcept(true) 
		{
			if (curr.m_queue.empty())
				return false;
			t = move(curr.m_queue.front());
			curr.m_queue.pop();
			return true;
		}
		std::shared_ptr<T> TryDequeue() noexcept(true)
		{
			if (curr.m_queue.empty())
				return std::make_shared<T>();
			auto res = std::make_shared<T>(curr.m_queue.front());
			curr.m_queue.pop();
			return res;
		}
		void Dequeue(T& t) noexcept(true)
		{
			cv.wait(guard, [this]() { return !curr.m_queue.empty(); });
			t = move(curr.m_queue.front());
			curr.m_queue.pop();
		}
		std::shared_ptr<T> Dequeue() noexcept(true) 
		{
			cv.wait(guard, [this]() { return !curr.m_queue.empty(); });
			auto res(std::make_shared<T>(curr.m_queue.front()));
			curr.m_queue.pop();
			return res;
		}
	};

	Accessor GetQueue()
	{
		return { *this };
	}
};
