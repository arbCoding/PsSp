#ifndef PSSP_DATA_POOL_HPP_20230612
#define PSSP_DATA_POOL_HPP_20230612

//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
#include "pssp_projects.hpp"
#include <sac_stream.hpp>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
//-----------------------------------------------------------------------------
// End Include statements
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
// At some point, memory management WILL be an issue
// To expect that all projects will always be able to fit all of their data
//  into memory is naive at best.
//
// To this end, a pool for data objects need to be made that will handle
//  the memory management. Initially it will be crude, I hope in the future
//  to add smarter functionality (such as setting the default size of the pool
//  to be a fraction of the maximum system RAM. There are issues with doing that
//  across different OSes though, which I will deal with later)
//
// This can go by any number of names: a memory pool, and object pool, a pointer
//  pool, etc. I'm going to call it a data pool because it represents our
//  mechanism for accessing data.
//
// The sac_1c struct is already designed to be thread-safe, with a shared_mutex
//  that can be locked in shared mode (read-only, multi-thread) or exclusive mode
//  (read/write, single-thread).
//
// Memory management at the basic level will be done with RAII (Resource Acquisition
//  Is Initialization), by using std::unique_ptr's to hold the references to the actual
//  data.
//
// A task can request a piece of data by it's unique_data, which is assigned
//  by the project database via SQLite3. If it is in the data pool, a raw pointer
//  to the data is returned (the task cannot own the data). If it is not in the pool
//  it is added to the pool and then the raw pointer is returned. If the pool is full,
//  then the pool will need to first remove an object from the pool (that is not being used)
//  so that this object can instead be added.
//
// When removing an object from the pool, we need to preserve it's present state
//  because it has not yet been checkpointed, but not in memory. To that end
//  I think it is appropriate to temporarily store it in the project database
//  in a unique table that is designed to maintain uncheckpointed copies of data.
//
// So when loading an object for the pool, it should first try to take it from that table
//  and if not, to retrieve the version that is associated with the current checkpoint.
//
// That means when a checkpoint is made, we can dump everything in the pool into
//  the temporary table and then move the rows from that table to their appropriate places
//  in the data tables.
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------
namespace pssp
{
//-----------------------------------------------------------------------------
// sac_1c struct
//-----------------------------------------------------------------------------
struct sac_1c
{
    std::string file_name{};
    SAC::SacStream sac{};
    std::shared_mutex mutex_{};
    int data_id{};

    sac_1c() : file_name(), sac(), mutex_(), data_id() {}
    // Copy constructor
    sac_1c(const sac_1c& other)
    {
        file_name = other.file_name;
        sac = other.sac;
        data_id = other.data_id;
        // Don't copy the mutex
    }
    // Assignment operator
    sac_1c& operator=(const sac_1c& other)
    {
        if (this != &other)
        {
            file_name = other.file_name;
            sac = other.sac;
            data_id = other.data_id;
            // Don't assign the mutex
        }
        return *this;
    }
};
//-----------------------------------------------------------------------------
// End sac_1c struct
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// DataPool class
//-----------------------------------------------------------------------------
class DataPool
{
public:
    // Parameterized constructor basd upon allowed size of pool
    //=========================================================================
    // max_data must allow at least as many objects in the pool
    // as the number of threads that will be accessing them
    // once it goes below that number, deadlocks begin to occur
    // (so on MacOS, I have 7-8 threads (1 for gui, 7 for processing))
    // If I allow up to 7 (without showing plots of data) it is perfectly fine
    // If I allow up to 6, it can experience deadlocks (not always so can be hard to track
    // down)
    //=========================================================================
    // In the future I'd like to implement smart resizing of the data-pool
    // if we're close to max-size and the system still has tons of memory
    // left, we should increase max-size.
    // If we're below max-size and the system is low on memory,
    // we should decrease max-size
    // But, max size must never go below the number of threads
    // in the thread-pool
    DataPool(std::size_t max_data_ = 1) : max_data(max_data_) {}
    // Request a pointer for the data (raw pointer, only the pool owns the data!)
    std::shared_ptr<sac_1c> get_ptr(Project& project, int data_id);
    // How much data is in the pool
    std::size_t n_data() const;
    // Fully empty the pool
    void empty_pool();
    void remove_data(Project& project, int data_id);
    void reload_data(Project& project, int data_id);
    std::size_t max_data{};
    // Add data to the pool
    void add_data(Project& project, int data_id);
    // Function for returning data to data-pool
    void return_ptr(Project& project, std::shared_ptr<sac_1c>& sac_ptr);
    //std::vector<int> get_iter(const std::vector<int>& input_ids);
    std::vector<int> get_iter(Project& project);
private:
    std::mutex mutex_{};
    std::map<int, std::shared_ptr<sac_1c>> data_pool_{};
    // Add and return a new raw pointer
    std::shared_ptr<sac_1c> get_new_pointer(Project& project, int data_id);
    // Find an object in the pool that is not being used and remove it
    void remove_unused_once(Project& project, std::size_t& removed);
    void clear_chunk(Project& project);
};
//-----------------------------------------------------------------------------
// End DataPool class
//-----------------------------------------------------------------------------
}

#endif