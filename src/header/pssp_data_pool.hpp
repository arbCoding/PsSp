#ifndef PSSP_DATA_POOL_HPP_20230612
#define PSSP_DATA_POOL_HPP_20230612

//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
#include "pssp_projects.hpp"
#include <sac_stream.hpp>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <unordered_map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <atomic>
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
    // Let's be safe and assume a huge size for seismograms
    // say each seismogram is 100Hz, 24 hours. At 40 Hz, 1 hour, it is ~1Mb per
    // so at 2.5 the sampling rate, that is 1 hour at ~2.5Mb (call it 3Mb) per 1 hour
    // seismogram. So ~75Mb per seismogram (call it 100Mb per). That would make
    // 10 seismograms 1 Gb of Ram. So in that scenario, the DataPool must be fairly small
    // maybe only a maximum of 10-50 seismograms at a time.
    // If we worked with 1 hour seismograms at 40 Hz (as I am testing on) then it is
    // 1000 seismograms to 1 Gb of Ram. In that scenario, the DataPool can be fairly large
    // Since I'm using 700 seismograms that are 1 hour long at 40 Hz, let's first set the maximum
    // size to 1000 (so that it is all in memory). Get the basics down, then reduce to 500 and see
    // what happens (and implement the hot loading/unload of memory).
    DataPool(std::size_t max_data_ = 1000) : max_data(max_data_) {}
    // Request a pointer for the data (raw pointer, only the pool owns the data!)
    sac_1c* get_pointer(Project& project, int data_id);
    // How much data is in the pool
    std::size_t n_data() const;
    // Fully empty the pool
    void empty_pool();
    void remove_data(Project& project, int data_id);
    void reload_data(Project& project, int data_id);
    std::size_t max_data{};
    // Add data to the pool
    void add_data(Project& project, int data_id);
private:
    std::mutex mutex_{};
    std::unordered_map<int, std::unique_ptr<sac_1c>> data_pool_{};
    // Add and return a new raw pointer
    sac_1c* get_new_pointer(Project& project, int data_id);
};
//-----------------------------------------------------------------------------
// End DataPool class
//-----------------------------------------------------------------------------
}

#endif