#pragma once

#include "libs/libcoro/task.hpp"
#include "libs/libcoro/event.hpp"

#include <list>
#include <deque>
#include <mutex>
#include <chrono>
#include <cassert>
#include <algorithm>
#include <condition_variable>

namespace hr
{
	class Scheduler
	{
		friend class Dispatcher;

		struct Task
		{
			size_t taskId;
			size_t dispatcherId;
			coro::task<void> coroTask;
		};

		struct TaskHandle
		{
			size_t taskId;
			std::coroutine_handle<> handle;
		};

		std::mutex mWaitMutex;
		std::condition_variable mWaitCV;
		std::atomic<bool> mShutdownRequested{false};

		std::atomic<size_t> mWaitFinishRequests{0};
		std::condition_variable mWaitFinishCV;

		std::deque<Task> mTasks;
		std::deque<TaskHandle> mQueue;

		std::chrono::time_point<std::chrono::steady_clock> mWaitTimedLastCheck;
		std::list<std::tuple<std::chrono::milliseconds, TaskHandle>> mWaitQueue;

		static std::atomic<size_t> sGenTaskId;

		class OpThreadSwitch
		{
			friend class Scheduler;

			Scheduler& mScheduler;
			size_t mTaskId{0};
			std::coroutine_handle<> mAwaitingCoro{nullptr};
			std::optional<std::chrono::milliseconds> mWaitDuration;

			explicit OpThreadSwitch(Scheduler& scheduler, size_t taskId) noexcept
			  : mScheduler{scheduler}
			  , mTaskId{taskId}
			{ }

			explicit OpThreadSwitch(Scheduler& scheduler, size_t taskId, std::chrono::milliseconds duration) noexcept
			  : mScheduler{scheduler}
			  , mTaskId{taskId}
			  , mWaitDuration{duration}
			{ }

		public:
			bool await_ready() noexcept
			{
				return false;
			}

			void await_suspend(std::coroutine_handle<> awaitingCoro) noexcept
			{
				mAwaitingCoro = awaitingCoro;
				mScheduler.scheduleCoro(mTaskId, mAwaitingCoro, mWaitDuration);
			}

			void await_resume() noexcept
			{
			}
		};

		void scheduleCoro(size_t taskId, std::coroutine_handle<> handle, std::optional<std::chrono::milliseconds> waitDuration) noexcept
		{
			if (!handle)
				return;

			{
				std::lock_guard lock{mWaitMutex};

				if (waitDuration && (waitDuration.value().count() > 0))
				{
					//put into the wait queue
					if (mWaitQueue.empty())
					{
						mWaitQueue.push_back(std::make_tuple(*waitDuration, TaskHandle{.taskId = taskId, .handle = handle}));
						mWaitTimedLastCheck = std::chrono::steady_clock::now();
					}
					else
					{
						auto newElement = std::make_tuple(*waitDuration, TaskHandle{.taskId = taskId, .handle = handle});
						auto upper = std::upper_bound(mWaitQueue.begin(), mWaitQueue.end(), newElement, [](const auto& a, const auto& b) { return (std::get<0>(a) < std::get<0>(b)); });
						mWaitQueue.emplace(upper, std::move(newElement));
					}
				}
				else
				{
					//put directly into queue
					mQueue.emplace_back(TaskHandle{.taskId = taskId, .handle = handle});
				}
				
			}

			mWaitCV.notify_one();
		}

		[[nodiscard]] OpThreadSwitch schedule(size_t taskId) noexcept
		{
			return OpThreadSwitch{*this, taskId};
		}

		[[nodiscard]] OpThreadSwitch schedule(size_t taskId, std::chrono::milliseconds duration) noexcept
		{
			return OpThreadSwitch{*this, taskId, duration};
		}

		template<class TCallback, class... TArgs>
		[[nodiscard]] coro::task<void> schedule(size_t dispatcherId, size_t taskId, TCallback callback, TArgs... args)
		{
			co_await schedule(taskId); //this will swith to one of the scheduler's threads

			//we're now inside of a scheduler's thread

			if constexpr (std::is_invocable_v<TCallback, Context&, TArgs...>)
			{
				Context ctx{*this, dispatcherId, taskId};

				if constexpr (std::is_same_v<void, decltype(callback(ctx, std::forward<TArgs>(args)...))>)
				{
					//task that receives a Context (without other parameters) and returns nothing
					callback(ctx, std::forward<TArgs>(args)...);
					co_return;
				}
				else
				{
					//task that receives a Context (without other parameters) and returns (i.e.: a coroutine)
					co_await callback(ctx, std::forward<TArgs>(args)...);
					co_return;
				}
			}
			else
			{
				if constexpr (std::is_same_v<void, decltype(callback(std::forward<TArgs>(args)...))>)
				{
					//task receives and returns 
					callback(std::forward<TArgs>(args)...);
					co_return;
				}
				else
				{
					//task receives nothing and returns (i.e.: a coroutine)
					co_await callback(std::forward<TArgs>(args)...);
					co_return;
				}
			}
		}

