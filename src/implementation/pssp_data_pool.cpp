#include "pssp_data_pool.hpp"
#include <iterator>
#include <mutex>

namespace pssp
{
std::shared_ptr<sac_1c> DataPool::get_ptr(Project& project, int data_id)
{
    // We lock the pool when requesting the data
    std::lock_guard<std::mutex> lock_pool(mutex_);
    // if data_id = -1, there is nothing to retrieve (if it is not a project, nothing to retrieve)
    if (data_id == -1 || !project.is_project) { return nullptr; }
    // If it is the pool, simply return the raw pointer
    if (data_pool_.count(data_id) > 0)
    {
        return data_pool_[data_id];
    }
    // The pool is full!
    // So we remove one unused data before adding a new one
    // Since updating boost to 1.82.0 this line crashes.
    //if (n_data() >= max_data) { clear_chunk(project); }
    if (n_data() > max_data) { clear_chunk(project); }
    // If it is not found, we need to add it to the pool
    return get_new_pointer(project, data_id);
}

// How much data is in the pool
std::size_t DataPool::n_data() const
{
    return data_pool_.size();
}

void DataPool::add_data(Project& project, int data_id)
{
    if (data_id == -1 || !project.is_project) { return; }
    sac_1c sac{};
    std::lock_guard<std::shared_mutex> lock_sac(sac.mutex_);
    sac.file_name = project.get_source(data_id);
    sac.sac = project.load_sacstream(data_id);
    sac.data_id = data_id;
    // Add the pointer to the pool
    data_pool_[data_id] = std::make_unique<sac_1c>(std::move(sac));
}

// Add and return a new raw pointer
std::shared_ptr<sac_1c> DataPool::get_new_pointer(Project& project, int data_id)
{
    if (data_id == -1 || !project.is_project) { return nullptr; }
    // Add it to the pool
    add_data(project, data_id);
    // Retrieve the raw pointer form the pool
    return data_pool_[data_id];
}

// Fully empty the pool
void DataPool::empty_pool()
{
    // Lock the pool, don't want any funny business
    std::lock_guard<std::mutex> lock_pool(mutex_);
    while (data_pool_.size() > 0)
    {
        // Iterator in reverse order (oldest to newest)
    for (auto rit = data_pool_.rbegin(); rit != data_pool_.rend(); ++rit)
    {
        std::shared_ptr<sac_1c>& sac_ptr = rit->second;
        bool success{false};
        if (sac_ptr->mutex_.try_lock())
        {
            success = true;
        }
        if (success)
        {
            // Delete the object
            sac_ptr.reset();
            auto erase_iter = std::next(rit).base();
            data_pool_.erase(erase_iter);
            break;
        }
    }
    }
    // Clear the data_pool_ map
    data_pool_.clear();
}

void DataPool::remove_data(Project& project, int data_id)
{
    if (data_id == -1 || !project.is_project) { return; }
    if (data_pool_.count(data_id) > 0)
    {
        std::shared_ptr<sac_1c>& data_ptr = data_pool_[data_id];
        // We loop until we can remove it
        while (true)
        {
            // Acquire the lock to ensure we're the only one using it!
            {
                std::unique_lock<std::shared_mutex> lock_sac(data_ptr->mutex_);
            }
            // Unlock and delete the object
            data_ptr.reset();
            data_pool_.erase((data_id));
            break;
        }
    }
}

void DataPool::reload_data(Project& project, int data_id)
{
    std::lock_guard<std::mutex> lock_pool(mutex_);
    if (data_id == -1 || !project.is_project) { return; } 
    // Remove if it exists
    remove_data(project, data_id);
    // Readd
    add_data(project, data_id);
}

// Find an object in the pool that is not being used and remove it
// Note that the pool is locked already whenever this is called
// so nobody can get this during the operation anyway
//=============================================================================
// I think I have found the solution
//=============================================================================
// The data pool should use shared_ptr, not unique_ptr
// I suspect that a unique_ptr is getting deleted while
// another process is waiting to lock it (since multi-threaded)
// so the better way to deal with it would be to pass
// shared pointers (instead of raw pointers from the unique pointers)
// and if it is marked as not in use remove it from the pool
// it'll automatically go out of scope when all the functions
// that got it end
// That will hopefully prevent things from getting stuck
//
// Issue that will raise
// An object that is processed after being removed from the pool
// the older version is stored as temporary, not the processed
// version
//
// To deal with that I would need to implement a mechanism for "returning"
// the object to the pool. If it is still in the pool, no big deal
// If it is not in the pool anymore, the pool can write that to the
// appropriate temporary file and then disown it again (since not in the pool
// couldn't be requested anyway).
//
// What if it gets requested by a different thread, after it is removed, but before
// it is returned? That should not happen, once the removal scheme has been improved
// to remove the oldest first (using a std::map instead of unordered_map).
//
// This is because we will maintain that a single-work flow will own the shared_ptr
// throughout the entire task (instead of re-requesting it at various stages). That means
// a function inside the thread will only request the pointer once and use that until it is
// done, without anybody else being allowed to use it (nor having any reason to request it).
//
// I should also provide a mechanism to make it so that operations that work on a chunk of data
// (via iterating over a std::vector<int> first work on the data that is currently in the pool,
// to reduce the amount of unloading and reloading).
//
// When acquiring that list, the pool should be locked such that it doesn't get modified while
// the list is being made.
//
// Turns out there are also std::atomic<std::shared_ptr> objects (new C++20)
// which may be useful for this problem. More research needs to be done to prevent
// shenanigans with the data-pool (I know this is tedious and won't matter for many
// workflows, but for the workflows where it will matter it'll be a huge win to have
// this implemented. This could make working on small embedded systems possible, things like
// processing data on the raspberry pi, or old/weak computers would really benefit from this
// as would situations where either the possible data in the project is absolutely huge,
// or the seismograms themselves are quite huge (high sampling rate for long time,
// such as with optical cable seismometers, which are becoming more popular))
//
// So while it might seem like a step backward to focus on this aspect of the program
// (as it broke a lot of functionality) it'll ultimately have a huge payoff in the future.
// It could even allow the user to tune how much of their system they're willing to commit
// to their analysis (at the sacrifice of speed) so that they don't lockdown their machine
// exclusively for analysis. Making the data-pool smart enough to be safe and relatively
// speedy will pay dividends in the future. It's better to do this now, while the code-base
// is still fairly small than later on when the changes would be even more difficult to propagate
// through the code. And for projects+computer combinations where fitting all the data into
// ram is a non-issue, this will have a negligably small impact on the performance. 
void DataPool::remove_unused_once(Project& project, std::size_t& removed)
{
    // Iterator in reverse order (oldest to newest)
    for (auto rit = data_pool_.rbegin(); rit != data_pool_.rend(); ++rit)
    {
        std::shared_ptr<sac_1c>& sac_ptr = rit->second;
        bool success{false};
        if (sac_ptr->mutex_.try_lock())
        {
            success = true;
            project.store_in_temporary_data(sac_ptr->sac, sac_ptr->data_id);
        }
        if (success)
        {
            // Delete the object
            sac_ptr.reset();
            auto erase_iter = std::next(rit).base();
            data_pool_.erase(erase_iter);
            ++removed;
            break;
        }
    }
}

// Clear a chunk of the data_pool (say half)
void DataPool::clear_chunk(Project& project)
{
    std::size_t target{n_data() / 2};
    std::size_t chunk_size{target / 4};
    std::size_t removed_count{0};
    while (n_data() > target)
    {
        for (std::size_t i{0}; i < chunk_size; ++i)
        {
            if (n_data() <= target) { break; }
            remove_unused_once(project, removed_count);
        }
        if (removed_count <= chunk_size)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void DataPool::return_ptr(Project& project, std::shared_ptr<sac_1c>& sac_ptr)
{
    for (auto& element : data_pool_)
    {
        std::shared_ptr<sac_1c> current_data = element.second;
        // Same pointers, it is in the data-pool, do nothing
        if (current_data == sac_ptr) { return; }
    }
    // It was not in the data-pool
    project.store_in_temporary_data(sac_ptr->sac, sac_ptr->data_id);
}

// Use the data that is currently in the pool first
// then load other data
std::vector<int> DataPool::get_iter(const std::vector<int>& input_ids)
{
    // lock the pool
    std::lock_guard<std::mutex> lock_pool(mutex_);
    // We know that the input_ids are sorted because that happens whenever data is read-in
    std::vector<int> in_pool{};
    for (auto& element : data_pool_)
    {
        in_pool.push_back(element.first);
    }
    std::sort(in_pool.begin(), in_pool.end());
    std::vector<int> not_in_pool{};
    not_in_pool.reserve(input_ids.size() - in_pool.size());
    std::set_difference(input_ids.begin(), input_ids.end(), in_pool.begin(), in_pool.end(), std::back_inserter(not_in_pool));
    std::vector<int> result{};
    result.reserve(in_pool.size() + not_in_pool.size());
    std::set_union(in_pool.begin(), in_pool.end(), not_in_pool.begin(), not_in_pool.end(), std::back_inserter(result));
    return result;
}
}