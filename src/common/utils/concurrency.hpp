#pragma once

#include <future>
#include <mutex>
#include <vector>
#include <functional>
#include <deque>

namespace utils::concurrency {
	template <typename T, typename MutexType = std::mutex>
	class container final {
	public:
		template <typename R = void, typename F>
		R access(F&& accessor) const {
			std::lock_guard<MutexType> _{mutex_};
			return accessor(object_);
		}

		template <typename R = void, typename F>
		R access(F&& accessor) {
			std::lock_guard<MutexType> _{mutex_};
			return accessor(object_);
		}

		template <typename R = void, typename F>
		R access_with_lock(F&& accessor) const {
			std::unique_lock<MutexType> lock{mutex_};
			return accessor(object_, lock);
		}

		template <typename R = void, typename F>
		R access_with_lock(F&& accessor) {
			std::unique_lock<MutexType> lock{mutex_};
			return accessor(object_, lock);
		}

		T& get_raw() {
			return object_;
		}

		const T& get_raw() const {
			return object_;
		}

		T copy() const {
			std::unique_lock<MutexType> lock{mutex_};
			return object_;
		}

		std::unique_lock<MutexType> acquire_lock() {
			return std::unique_lock<MutexType>{mutex_};
		}

	private:
		mutable MutexType mutex_{};
		T object_{};
	};

	template <typename T>
	class tick_queue final {
		using container_type = std::vector<T>;
		using iterator = typename container_type::iterator;

		std::mutex mutex_;
		container_type active_;
		container_type pending_;

		void promote() {
			std::lock_guard<std::mutex> lock(this->mutex_);

			for (const auto& item : this->pending_) {
				this->active_.emplace_back(std::move(item));
			}

			this->pending_.clear();
		}
	public:
		// Add element — always goes to the next cycle
		void push_back(T&& value) {
			std::lock_guard<std::mutex> lock(this->mutex_);
			this->pending_.push_back(std::move(value));
		}

		iterator begin() {
			this->promote();
			return this->active_.begin();
		};

		iterator end() {
			return this->active_.end();
		}

		iterator erase(iterator it) {
			std::lock_guard<std::mutex> lock(this->mutex_);
			return this->active_.erase(it);
		}
	};

	template <typename T>
	inline void async_for_each(std::vector<T>& v, std::function<void(T&)> f) {
#		if defined(_WIN64)
			std::vector<std::thread> threads;

			for (auto& e : v) {
				threads.emplace_back([f, &e] {
					f(e);
				});
			}

			for (auto& t : threads) {
				if (t.joinable()) {
					t.join();
				}
			}
#		else
			for (auto& e : v) {
				f(e);
			}
#		endif
	}

	template <typename T, typename F>
	inline std::future<void> async_for_one(T& element, F&& f) {
		// Launch asynchronously
		return std::async(std::launch::async, [&f, &element]() {
			f(element);
		});
	}
}
