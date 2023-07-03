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
        spectrum[i] /= butterworth_laplace(coeffs, current_z / corner_freq);
        // Since frequencies above Nyquist are mirrored
        spectrum[n_freq - i] = spectrum[i];
    }
}

// Given appropriately sized vectors (gain and phase), as well as the bounds to look at, fill the gain and phase vectors
void butterworth_high(const int n, std::vector<double>& gain, std::vector<double>& phase, std::vector<double>& freqs, const int n_freqs)
{
    if (gain.size() < static_cast<std::size_t>(n_freqs)) { gain.resize(n_freqs); phase.resize(n_freqs); }
    freqs = logspace(-2, 2, n_freqs);
    const std::vector<double> coeffs{butterworth_coeffs(n)};
    for (int i{0}; i < n_freqs; ++i)
    {
        // Avoid division by 0.0
        if (freqs[i] != 0.0)
        {
            const std::complex<double> current_z{0.0, freqs[i]};
            const std::complex<double> bn{butterworth_laplace(coeffs, 1.0 / current_z)};
            const double bn_mag{std::sqrt((bn.real() * bn.real()) + (bn.imag() * bn.imag()))};
            gain[i] = 1.0 / bn_mag;
            phase[i] = -std::abs(std::atan(-bn.imag() / bn.real()));
        }
        else { gain[0] = 0.0; phase[i] = 0.0; }
    }
}

void butterworth_high(const int n, const double min_freq, const double d_freq, const double corner_freq, std::vector<std::complex<double>>& spectrum)
{
    const std::vector<double> coeffs{butterworth_coeffs(n)};
    const std::size_t n_freq{spectrum.size()};
    // We cannot have division by 0 so we do it separately
    spectrum[0] = 0.0;
    spectrum[n_freq - 1] = spectrum[0];
    for (std::size_t i{1}; i <= n_freq / 2; ++i)
    {
        const std::complex<double> current_z{0.0, (i * d_freq) + min_freq};
        spectrum[i] /= butterworth_laplace(coeffs, corner_freq / current_z);
        // Since frequencies above Nyquist are mirrored
        spectrum[n_freq - i] = spectrum[i];
    }
}

// This and bandreject should scale based upon the frequencies chosen
// Given appropriately sized vectors (gain and phase), as well as the bounds to look at, fill the gain and phase vectors
void butterworth_bandpass(const int n, std::vector<double>& gain, std::vector<double>& phase, std::vector<double>& freqs, const int n_freqs)
{
    // This function is special, it should have a freq_naught of 1.0
    // And the bandwidth means I choose a freq freq_low or freq_high and then that determines the other
    // freq_naught = std::sqrt(freq_low * freq_high);
    // Then
    // freq_low = (freq_naught * freq_naught) / freq_high;
    // or
    // freq_high = (freq_naught * freq_naught) / freq_low;
    // So say freq_low = f(freq_naught); or freq_high = g(freq_naught);
    // If freq_low = freq_naught/5;
    // then freq_high = 5 * freq_naught;
    // Gain is max at freq_naught and is not necessarily one so I need to normalize by it!
    constexpr double freq_naught{1.0};
    constexpr double freq_low{freq_naught / 10.0};
    constexpr double freq_high{(freq_naught * freq_naught) / freq_low};
    constexpr double delta_freq{freq_high - freq_low};
    if (gain.size() < static_cast<std::size_t>(n_freqs)) { gain.resize(n_freqs); phase.resize(n_freqs); }
    freqs = logspace(-2, 2, n_freqs);
    const std::vector<double> coeffs{butterworth_coeffs(n)};
    double max_gain{0.0};
    for (int i{0}; i < n_freqs; ++i)
    {
        // Avoid division by 0.0
        if (freqs[i] != 0.0)
        {
            const std::complex<double> current_z{0.0, freqs[i]};
            const std::complex<double> bn{butterworth_laplace(coeffs, delta_freq * ((current_z / freq_naught) + (freq_naught / current_z)))};
            const double bn_mag{std::sqrt((bn.real() * bn.real()) + (bn.imag() * bn.imag()))};
            const double current_gain{1.0 / bn_mag};
            max_gain = ((max_gain > current_gain) ? max_gain : current_gain);
            gain[i] = 1.0 / bn_mag;
            phase[i] = -std::abs(std::atan(-bn.imag() / bn.real()));
        }
        else { gain[0] = 0.0; phase[i] = 0.0; }
    }
    // Normalize
    for (int i{0}; i < n_freqs; ++i) { gain[i] /= max_gain; }
}