		template<class TCallback, class... TArgs>
		size_t post(size_t dispatcherId, TCallback&& callback, TArgs&&... args)
		{
			if (mShutdownRequested)
				return 0;

			auto taskId = ++sGenTaskId;
			auto coroTask = schedule(dispatcherId, taskId, std::forward<TCallback>(callback), std::forward<TArgs>(args)...);

			auto coroHandler = coroTask.handle();
			{
				std::unique_lock lock_guard{mWaitMutex};
				mTasks.emplace_back(Task{.taskId = taskId, .dispatcherId = dispatcherId, .coroTask = std::move(coroTask)});
			}

			//we can start now (to co_await on the thread switch), instead of having to explicitly wait on the corountine
			assert(coroHandler);
			coroHandler.resume();

			return taskId;
		}

		void wait(size_t dispatcherId) noexcept
		{
			std::unique_lock lock{mWaitMutex};

			if (mTasks.empty())
				return;

			++mWaitFinishRequests;

			while (std::any_of(mTasks.begin(), mTasks.end(), [dispatcherId](const auto& task) { return (task.dispatcherId == dispatcherId); }))
				mWaitFinishCV.wait(lock);

			--mWaitFinishRequests;
		}

	public:
		class Context
		{
			friend class Scheduler;

			Scheduler& mScheduler;
			size_t mTaskId;
			size_t mDispatcherId;

			Context(Scheduler& scheduler, size_t dispatcherId, size_t taskId) noexcept
			  : mScheduler{scheduler}
			  , mTaskId{taskId}
			  , mDispatcherId{dispatcherId}
			{ }

		public:
			template<class TCallback, class... TArgs>
			bool post(TCallback&& callback, TArgs&&... args)
			{
				return mScheduler.post(mDispatcherId, std::forward<TCallback>(callback), std::forward<TArgs>(args)...);
			}

			[[nodiscard]] OpThreadSwitch yield()
			{
				return mScheduler.schedule(mTaskId);
			}

			[[nodiscard]] OpThreadSwitch yield(std::chrono::milliseconds duration)
			{
				return mScheduler.schedule(mTaskId, duration);
			}
		};

	public:
		void run()
		{
			while (!mShutdownRequested)
			{
				TaskHandle taskHandle{.taskId = 0, .handle = nullptr};
				{
					std::unique_lock lock{mWaitMutex};

					//before anything, process the wait queue
					if (!mWaitQueue.empty())
					{
						auto lastCheck = std::exchange(mWaitTimedLastCheck, std::chrono::steady_clock::now());
						auto timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(mWaitTimedLastCheck - lastCheck);

						bool tasksExpired{false};
						{
							//move all expired tasks to the queue
							auto it = mWaitQueue.begin();
							while ((it != mWaitQueue.end()) && (std::get<0>(*it) <= timeElapsed))
							{
								tasksExpired = true;
								mQueue.emplace_back(std::move(std::get<1>(*it)));
								it = mWaitQueue.erase(it);
							}

							//subtract the elapsed time for the rest
							for (; it != mWaitQueue.end(); ++it)
								std::get<0>(*it) -= timeElapsed;
						}

						//wake up everyone if we need to (note: this thread can pick up the last task queued)
						if (tasksExpired && (mQueue.size() > 1))
							mWaitCV.notify_all();
					}

					//either take one task from the queue or wait
					if (mQueue.empty())
					{
						if (mWaitQueue.empty())
							mWaitCV.wait(lock);
						else
							mWaitCV.wait_for(lock, std::get<0>(mWaitQueue.front()));

						continue; //restart (have to re-check wait queue)
					}

					//take one task out
					taskHandle = mQueue.front();
					mQueue.pop_front();
				}

				assert((taskHandle.taskId > 0) && taskHandle.handle);
				taskHandle.handle.resume(); //run the task

				//reaching this point, either the task finished or added something more to the queue (co_await on something)

				{
					std::lock_guard lock{mWaitMutex};

					auto it = std::find_if(mTasks.begin(), mTasks.end(), [taskId = taskHandle.taskId](const auto& task) { return (task.taskId == taskId); });
					assert(it != mTasks.end());

					if (taskHandle.handle.done() || it->coroTask.is_ready())
						mTasks.erase(it);

					if (mWaitFinishRequests > 0)
						mWaitFinishCV.notify_all();
				}
			}
		}

		void stop() noexcept
		{
			if (!mShutdownRequested.exchange(true)) mWaitCV.notify_all();
		}
	};

	class Dispatcher
	{
		size_t mId{0};
		Scheduler& mScheduler;
		std::atomic<bool> mShutdownRequested{false};

		static std::atomic<size_t> sGenDispatcherId;

	public:
		Dispatcher(Scheduler& scheduler) noexcept
		  : mScheduler{scheduler}
		{
			mId = ++sGenDispatcherId;
		}

		Dispatcher(Dispatcher& dispatcher) noexcept
		  : mScheduler{dispatcher.mScheduler}
		{
			mId = ++sGenDispatcherId;
		}

		template<class TCallback, class... TArgs>
		bool post(TCallback&& callback, TArgs&&... args)
		{
			if (mShutdownRequested)
				return false;

			return mScheduler.post(mId, std::forward<TCallback>(callback), std::forward<TArgs>(args)...);
		}

		void stop() noexcept
		{
			mShutdownRequested = true;
		}

		void wait() noexcept
		{
			mScheduler.wait(mId);
		}

		void stopAndWait() noexcept
		{
			stop();
			wait();
		}
	};
}
