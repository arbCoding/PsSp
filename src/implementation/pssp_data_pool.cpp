#include "pssp_data_pool.hpp"
#include "pssp_misc.hpp"
#include <mutex>

namespace pssp
{
std::shared_ptr<sac_1c> DataPool::get_ptr(Project& project, const int data_id, const int checkpoint_id, const bool from_checkpoint)
{
    // We lock the pool when requesting the data
    std::scoped_lock lock_pool(mutex_);
    // if data_id = -1, there is nothing to retrieve (if it is not a project, nothing to retrieve)
    if (data_id == -1 || !project.is_project) { return nullptr; }
    // If it is the pool, simply return the raw pointer
    if (data_pool_.contains(data_id) && !from_checkpoint) { return data_pool_[data_id]; }
    // The pool is full!
    // We need to try to clear a chunk
    if (n_data() > max_data) { clear_chunk(project); }
    // If it is not found, we need to add it to the pool
    return get_new_pointer(project, data_id, checkpoint_id, from_checkpoint);
}

// How much data is in the pool
std::size_t DataPool::n_data() const { return data_pool_.size(); }

// Add the data to the data-pool with a unique_ptr
void DataPool::add_data(Project& project, const int data_id, const int checkpoint_id, const bool from_checkpoint)
{
    if (data_id == -1 || !project.is_project) { return; }
    sac_1c sac{};
    // Prevent data race
    std::scoped_lock lock_project(project.mutex);
    sac.file_name = project.get_source(data_id);
    sac.sac = project.load_temporary_sacstream(data_id, checkpoint_id, from_checkpoint);
    sac.data_id = data_id;
    // Add the pointer to the pool
    data_pool_[data_id] = std::make_unique<sac_1c>(sac);
}

// Add and return a new raw pointer
std::shared_ptr<sac_1c> DataPool::get_new_pointer(Project& project, const int data_id, const int checkpoint_id, const bool from_checkpoint)
{
    if (data_id == -1 || !project.is_project) { return nullptr; }
    // Add it to the pool
    add_data(project, data_id, checkpoint_id, from_checkpoint);
    // Retrieve the raw pointer form the pool
    return data_pool_[data_id];
}

// Fully empty the pool
void DataPool::empty_pool()
{
    std::scoped_lock lock_pool(mutex_);
    int count{0};
    constexpr int attempt_limit{10};
    while (!data_pool_.empty())
    {
        std::vector<int> to_delete{};
        // Iterator in reverse order (oldest to newest)
        for (auto rit = data_pool_.rbegin(); rit != data_pool_.rend(); ++rit)
        {
            std::shared_ptr<sac_1c> const& sac_ptr = rit->second;
            {
                std::unique_lock lock_sac(sac_ptr->mutex_, std::try_to_lock);
                if (!sac_ptr || lock_sac.owns_lock()) { to_delete.push_back(rit->first); }
            }
        }
        if (!to_delete.empty() && count >= attempt_limit) { break; }
        for (const auto& data : to_delete)
        {
            data_pool_[data].reset();
            data_pool_.erase(data);
        }
        ++count;
    }
    // Clear the data_pool_ map
    data_pool_.clear();
}

void DataPool::remove_data(const Project& project, int data_id)
{
    if (data_id == -1 || !project.is_project) { return; }
    while (true)
    {
        if (data_pool_.contains(data_id))
        {
            std::shared_ptr<sac_1c>& data_ptr = data_pool_[data_id];
            // We loop until we can remove it
            // Acquire the lock to ensure we're the only one using it!
            {
                std::unique_lock lock_sac(data_ptr->mutex_);
            }
            // Unlock and delete the object
            data_ptr.reset();
            data_pool_.erase(data_id);
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
            std::unique_lock lock_sac(sac_ptr->mutex_, std::try_to_lock);
            if (lock_sac.owns_lock())
            {
                success = true;
                std::scoped_lock lock_project(project.mutex);
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

void DataPool::return_ptr(Project& project, const std::shared_ptr<sac_1c>& sac_ptr)
{
    std::scoped_lock lock_pool(mutex_);
    for (const auto& [key, current_data] : data_pool_)
    {
        // Same pointers, it is in the data-pool, do nothing
        if (current_data == sac_ptr) { return; }
    }
    // It was not in the data-pool
    std::scoped_lock lock_project(project.mutex);
    project.store_in_temporary_data(sac_ptr->sac, sac_ptr->data_id);
}

// Use the data that is currently in the pool first
// then load other data
//std::vector<int> DataPool::get_iter(const std::vector<int>& input_ids)
std::vector<int> DataPool::get_iter(Project& project)
{
    // lock the pool
    std::scoped_lock lock_pool(mutex_);
    // We know that the input_ids are sorted because that happens whenever data is read-in
    std::vector<int> in_pool{};
    for (const auto& [key, sac_ptr] : data_pool_)
    {
        in_pool.push_back(key);
    }
    std::ranges::sort(in_pool);
    std::vector<int> not_in_pool{};
    std::scoped_lock lock_project(project.mutex);
    std::vector<int> input_ids{project.current_data_ids};
    std::ranges::sort(input_ids);
    not_in_pool.reserve(input_ids.size() - in_pool.size());
    std::ranges::set_difference(input_ids, in_pool, std::back_inserter(not_in_pool));
    std::vector<int> result{};
    result.reserve(in_pool.size() + not_in_pool.size());
    std::ranges::set_union(in_pool, not_in_pool, std::back_inserter(result));
    return result;
}
}