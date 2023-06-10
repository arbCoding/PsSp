#include "pssp_threadpool.hpp"

namespace pssp
{
//-----------------------------------------------------------------------------
// Public Stuff
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------
// Parameterized constructor
//------------------------------------------------------------------------
ThreadPool::ThreadPool(std::size_t n_threads) : n_threads_(n_threads), n_busy_threads_{0}, stop_{false}
{
    // Using bind to create a forwarding call operator
    // &ThreadPool::worker_thread is the callable object (memory location
    // of worker_thread)
    // We're binding it to the ThreadPool (this)
    // It ensures the worker_thread has access to components of ThreadPool
    // (the task-queue and the condition variable)
    for (std::size_t i = 0; i < n_threads; ++i) { threads_.emplace_back(std::bind(&ThreadPool::worker_thread, this)); }
}
//------------------------------------------------------------------------
// End Parameterized constructor
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Deconstructor
//------------------------------------------------------------------------
ThreadPool::~ThreadPool()
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
    for (auto& thread : threads_) { thread.join(); }
}
//------------------------------------------------------------------------
// End Deconstructor
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Getters
//------------------------------------------------------------------------
std::size_t ThreadPool::n_threads_total() const { return n_threads_; }

std::size_t ThreadPool::n_busy_threads() const { return n_busy_threads_.load(); }

std::size_t ThreadPool::n_idle_threads() const { return n_threads_ - n_busy_threads_.load(); }

std::size_t ThreadPool::n_tasks()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return tasks_.size();
}
//------------------------------------------------------------------------
// End Getters
//------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// End Public Stuff
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Private Stuff
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------
// Worker Thread
//------------------------------------------------------------------------
void ThreadPool::worker_thread()
{
    // The while lloop without the condition variable would always be spinning
    // its wheels looking for work (if there were none)
    while (true)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // We use this to freeze the thread
        // It only unfreezes if the task queue is non-empty and it gets notified
        // Or if it is told to stop and gets notified
        condition_.wait(lock, [this]() { return !tasks_.empty() || stop_; });
        // Exit cleanly
        if (stop_) { break; }
        // We're busy
        ++n_busy_threads_;
        // We move the task from the queue (tasks_) to our local variable task
        // This leaves tasks_.front() unspecified (empty, but still there)
        auto task = std::move(tasks_.front());
        // Remove the empty front-most element from the queue
        tasks_.pop();
        // Unlock the mutex
        lock.unlock();
        // Do the task
        task();
        // No long busy
        --n_busy_threads_;
    }
}
//------------------------------------------------------------------------
// End Worker Thread
//------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// End Private Stuff
//-----------------------------------------------------------------------------
}