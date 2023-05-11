#ifndef PSSP_THREAD_POOL_H
#define PSSP_THREAD_POOL_H

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace pssp
{
class ThreadPool {
public:
  ThreadPool(size_t num_threads)
  {
    for (size_t i = 0; i < num_threads; ++i)
    {
      threads_.emplace_back(std::bind(&ThreadPool::worker_thread, this));
    }
  }

  ~ThreadPool()
  {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      stop_ = true;
    }
    condition_.notify_all();
    for (auto& thread : threads_)
    {
      thread.join();
    }
  }

  template <typename Function, typename... Args>
  void enqueue(Function&& func, Args&&... args)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    tasks_.emplace(std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
    condition_.notify_one();
  }
  
private:
  void worker_thread()
  {
    while (true)
    {
      std::unique_lock<std::mutex> lock(mutex_);
      condition_.wait(lock, [this]() { return !tasks_.empty() || stop_; });
      if (stop_)
      {
        break;
      }
      auto task = std::move(tasks_.front());
      tasks_.pop();
      lock.unlock();
      task();
    }
  }

  std::vector<std::thread> threads_;
  std::queue<std::function<void()>> tasks_;
  std::mutex mutex_;
  std::condition_variable condition_;
  bool stop_ = false;
};
}

#endif
