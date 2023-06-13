#include "pssp_data_pool.hpp"
#include <mutex>

namespace pssp
{
sac_1c* DataPool::get_pointer(Project& project, int data_id)
{
    // if data_id = -1, there is nothing to retrieve (if it is not a project, nothing to retrieve)
    if (data_id == -1 || !project.is_project) { return nullptr; }
    // We lock the pool when requesting the data
    std::lock_guard<std::mutex> lock_pool(mutex_);
    // If it is the pool, simply return the raw pointer
    if (data_pool_.count(data_id) > 0)
    {
        data_pool_[data_id]->in_use = true;
        return data_pool_[data_id].get();
    }
    // The pool is full!
    // So we remove one unused data before adding a new one
    if (n_data() >= max_data) { clear_chunk(project); }
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
sac_1c* DataPool::get_new_pointer(Project& project, int data_id)
{
    if (data_id == -1 || !project.is_project) { return nullptr; }
    // Add it to the pool
    add_data(project, data_id);
    // Retrieve the raw pointer form the pool
    data_pool_[data_id]->in_use = true;
    return data_pool_[data_id].get();
}

// Fully empty the pool
void DataPool::empty_pool()
{
    // Lock the pool, don't want any funny business
    std::lock_guard<std::mutex> lock_pool(mutex_);
    while (data_pool_.size() > 0)
    {
        for (auto& element : data_pool_)
        {
            std::unique_ptr<sac_1c>& data_ptr = element.second;
            // Acquire the lock to ensure we're the only one using it!
            {
                std::lock_guard<std::shared_mutex> lock_sac(data_ptr->mutex_);
            }
            // Unlock and delete the object
            if (!data_ptr->in_use) { data_ptr.reset();  data_pool_.erase(element.first); }
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
        std::unique_ptr<sac_1c>& data_ptr = data_pool_[data_id];
        // We loop until we can remove it
        while (true)
        {
            // Acquire the lock to ensure we're the only one using it!
            {
                std::unique_lock<std::shared_mutex> lock_sac(data_ptr->mutex_);
            }
            if (!data_ptr->in_use)
            {
                // Unlock and delete the object
                data_ptr.reset();
                data_pool_.erase((data_id));
                break;
            }
        }
    }
}

void DataPool::reload_data(Project& project, int data_id)
{
    if (data_id == -1 || !project.is_project) { return; }
    std::lock_guard<std::mutex> lock_pool(mutex_);
    // Remove if it exists
    remove_data(project, data_id);
    // Readd
    add_data(project, data_id);
}

// Find an object in the pool that is not being used and remove it
// Note that the pool is locked already whenever this is called
// so nobody can get this during the operation anyway
void DataPool::remove_unused_once(Project& project)
{
    bool success{false};
    // Search until we are able to remove one
    while (!success)
    {
        for (auto& element : data_pool_)
        {
            std::unique_ptr<sac_1c>& data_ptr = element.second;
            if (data_ptr->mutex_.try_lock() && !data_ptr->in_use)
            {
                project.store_in_temporary_data(data_ptr->sac, data_ptr->data_id);
                success = true;
            }
            if (success)
            {
                // Delete the object
                data_ptr.reset();
                data_pool_.erase(element.first);
                // Break out of the for loop
                break;
            }
        }
    }
}

// Clear a chunk of the data_pool (say half)
void DataPool::clear_chunk(Project& project)
{
    std::size_t target{max_data / 2};
    while (n_data() > target)
    {
        remove_unused_once(project);
    }
}
}