#pragma once

#include <map>
#include <thread>
#include <mutex>
#include <coroutine>
#include <utility>
#include <shared_mutex>
#include <list>
#include <vector>
#include <algorithm>

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class MTDictionary
{
private:
	class BucketType
	{
	public:
		Value operator[] (const Key& key) {
			std::shared_lock<std::shared_mutex> lock(mtx);
			const iterator entry = GetEntryFrom(key);
			return (entry == data.end()) ? Value{} : entry->second;
		}
		
		void Emplace(const Key& key, const Value& value) {
			std::unique_lock<std::shared_mutex> lock(mtx);
			const iterator entry = GetEntryFrom(key);
			if (entry == data.end())
				data.emplace_back(bucketValue(key, value));
			else {
				(*entry).second = value;
			}
		}
		
		void Remove(const Key& key) {
			std::unique_lock<std::shared_mutex> lock(mtx);
			const iterator entry = GetEntryFrom(key);
			if (entry != data.end()) {
				data.erase(entry);
			}
		}

		bool empty() const {
			std::shared_lock lock(mtx);
			return data.empty();
		}

		size_t size() const {
			std::shared_lock lock(mtx);
			return data.size();
		}
	private:
		using bucketValue = typename std::pair<Key, Value>;
		using bucketData = typename std::list<bucketValue>;
		using iterator = bucketData::iterator;
		iterator GetEntryFrom(const Key& key) {
 			return std::find_if(data.begin(), data.end(), [&](const bucketValue& t) { return t.first == key; });
		}
		bucketData data;
		mutable std::shared_mutex mtx; // 可能会涉及到锁转移所有权问题所以需要使用shared_mutex
	};

	std::vector<std::unique_ptr<BucketType>> buckets;
	Hash hasher;
	BucketType& GetBucketFrom(const Key& key) const {
		const size_t pos = hasher(key) % buckets.size();
		return *buckets[pos];
	}
public:
	MTDictionary(size_t bucketCount = 23, const Hash& hasher = Hash()) : buckets(bucketCount), hasher(hasher) {
		for (int i = 0; i < bucketCount; ++i)
			buckets[i].reset(new BucketType);
	}
	MTDictionary(const MTDictionary&) = delete;
	MTDictionary& operator=(const MTDictionary&) = delete;
	Key operator[](const Key& key) {
		return GetBucketFrom(key)[key];
	}
	void Emplace(const Key& key, const Value& value) {
		GetBucketFrom(key).Emplace(key, value);
	}
	void Remove(const Key& key) {
		GetBucketFrom(key).Remove(key);
	}
	std::map<Key, Value> GetWholeMap() {
		std::vector<std::unique_lock<std::shared_lock>> locks;
		for (auto& bucket : buckets) {
			locks.emplace_back(std::unique_lock(bucket.mtx));
		}
		std::map<Key, Value> ans;
		for (auto& bucket : buckets) {
			for (auto iter = bucket.data.begin(); iter != bucket.data.end(); ++iter) {
				ans.emplace(*iter);
			}
		}
		return ans;
	} // 获取当前的整个查询表 

	bool empty() const {
		bool ans = false;
		for (auto& bucket : buckets) {
			ans &= bucket->empty();
		}
		return ans;
	}

	size_t size() const {
		size_t ans = 0;
		for (auto& bucket : buckets) {
			ans += bucket->size();
		}
		return ans;
	}
};

