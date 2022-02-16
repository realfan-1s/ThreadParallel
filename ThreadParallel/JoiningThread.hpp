#pragma once
#include <thread>
#include <utility>

class JoiningThread
{
private:
	std::thread t;
public:
	JoiningThread() noexcept = default;
	template <typename F, typename ... Args>
	explicit JoiningThread(F&& f, Args&&... args) : t(std::forward<F>(f), std::forward<Args>(args)...) {}
	JoiningThread(JoiningThread&) = delete;
	JoiningThread& operator=(JoiningThread&) = delete;
	JoiningThread(JoiningThread&& other) noexcept : t(std::move(other.t)) {}
	JoiningThread(std::thread t1) noexcept : t(std::move(t1)) {}
	JoiningThread& operator=(JoiningThread&& other) noexcept {
		if (t.joinable())
			t.join();
		t = std::move(other.t);
		return *this;
	}
	JoiningThread& operator=(std::thread other) noexcept {
		if (t.joinable())
			t.join();
		t = std::move(other);
		return *this;
	}
	~JoiningThread() {
		if (t.joinable())
			t.join();
	}
	std::thread::id GetID() const noexcept {
		return t.get_id();
	}
};

