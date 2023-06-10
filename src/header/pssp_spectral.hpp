#ifndef PSSP_SPECTRAL_HPP_20230610
#define PSSP_SPECTRAL_HPP_20230610

#include "pssp_fftw_planpool.hpp"
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <vector>
#include <complex>
// Needed by FFTW library
#include <cmath>

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
}

#endif
