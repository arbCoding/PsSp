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
    if (renormalize) { normalization_constant = std::sqrt(static_cast<double>(n)); }
    // Empty vector of correct size
    std::vector<std::complex<double>> spectrum(n);
    // Copy the contents, cannot use move semantics..., renormalize if appropriate (or divide by 1.0)
    for (std::size_t i{0}; i < n; ++i)
    { spectrum[i] = std::complex<double>(fftw_spectrum[i][0] / normalization_constant, fftw_spectrum[i][1] / normalization_constant); }
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
    // If we didn't originally normalize in the fft, we now need to do it by a factor of n
    double normalization_constant{static_cast<double>(n)};
    if (renormalize) { normalization_constant = std::sqrt(static_cast<double>(n)); }
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

std::vector<double> butterworth_coeffs(int n)
{
    // If order = n, then n + 1 elements (0 up to and including n)
    std::vector<double> coeffs(n + 1, 0.0);
    // Normalization condition
    coeffs[0] = 1.0;
    // Symmetry condition
    coeffs[n] = 1.0;
    // If we're done, return
    if (n == 1) { return coeffs; }
    const double gamma{std::numbers::pi / static_cast<double>(2 * n)};
    for (int k{1}; k <= n / 2; ++k)
    {
        coeffs[k] = coeffs[k - 1] * (std::cos(static_cast<double>((k - 1)) * gamma) / std::sin(static_cast<double>(k) * gamma));
        // Symmetry
        coeffs[n - k] = coeffs[k];
    }
    return coeffs;
}

std::complex<double> butterworth_laplace(const std::vector<double>& coeffs, const std::complex<double> s)
{
    const std::size_t n{coeffs.size() - 1};
    std::complex<double> result{coeffs[0]};
    for (std::size_t i{1}; i <= n; ++i) { result += coeffs[i] * std::pow(s, i); }
    return result;
}


// Given appropriately sized vectors (gain and phase), as well as the bounds to look at, fill the gain and phase vectors
void butterworth_low(const int n, std::vector<double>& gain, std::vector<double>& phase, std::vector<double>& freqs, const int n_freqs)
{
    if (gain.size() < static_cast<std::size_t>(n_freqs)) { gain.resize(n_freqs); phase.resize(n_freqs); }
    freqs = logspace(-2, 2, n_freqs);
    const std::vector<double> coeffs{butterworth_coeffs(n)};
    for (int i{0}; i < n_freqs; ++i)
    {
        const std::complex<double> current_z{0.0, freqs[i]};
        const std::complex<double> bn{butterworth_laplace(coeffs, current_z)};
        const double bn_mag{std::sqrt((bn.real() * bn.real()) + (bn.imag() * bn.imag()))};
        gain[i] = 1.0 / bn_mag;
        phase[i] = -std::abs(std::atan(-bn.imag() / bn.real()));
    }
}

void butterworth_low(const int n, const double min_freq, const double d_freq, const double corner_freq, std::vector<std::complex<double>>& spectrum)
{
    const std::vector<double> coeffs{butterworth_coeffs(n)};
    std::size_t n_freq{spectrum.size()};
    for (std::size_t i{0}; i <= n_freq / 2; ++i)
    {
        const std::complex<double> current_z{0.0, (i * d_freq) + min_freq};
        const std::complex<double> filter_response{butterworth_laplace(coeffs, current_z / corner_freq)};
        spectrum[i] /= filter_response;
        // Since frequencies above Nyquist are mirrored
        spectrum[n_freq - i] /= std::conj(filter_response);
    }
}

void butterworth_high(const int n, const double min_freq, const double d_freq, const double corner_freq, std::vector<std::complex<double>>& spectrum)
{
    const std::vector<double> coeffs{butterworth_coeffs(n)};
    // We cannot have division by 0 so we do it separately
    spectrum[0] = 0.0;
    for (std::size_t i{1}; i < spectrum.size(); ++i)
    {
        const std::complex<double> current_z{0.0, (i * d_freq) + min_freq};
        spectrum[i] /= butterworth_laplace(coeffs, corner_freq / current_z);
    }
}

// This works perfectly now
std::vector<double> logspace(const double start_power, const double end_power, const int n_samples, const double base)
{
    std::vector<double> result(n_samples);
    const double exp_scale{(end_power - start_power) / (n_samples - 1)};
    for (int i{0}; i < n_samples; ++i) { result[i] = std::pow(base, i * exp_scale + start_power); }
    return result;
}
}
