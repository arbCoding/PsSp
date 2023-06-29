#ifndef PSSP_SPECTRAL_HPP_20230610
#define PSSP_SPECTRAL_HPP_20230610

#include "pssp_fftw_planpool.hpp"
// FFTW3 library
#include <fftw3.h>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
// Needed by FFTW library
#include <cmath>
#include <complex>
#include <vector>

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
// This is raw spectral functionality on vectors
// Converting this to SacStream appropriate stuff happens in pssp_misc.hpp/.cpp
//
// The option to renormalize is provided for the sake of plotting mostly
//  e.g. plotting the spectrum, you'd like for the units to make sense
//
// However, it isn't needed for filtering/application of transfer function
//  because it automatically normalizes on the inverse-FFT (assuming you want
//  to modify the time-series, not work purely in the spectral domain)
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

namespace pssp
{
// Given a plan pool, and a vector of time-series data (evenly sampled) return the forward FFT as a new vector
// Renormalize = false = default FFTW normalization (FFT -> 1; IFFT -> 1/N)
// Renormalize = true = "standard" normalization (FFT -> 1/sqrt(N); IFFT -> 1/sqrt(N))
std::vector<std::complex<double>>  fft_time_series(FFTWPlanPool& plan_pool, const std::vector<double>& time_series, bool renormalize = false);
std::vector<double> ifft_spectrum(FFTWPlanPool& plan_pool, const std::vector<std::complex<double>>& spectrum, bool renormalize = false);
// Normalized Butterworth polynomials: https://en.wikipedia.org/wiki/Butterworth_filter#Normalized_Butterworth_polynomials
// These are the a_k
std::vector<double> butterworth_coeffs(int n);
// These are the b_n(s)
std::complex<double> butterworth_laplace(const std::vector<double>& coeffs, const std::complex<double> s);
// Given appropriately sized vectors (gain and phase), as well as the bounds to look at, fill the gain and phase vectors
void butterworth_low(const int n, std::vector<double>& gain, std::vector<double>& phase, const double min_freq, const double max_freq, const int n_freq);
// Now for the true butterworth lowpass filter
void butterworth_low(const int n, const double min_freq, const double d_freq, const double corner_freq, std::vector<std::complex<double>>& spectrum);
// True butterworth highpass filter
void butterworth_high(const int n, const double min_freq, const double d_freq, const double corner_freq, std::vector<std::complex<double>>& spectrum);
}

#endif
