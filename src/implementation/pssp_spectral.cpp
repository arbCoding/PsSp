#include "pssp_spectral.hpp"

namespace pssp
{
//-----------------------------------------------------------------------------
// FFT of time-series using FFTWPlanPool (multi-thread)
//-----------------------------------------------------------------------------
std::vector<std::complex<double>> fft_time_series(FFTWPlanPool& plan_pool, const std::vector<double>& time_series, bool renormalize)
{
    const std::size_t n{time_series.size()};
    auto* signal = (double*) fftw_malloc(sizeof(double) * n);
    // Copy into the new double
    for (std::size_t i{0}; i < n; ++i) { signal[i] = time_series[i]; }
    // We'll do full size as it isn't a huge time savings to do only half
    // Can changed later if it becomes an issue
    auto* fftw_spectrum = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * n);
    // Get the plan
    fftw_plan plan = plan_pool.acquire_fft_plan(n);
    // Execute the plan
    fftw_execute_dft_r2c(plan, signal, fftw_spectrum);
    // Normalization constant, division by 1 is faster than constantly checking an if statement
    double normalization_constant{1};
    if (renormalize)
    {
        normalization_constant = std::sqrt(static_cast<double>(n));
    }
    // Empty vector of correct size
    std::vector<std::complex<double>> spectrum(n);
    // Copy the contents, cannot use move semantics..., renormalize if appropriate (or divide by 1.0)
    for (std::size_t i{0}; i < n; ++i)
    {
        spectrum[i] = std::complex<double>(fftw_spectrum[i][0] / normalization_constant, fftw_spectrum[i][1] / normalization_constant);
    }
    // Clean up
    fftw_free(signal);
    fftw_free(fftw_spectrum);
    return spectrum;
}
//-----------------------------------------------------------------------------
// End FFT of time-series using FFTWPlanPool (multi-thread)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Inverse-FFT of spectrum using FFTWPlanPool (multi-thread)
//-----------------------------------------------------------------------------
std::vector<double> ifft_spectrum(FFTWPlanPool& plan_pool, const std::vector<std::complex<double>>& spectrum, bool renormalize)
{
    const std::size_t n{spectrum.size()};
    auto* fftw_spectrum = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * n);
    // Copy into the new fftw_complex
    for (std::size_t i{0}; i < n; ++i) { fftw_spectrum[i][0] = spectrum[i].real(); fftw_spectrum[i][1] = spectrum[i].imag(); }
    auto* signal = (double*) fftw_malloc(sizeof(double) * n);
    // Get the plan
    fftw_plan plan = plan_pool.acquire_ifft_plan(n);
    // Execute the plan
    fftw_execute_dft_c2r(plan, fftw_spectrum, signal);
    // Normalization constant, divison by 1 is faster than constantly checking and if statement
    double normalization_constant{1};
    if (renormalize)
    {
        normalization_constant = std::sqrt(static_cast<double>(n));
    }
    // Empty vector of correct size
    std::vector<double> time_series(n);

    // Copy the contents, cannot use move semantics..., renormalize if appropriate (or divide by 1.0)
    for (std::size_t i{0}; i < n; ++i) { time_series[i] = signal[i] / normalization_constant; }
    // Clean up
    fftw_free(signal);
    fftw_free(fftw_spectrum);
    return time_series;
}
//-----------------------------------------------------------------------------
// End Inverse-FFT of spectrum using FFTWPlanPool (multi-thread)
//-----------------------------------------------------------------------------
}
