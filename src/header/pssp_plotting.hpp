#ifndef PSSP_PLOTTING_HPP_20230721
#define PSSP_PLOTTING_HPP_20230721

//-----------------------------------------------------------------------------
// Include statments
//-----------------------------------------------------------------------------
// sac_1c struct
#include "pssp_data_pool.hpp"
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <vector>
//-----------------------------------------------------------------------------
// End Include statments
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
// This is where I will implement functionality that is focused on plotting
// That means this will handle:
//  Given a SacStream calculate the relative time vector
//  Given a SacStream calculate the absolute time vector
//  Given a minimum and a maximum time-value and a SacStream object, trim
//      the data vector to that time-window
//  Given a "minimum" and a "maximum" time-value and a SacStream object, trim
//      the data vector to the time-window, with a buffer on both ends
//  Given a value of pixel-width, determine our current point/pixel ratio.
//  Given a target point/pixel ratio and our current point/pixel ratio,
//      determine if we want to down-sample or up-sample
//  Given a target point/pixel ratio and our current point/pixel ratio,
//      determine the downsample factor (every other, every third, etc...)
//  I should store a couple different copies of the vectors: original
//      current_focus, higher-sampled, lower-sampled? I need to think about how to
//      handle that sort of setup...
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Begin pssp namespace
//-----------------------------------------------------------------------------
namespace pssp
{
// Given a sac_1c I want the array of time values for start to end (relative to the reference time in seconds)
std::vector<double> relative_times(sac_1c& timeseries);
// For absolute times, I'll need to expand my datetime library
};
//-----------------------------------------------------------------------------
// End pssp namespace
//-----------------------------------------------------------------------------
#endif