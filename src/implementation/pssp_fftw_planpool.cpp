#include "pssp_fftw_planpool.hpp"

namespace pssp
{
//-----------------------------------------------------------------------------
// Empty plan pool
//-----------------------------------------------------------------------------
void FFTWPlanPool::empty_pool()
{
    std::lock_guard<std::mutex> lock_pool(mutex_);
    for (auto& entry : fft_plans_) { fftw_destroy_plan(entry.second); }
    fft_plans_.clear();
    for (auto& entry: ifft_plans_) { fftw_destroy_plan(entry.second); }
    ifft_plans_.clear();
}
//-----------------------------------------------------------------------------
// End Empty plan pool
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Desctruction!
//-----------------------------------------------------------------------------
FFTWPlanPool::~FFTWPlanPool()
{
    empty_pool();
}
//-----------------------------------------------------------------------------
// End Desctruction!
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Acquire FFT plan
//-----------------------------------------------------------------------------
// If the plan doesn't exist, create and return it
// If the plan does exist, return it
// Only plan creation and destruction are not thread-safe, so we need the semaphore lock!
fftw_plan FFTWPlanPool::acquire_fft_plan(const std::size_t size)
{
    auto it = fft_plans_.find(size);
    if (it == fft_plans_.end())
    {
        fftw_plan plan = make_fft_plan(size);
        return plan;
    }
    else { return it->second; }
}
//-----------------------------------------------------------------------------
// End Acquire FFT plan
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Acquire Inverse FFT plan
//-----------------------------------------------------------------------------
fftw_plan FFTWPlanPool::acquire_ifft_plan(const std::size_t size)
{
    auto it = ifft_plans_.find(size);
    if (it == ifft_plans_.end())
    {
        fftw_plan plan = make_ifft_plan(size);
        return plan;
    }
    else { return it->second; }
}
//-----------------------------------------------------------------------------
// End Acquire Inverse FFT plan
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// How many plans do we have total?
//-----------------------------------------------------------------------------
std::size_t FFTWPlanPool::n_plans() const
{
    return (fft_plans_.size() + ifft_plans_.size());
}
//-----------------------------------------------------------------------------
// End How many plans do we have total?
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Make FFT plan safely
//-----------------------------------------------------------------------------
fftw_plan FFTWPlanPool::make_fft_plan(const std::size_t size)
{
    std::lock_guard<std::mutex> lock_pool(mutex_);
    // r2c is always FFTW_FORWARD (real-to-complex) (forward FFT)
#if defined(__APPLE__)
    // FFTW_MEASURE takes more time than FFTW_ESTIMATE, but is more optimal on execution
    // Because the plan may be used multiple times, might as well use measure
    fftw_plan plan = fftw_plan_dft_r2c_1d(size, nullptr, nullptr, FFTW_MEASURE);
#elif defined(__linux__)
    // FFTW_MEASURE crashes on Linux (fine on MacOS)
    fftw_plan plan = fftw_plan_dft_r2c_1d(size, nullptr, nullptr, FFTW_ESTIMATE);
#else
    // I have no idea what works best on Windows, I assume FFTW_ESTIMATE is safe
    fftw_plan plan = fftw_plan_dft_r2c_1d(size, nullptr, nullptr, FFTW_ESTIMATE);
#endif
    fft_plans_[size] = plan;
    return plan;
}
//-----------------------------------------------------------------------------
// End Make FFT plan safely
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Make Inverse FFT plan safely
//-----------------------------------------------------------------------------
fftw_plan FFTWPlanPool::make_ifft_plan(const std::size_t size)
{
    std::lock_guard<std::mutex> lock_pool(mutex_);
    // c2r is always FFTW_BACKWARD (complex-to-real) (inverse FFT)
#if defined(__APPLE__)
    // FFTW_MEASURE takes more time than FFTW_ESTIMATE, but is more optimal on execution
    // Because the plan may be used multiple times, might as well use measure
    fftw_plan plan = fftw_plan_dft_c2r_1d(size, nullptr, nullptr, FFTW_MEASURE);
#elif defined(__linux__)
    // FFTW_MEASURE crashes on Linux (fine on MacOS)
    fftw_plan plan = fftw_plan_dft_c2r_1d(size, nullptr, nullptr, FFTW_ESTIMATE);
#else
    // I have no idea what works best on Windows, I assume FFTW_ESTIMATE is safe
    fftw_plan plan = fftw_plan_dft_c2r_1d(size, nullptr, nullptr, FFTW_ESTIMATE);
#endif
    ifft_plans_[size] = plan;
    return plan;
}
//-----------------------------------------------------------------------------
// End Make Inverse FFT plan safely
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Make both FFT and inverse-FFT plans safely
//-----------------------------------------------------------------------------
void FFTWPlanPool::make_fft_ifft_plan_pair(const std::size_t size)
{
    acquire_fft_plan(size);
    acquire_ifft_plan(size);
}
//-----------------------------------------------------------------------------
// End Make both FFT and inverse-FFT plans safely
//-----------------------------------------------------------------------------
}