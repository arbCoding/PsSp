#ifndef PSSP_THREAD_POOL_HPP_20230610
#define PSSP_THREAD_POOL_HPP_20230610

//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
// Thread-safe integral type variables
#include <atomic>
// std::condition_variable
#include <condition_variable>
// Needed for `typname Function` and `typename... Args` template components
#include <functional>
// std::mutex, std::unique_lock
#include <mutex>
// std::queue
#include <queue>
// std::thread
#include <thread>
// std::vector
#include <vector>
//-----------------------------------------------------------------------------
// End Include statements
//-----------------------------------------------------------------------------

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
// Remember, smart usage of mutexes is how you prevent data-races. If you don't
// prevent data-races, you'll probably get super frustrating and non-intuitive
// bugs/crashes. When in doubt, first lock it out.
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ToDo
//-----------------------------------------------------------------------------
// Up-to-date as of 13 May 2023
// 1) Add a public kill-switch to call on shutdown of the program using a
//    ThreadPool
//-----------------------------------------------------------------------------
// End ToDo
//-----------------------------------------------------------------------------

namespace pssp
{
//-----------------------------------------------------------------------------
// ThreadPool class
//-----------------------------------------------------------------------------
class ThreadPool 
{
public:
// Parameterized constructor
// Tell me how many threads you want and I'll add them to the vector of
// threads
explicit ThreadPool(std::size_t n_threads = std::thread::hardware_concurrency() - 1);
// Destructor
~ThreadPool();
//========================================================================
// SPECIAL CASE
//========================================================================
// Normally I can split between interface and implementation
// But this template requires it to be entirely in the header file
//------------------------------------------------------------------------
// Enqueue function
//------------------------------------------------------------------------
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
    // Intentionally ignoring the return value of bind (CppCheck warning) as it isn't needed in this context
    tasks_.emplace(std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
    // Let one worker know they have stuff to do
    condition_.notify_one();
}
//------------------------------------------------------------------------
// End Enqueue function
//------------------------------------------------------------------------
//========================================================================
// END SPECIAL CASE
//========================================================================
// Getters
// Declared as constant because they don't modify state, just check it
std::size_t n_threads_total() const;
std::size_t n_busy_threads() const;
std::size_t n_idle_threads() const;
std::size_t n_tasks();
private:
// Worker thread
void worker_thread();
//-------------------------------------------------------------------------
// Private internal variables
//-------------------------------------------------------------------------
std::size_t n_threads_;
std::atomic<std::size_t> n_busy_threads_{0};
// Vector of threads
std::vector<std::thread> threads_{};
// Function queue of tasks to do
std::queue<std::function<void()>> tasks_{};
// Mutex for safety
std::mutex mutex_{};
// Condition variable for waiting patiently
std::condition_variable condition_{};
// Stop flag
std::atomic<bool> stop_{false};
//-------------------------------------------------------------------------
// Private internal variables
//-------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// End ThreadPool class
//-----------------------------------------------------------------------------
}
#endif