void butterworth_bandpass(const int n, const double min_freq, const double d_freq, const double corner_freq_low, const double corner_freq_high, std::vector<std::complex<double>>& spectrum)
{
    const std::vector<double> coeffs{butterworth_coeffs(n)};
    const std::size_t n_freq{spectrum.size()};
    // These are never zero!
    // Bandwidth
    const double delta_freq{corner_freq_high - corner_freq_low};
    // Central frequency (geometric mean)
    const double freq_naught{std::sqrt(corner_freq_low * corner_freq_high)};
    // Avoid divide by zero
    spectrum[0] = 0.0;
    spectrum[n_freq - 1] = spectrum[0];
    // This filter doesn't automatically normalize, it depends on the bandwidth so I keep track of the gain to ensure we normalize
    double max_gain{0.0};
    for (std::size_t i{1}; i <= n_freq / 2; ++i)
    {
        const std::complex<double> current_z{0.0, (i * d_freq) + min_freq};
        const std::complex<double> bn{butterworth_laplace(coeffs, delta_freq * ((current_z / freq_naught) + (freq_naught / current_z)))};
        const double bn_mag{std::sqrt((bn.real() * bn.real()) + (bn.imag() * bn.imag()))};
        const double current_gain{1.0 / bn_mag};
        max_gain = ((max_gain > current_gain) ? max_gain : current_gain);
        spectrum[i] /= bn;
    }
    // Normalize and take advantage of symmetry of the FFTW output spectrum
    for (std::size_t i{0}; i <= n_freq / 2; ++i) { spectrum[i] /= max_gain; spectrum[n_freq - i] = spectrum[i]; }
}

// Bandreject doesn't seem to actaully work?
// I've tested it on data, the gain/phase display looks okay, but the signal doesn't seem to get edited much (if at all)
//=============================================================================
// I need to get amplitude/phase display for seismograms to make it easier to tell what is going on
//=============================================================================
// I think the issue may be with frequency scaling the bandpass/bandreject
// I need to calculate the freq_naught, then scale the lowpass to freq_naught
// Then go from lowpass at freq_naught to bandpass at freq_naught (or bandreject)
// I think those are the issues.f
void butterworth_bandreject(const int n, std::vector<double>& gain, std::vector<double>& phase, std::vector<double>& freqs, const int n_freqs)
{
    constexpr double freq_naught{1.0};
    constexpr double freq_low{freq_naught / 10.0};
    constexpr double freq_high{(freq_naught * freq_naught) / freq_low};
    constexpr double delta_freq{freq_high - freq_low};
    if (gain.size() < static_cast<std::size_t>(n_freqs)) { gain.resize(n_freqs); phase.resize(n_freqs); }
    freqs = logspace(-2, 2, n_freqs);
    const std::vector<double> coeffs{butterworth_coeffs(n)};
    double max_gain{0.0};
    for (int i{0}; i < n_freqs; ++i)
    {
        // Avoid division by 0.0
        if (freqs[i] != 0.0)
        {
            const std::complex<double> current_z{0.0, freqs[i]};
            const std::complex<double> bn{butterworth_laplace(coeffs, 1.0 / (delta_freq * ((current_z / freq_naught) + (freq_naught / current_z))))};
            const double bn_mag{std::sqrt((bn.real() * bn.real()) + (bn.imag() * bn.imag()))};
            const double current_gain{1.0 / bn_mag};
            max_gain = ((max_gain > current_gain) ? max_gain : current_gain);
            gain[i] = 1.0 / bn_mag;
            phase[i] = -std::abs(std::atan(-bn.imag() / bn.real()));
        }
        else { gain[0] = 0.0; phase[i] = 0.0; }
    }
    // Normalize
    //for (int i{0}; i < n_freqs; ++i) { gain[i] /= max_gain; }
}

// True butterworth bandreject filter
void butterworth_bandreject(const int n, const double min_freq, const double d_freq, const double corner_freq_low, const double corner_freq_high, std::vector<std::complex<double>>& spectrum)
{
    const std::vector<double> coeffs{butterworth_coeffs(n)};
    const std::size_t n_freq{spectrum.size()};
    const double delta_freq{corner_freq_high - corner_freq_low};
    const double freq_naught{std::sqrt(corner_freq_low + corner_freq_high)};
    spectrum[0] = 0.0;
    spectrum[n_freq - 1] = spectrum[0];
    double max_gain{0.0};
    for (std::size_t i{1}; i <= n_freq / 2; ++i)
    {
        const std::complex<double> current_z{0.0, (i * d_freq) + min_freq};
        const std::complex<double> bn{butterworth_laplace(coeffs, 1.0 / (delta_freq * ((current_z / freq_naught) + (freq_naught / current_z))))};
        const double bn_mag{std::sqrt((bn.real() * bn.real()) + (bn.imag() * bn.imag()))};
        const double current_gain{1.0 / bn_mag};
        max_gain = ((max_gain > current_gain) ? max_gain : current_gain);
        spectrum[i] /= bn;
    }
    // Normalize and take advantage of symmetry of the FFTW output spectrum
    //for (std::size_t i{0}; i <= n_freq / 2; ++i) { spectrum[i] /= max_gain; spectrum[n_freq - i] = spectrum[i]; }
}

// I need to add amplitude and phase display for seismograms (may be more useful
// to determine the effect a filter had on the signal than just showing the real and imaginary components).
//
// Then TESTS I need so many tests for the spectral stuff. Which is going to require template signals (dirac delta, boxcar, triangle, gaussian, sombrero hat
// maybe even a chirp signal so that the filter's can be tested (and possibly could display the effects for the user)).
//
// After that I need to to do instrument response removal (which means I also need to handle reading in pole-zero files). Which likely requires test pz files...

// This works perfectly now
std::vector<double> logspace(const double start_power, const double end_power, const int n_samples, const double base)
{
    std::vector<double> result(n_samples);
    const double exp_scale{(end_power - start_power) / (n_samples - 1)};
    for (int i{0}; i < n_samples; ++i) { result[i] = std::pow(base, i * exp_scale + start_power); }
    return result;
}
}
