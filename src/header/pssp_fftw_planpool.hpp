#ifndef PSSP_FFTW_PLANPOOL_HPP_20230610
#define PSSP_FFTW_PLANPOOL_HPP_20230610

// FFTW3 library
#include <fftw3.h>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <unordered_map>
#include <mutex>
// Needed by FFTW library
#include <cmath>

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
// FFTW3 is thread-safe, except for plan creation and destruction.
// See their discussion of the topic here:
//  https://www.fftw.org/fftw3_doc/Thread-safety.html
//
// To that end, we use a standard mutex (a semaphore locking mechanism)
//  to handle plan creation and destruction.
//
// The plan pool can be passed by reference to any number of threads
//  which can then try to acquire an appropriate plan (it will call the right
//  creation function if necessary) without issue
//
// Plans can be used in parallel without issue as execute doesn't modify
//  the plans.
//
// Information on FFTW planning flags can be found here:
//  https://www.fftw.org/fftw3_doc/Planner-Flags.html
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

namespace pssp
{
class FFTWPlanPool
{
    public:
        // Empty constructor
        FFTWPlanPool() {}
        // Empty plan pool
        void empty_pool();
        // Deconstructor
        ~FFTWPlanPool();
        // Forward plans
        fftw_plan acquire_fft_plan(const std::size_t size);
        // Inverse plans
        fftw_plan acquire_ifft_plan(const std::size_t size);
        // Get number of plans in planpool
        std::size_t n_plans() const;
        // Make fft plan
        fftw_plan make_fft_plan(const std::size_t size);
        // Make ifft plan
        fftw_plan make_ifft_plan(const std::size_t size);
        // Make both and just keep it in the pool
        void make_fft_ifft_plan_pair(const std::size_t size);
    private:
        // Unordered_map of plans
        std::unordered_map<std::size_t, fftw_plan> fft_plans_;
        std::unordered_map<std::size_t, fftw_plan> ifft_plans_;
        // Mutex for locking
        std::mutex mutex_;
};
}

#endif