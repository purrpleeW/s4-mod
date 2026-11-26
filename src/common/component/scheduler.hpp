#pragma once
#include "common_core.hpp"
#include "utils/concurrency.hpp"
#include "utils/thread.hpp"

namespace scheduler {
	enum class pipeline {
		async = 0, 			//& Async pipeline, runs in a seperate thread
		client,				//& Game client pipeline, usually R_EndFrame
		server, 			//& Game server pipeline
		main, 				//& Universal game pipeline, works on both
		dvars, 				//& prev. dvars_init, Dvar initialization started
		frontend_enter, 	//& Frontend is entered/loaded (game loaded, match exited, etc.)
		frontend_exit, 		//& Frontend is exited (match started, etc.)
		count,				//? sanity checking
	};

	namespace detail {
		enum class task_type {
			looping, 		  //& -> scheduler::loop
			once,			  //& -> scheduler::once
			on_init, 		  //& -> scheduler::on_game_init
			count,			  //? sanity checking
		};

		struct task {
			std::function<void()> handler_{};
			std::chrono::milliseconds interval_{};
			task_type task_type_{};
			std::chrono::high_resolution_clock::time_point last_call{};
			std::atomic<bool>* conditional_run_;

			const bool yield() noexcept {
				const auto now = std::chrono::high_resolution_clock::now();
				const auto diff = now - this->last_call;

				if (diff < this->interval_) {
					return false;
				}

				this->last_call = now;
				return true;
			}
		};

		struct task_pipeline {
			void add(task&& task_) noexcept {
				return this->tasks_.push_back(std::move(task_));
			}

			void execute() noexcept {
				for (auto it = this->tasks_.begin(); it != tasks_.end();) {
					if (!it->yield()) {
						it++; continue;
					}

					if (it->conditional_run_ && !it->conditional_run_->load()) {
						it++; continue;
					}

					it->handler_();

					it->task_type_ == task_type::looping
						? it++
						: it = this->tasks_.erase(it);
					;
				}
			}

		private:
			utils::concurrency::tick_queue<task> tasks_{};
		};

		inline std::array<task_pipeline, ENUM_UNDER(pipeline::count)> pipeline_feed = {};
		inline std::function<bool()> is_in_frontend_callback = [] {
			return false;
		};

		template <const pipeline p_type, const task_type t_type>
		void dispatch(const std::function<void()>& callback, const std::chrono::milliseconds delay,
			std::atomic<bool>* conditional = nullptr) noexcept
		{
			static_assert(ENUM_UNDER(p_type) >= 0 && ENUM_UNDER(p_type) < ENUM_UNDER(pipeline::count), "Invalid pipeline type specified");
			static_assert(ENUM_UNDER(t_type) >= 0 && ENUM_UNDER(t_type) < ENUM_UNDER(task_type::count), "Invalid task type specified");

			auto dispatchable_task_ = task{
				.task_type_ = t_type,
				.interval_ = delay,
				.handler_ = std::move(callback),
				.last_call = std::chrono::high_resolution_clock::now(),
				.conditional_run_ = conditional
			};

			return pipeline_feed[ENUM_UNDER(p_type)].add(std::move(dispatchable_task_));
		}
	}

	template <const pipeline p_type>
	void execute() noexcept {
		static_assert(ENUM_UNDER(p_type) >= 0 && ENUM_UNDER(p_type) < ENUM_UNDER(pipeline::count), "Invalid pipeline type specified");

		return detail::pipeline_feed[ENUM_UNDER(p_type)].execute();
	}

	template <const pipeline p_type>
	inline void loop(const std::function<void()>& callback, std::chrono::milliseconds delay = 0ms) noexcept {
		return detail::dispatch<p_type, detail::task_type::looping>(std::move(callback), delay);
	}

	template <const pipeline p_type>
	inline void conditional_loop(const std::function<void()>& callback, std::atomic<bool>* conditional, std::chrono::milliseconds delay = 0ms) noexcept {
		return detail::dispatch<p_type, detail::task_type::looping>(std::move(callback), delay, conditional);
	}

	template <const pipeline p_type>
	inline void once(const std::function<void()>& callback, std::chrono::milliseconds delay = 0ms) noexcept {
		return detail::dispatch<p_type, detail::task_type::once>(std::move(callback), delay);
	}

	template <const pipeline p_type>
	inline void conditional_once(const std::function<void()>& callback, std::atomic<bool>* conditional, std::chrono::milliseconds delay = 0ms) noexcept {
		return detail::dispatch<p_type, detail::task_type::once>(std::move(callback), delay, conditional);
	}

	template <const pipeline p_type>
	inline void on_game_init(const std::function<void()>& callback, std::chrono::milliseconds delay = 0ms) noexcept {
		return detail::dispatch<p_type, detail::task_type::on_init>(std::move(callback), delay);
	}

	inline std::jthread init_async() noexcept {
		loop<pipeline::client>([] {
			static std::optional<bool> was_in_frontend_store = std::nullopt;

			auto is_in_frontend = detail::is_in_frontend_callback();
			auto was_in_frontend = was_in_frontend_store.value_or(false);

			if (!was_in_frontend && is_in_frontend) {
				execute<pipeline::frontend_enter>();
			}
			else if (was_in_frontend && !is_in_frontend) {
				execute<pipeline::frontend_exit>();
			}

			was_in_frontend_store = is_in_frontend;
		});

		auto thread_handle = utils::thread::create_named_jthread("Async Scheduler Pipeline", [](std::stop_token st) {
			while (!st.stop_requested()) {
				execute<pipeline::async>();
				std::this_thread::sleep_for(10ms);
			}
		});

		return thread_handle;
	}
}