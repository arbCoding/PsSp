#include "pssp_data_pool.hpp"
#include <iterator>
#include <mutex>
#include <shared_mutex>

namespace pssp
{
std::shared_ptr<sac_1c> DataPool::get_ptr(Project& project, const int data_id, const int checkpoint_id)
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
    // We need to try to clear a chunk
    if (n_data() > max_data) { clear_chunk(project); }
    // If it is not found, we need to add it to the pool
    return get_new_pointer(project, data_id, checkpoint_id);
}

// How much data is in the pool
std::size_t DataPool::n_data() const
{
    return data_pool_.size();
}

// If it is not in temporary for the correct checkpoint
// and data_id, it is loaded from the proper checkpoint
// data table
//=============================================================================
// Needs a flag to determine if it should try the temporary_data first
// or directly skip it (straight from checkpoint)
void DataPool::add_data(Project& project, const int data_id, const int checkpoint_id)
{
    if (data_id == -1 || !project.is_project) { return; }
    sac_1c sac{};
    std::lock_guard<std::shared_mutex> lock_sac(sac.mutex_);
    sac.file_name = project.get_source(data_id);
    sac.sac = project.load_temporary_sacstream(data_id, checkpoint_id);
    sac.data_id = data_id;
    // Add the pointer to the pool
    data_pool_[data_id] = std::make_unique<sac_1c>(std::move(sac));
}

// Add and return a new raw pointer
std::shared_ptr<sac_1c> DataPool::get_new_pointer(Project& project, const int data_id, const int checkpoint_id)
{
    if (data_id == -1 || !project.is_project) { return nullptr; }
    // Add it to the pool
    add_data(project, data_id, checkpoint_id);
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
        std::vector<decltype(data_pool_.begin())> to_delete{};
        // Iterator in reverse order (oldest to newest)
        for (auto rit = data_pool_.rbegin(); rit != data_pool_.rend(); ++rit)
        {
            std::shared_ptr<sac_1c>& sac_ptr = rit->second;
            {
                std::unique_lock<std::shared_mutex> lock_sac(sac_ptr->mutex_, std::try_to_lock);
                if (lock_sac.owns_lock())
                {
                    to_delete.push_back(std::next(rit).base());
                }
            }
        }
        for (const auto& data : to_delete)
        {
            data_pool_.erase(data);
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

// Find an object in the pool that is not being used and remove it
// Note that the pool is locked already whenever this is called
// so nobody can get this during the operation anyway
void DataPool::remove_unused_once(Project& project, std::size_t& removed)
{
    // Iterator in reverse order (oldest to newest)
    for (auto rit = data_pool_.rbegin(); rit != data_pool_.rend(); ++rit)
    {
        std::shared_ptr<sac_1c>& sac_ptr = rit->second;
        bool success{false};
        // Non-blocking attempt to lock the object
        {
            std::unique_lock<std::shared_mutex> lock_sac(sac_ptr->mutex_, std::try_to_lock);
            if (lock_sac.owns_lock())
            {
                success = true; 
                project.store_in_temporary_data(sac_ptr->sac, sac_ptr->data_id);
            }
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
    std::size_t target{max_data / 2};
    // Make sure we're removing a sufficiently sized chunk
    std::size_t chunk_size{(n_data() - target) / 2};
    // Make sure chunk_size is never less than 1
    chunk_size = ((1 > chunk_size) ? 1 : chunk_size);
    // This doesn't actually get used for anything other than debugging
    // need to remove this
    std::size_t removed_count{0};
    // Even if we didn't hit the target, at least we're below max_data
    // if we tried to force target, we could deadlock
    while (n_data() > max_data)
    {
        for (std::size_t i{0}; i < chunk_size; ++i)
        {
            if (n_data() <= target) { break; }
            remove_unused_once(project, removed_count);
        }
    }
}

void DataPool::return_ptr(Project& project, std::shared_ptr<sac_1c>& sac_ptr)
{
    std::lock_guard<std::mutex> lock_pool(mutex_);
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
//std::vector<int> DataPool::get_iter(const std::vector<int>& input_ids)
std::vector<int> DataPool::get_iter(Project& project)
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
    std::lock_guard<std::shared_mutex> lock_project(project.mutex);
    std::vector<int> input_ids{project.current_data_ids};
    std::sort(input_ids.begin(), input_ids.end());
    not_in_pool.reserve(input_ids.size() - in_pool.size());
    std::set_difference(input_ids.begin(), input_ids.end(), in_pool.begin(), in_pool.end(), std::back_inserter(not_in_pool));
    std::vector<int> result{};
    result.reserve(in_pool.size() + not_in_pool.size());
    std::set_union(in_pool.begin(), in_pool.end(), not_in_pool.begin(), not_in_pool.end(), std::back_inserter(result));
    return result;
}
}