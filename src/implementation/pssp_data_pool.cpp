#include "pssp_data_pool.hpp"

namespace pssp
{
sac_1c* DataPool::get_pointer(Project& project, int data_id)
{
    // if data_id = -1, there is nothing to retrieve (if it is not a project, nothing to retrieve)
    if (data_id == -1 || !project.is_project) { return nullptr; }
    // We lock the pool when requesting the data
    std::lock_guard<std::mutex> lock_pool(mutex_);
    // If it is the pool, simply return the raw pointer
    if (data_pool_.count(data_id) > 0) { return data_pool_[data_id].get(); }
    // We'll handle managing the size of the pool later
    // For now, assume it fits
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
    return data_pool_[data_id].get();
}

// Fully empty the pool
void DataPool::empty_pool()
{
    // Lock the pool, don't want any funny business
    std::lock_guard<std::mutex> lock_pool(mutex_);
    for (auto& element : data_pool_)
    {
        std::unique_ptr<sac_1c>& data_ptr = element.second;
        // Acquire the lock to ensure we're the only one using it!
        {
            std::unique_lock<std::shared_mutex> lock_sac(data_ptr->mutex_);
        }
        // Unlock and delete the object
        data_ptr.reset();
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
        // Acquire the lock to ensure we're the only one using it!
        {
            std::unique_lock<std::shared_mutex> lock_sac(data_ptr->mutex_);
        }
        // Unlock and delete the object
        data_ptr.reset();
    }
}

void DataPool::reload_data(Project& project, int data_id)
{
    if (data_id == -1 || !project.is_project) { return; }
    // Remove if it exists
    remove_data(project, data_id);
    // Readd
    add_data(project, data_id);
}
}