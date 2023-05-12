#ifndef PSSP_THREAD_POOL_H
#define PSSP_THREAD_POOL_H

// std::thread
#include <thread>
// std::vector
#include <vector>
// std::queue
#include <queue>
// std::mutex, std::unique_lock
#include <mutex>
// std::condition_variable
#include <condition_variable>
// Needed for `typname Function` and `typename... Args` template components
#include <functional>

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
// The `ThreadPool` class provides a way of sending tasks off to workers (threads).
// It follows a 'Producer-Consumer' design pattern, using what is sometimes
// called a 'work-stealing' approach to parallelism.
//
// We build a task-queue of jobs to do (producer) and we tell our workers
// (threads) to come and get the work when they can. A worker can only hold
// one task at a time. When the worker is done with that task, they grab another.
//
// If there are no tasks currently available (checked with the condition
// varaible), the worker (thread) goes to sleep until it is notified 
// (via the condition variable) to get back to work or that the program is stopping
// (important to tell that we're stopping otherwise the threads will hang and prevent
// release of the system resources).
//
// To use this: early in your program, instantiate a ThreadPool with an appropriate
// number of threads (I use std::thread::hardware_concurrency() - 1 by default,
// that is a one-to-one ratio of threads allowed by the hardware to threads in the code
// [-1 because the program starts on the main thread]).
//
// Then you issue tasks to it using the `enqueue` function.
//
// e.g.:
//
// pssp::ThreadPool thread_pool{};
// thread_pool.enqueue(some_function, argument_1, std::ref(argument_2), ...)
//
// If passing-by-reference of a non-integral type (complex custom type) then the
// std::ref(object) is necessary.
//
// Remember, smart usage of mutex's is how you prevent data-races. If you don't
// prevent data-races, you'll probably get super frustrating and non-intuitive
// bugs/crashes. When in doubt, first lock it out.
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

namespace pssp
{
//-----------------------------------------------------------------------------
// ThreadPool class
//-----------------------------------------------------------------------------
  class ThreadPool 
  {
  public:
    //-------------------------------------------------------------------------
    // Parameterized constructor
    //-------------------------------------------------------------------------
    // Tell me how many threads you want and I'll add them to the vector of
    // threads
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency() - 1)
    {
      for (std::size_t i = 0; i < num_threads; ++i)
      {
        // Using bind to create a forwarding call operator
        // &ThreadPool::worker_thread is the callable object (memory location
        // of worker_thread)
        // We're binding it to the ThreadPool (this)
        // It ensures the worker_thread has access to components of ThreadPool
        // (the task-queue and the condition variable)
        threads_.emplace_back(std::bind(&ThreadPool::worker_thread, this));
      }
    }
    //-------------------------------------------------------------------------
    // End Parameterized constructor
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Deconstructor
    //-------------------------------------------------------------------------
    ~ThreadPool()
    {
      // By using the limited scope of this small chunk we're enforcing RAII
      // (Resource Acquisition Is Initialization)
      // The unique_lock goes out of scope at the end of the sub-block
      // (prevents the need to use raw mutex.lock()/mutex.unlock())
      {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
      }
      // Tell everyone we need to stop
      condition_.notify_all();
      // Wait until we can join all the threads
      for (auto& thread : threads_)
      {
        thread.join();
      }
    }
    //-------------------------------------------------------------------------
    // End Deconstructor
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Enqueue function
    //-------------------------------------------------------------------------
    // What an awesome template. Can template it to take different functions
    // and to take an arbitrary number of different arguments
    // So long as the function and arguments are consistent with each-other
    // it's all good (though you may need to use std::ref() to pass-by-reference
    // on non-integral types)
    template <typename Function, typename... Args>
    void enqueue(Function&& func, Args&&... args)
    {
      std::unique_lock<std::mutex> lock(mutex_);
      // Add it to the queue!
      tasks_.emplace(std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
      // Let one worker know they have stuff to do
      condition_.notify_one();
    }
    //-------------------------------------------------------------------------
    // End Enqueue function
    //-------------------------------------------------------------------------
  
  private:
    //-------------------------------------------------------------------------
    // Worker Thread
    //-------------------------------------------------------------------------
    void worker_thread()
    {
      // The while lloop without the condition variable would always be spinning
      // its wheels looking for work (if there were none)
      while (true)
      {
        std::unique_lock<std::mutex> lock(mutex_);
        // We use this to freeze the thread
        // In only unfreezes if the task queue is non-empty and it gets notified
        // Or if it is told to stop and gets notified
        condition_.wait(lock, [this]() { return !tasks_.empty() || stop_; });
        // Exit cleanly
        if (stop_)
        {
          break;
        }
        // We move the task from the queue (tasks_) to our local variable task
        // This leaves tasks_.front() unspecified (empty, but still there)
        auto task = std::move(tasks_.front());
        // Remove the empty front-most element from the queue
        tasks_.pop();
        // Unlock the mutex
        lock.unlock();
        // Do the task
        task();
      }
    }
    //-------------------------------------------------------------------------
    // End Worker Thread
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Private internal variables
    //-------------------------------------------------------------------------
    // Vector of threads
    std::vector<std::thread> threads_{};
    // Function queue of tasks to do
    std::queue<std::function<void()>> tasks_{};
    // Mutex for safety
    std::mutex mutex_{};
    // Condition variable for waiting patiently
    std::condition_variable condition_{};
    // Stop flag
    bool stop_{false};
    //-------------------------------------------------------------------------
    // Private internal variables
    //-------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// End ThreadPool class
//-----------------------------------------------------------------------------
}
#endif