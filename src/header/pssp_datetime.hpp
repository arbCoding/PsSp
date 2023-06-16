#ifndef PSSP_DATETIME_HPP_20230610
#define PSSP_DATETIME_HPP_20230610

#include <sac_stream.hpp>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <iomanip>
#include <sstream>
#include <string>

namespace pssp
{
// Left pad integers to a given width (with zeros)
std::string left_pad_integers(int n, int width);
// Days per month, should move to a time header
int days_per_month(int year, int month);
// Go from year, month, day to day of year (time header)
int ymd_2_doy(int year, int month, int day);
// Day of year to year, month, day
std::string doy_2_ymd(int year, int doy);
// Datetime string from sac header
std::string sac_reference_time(SAC::SacStream& sac);
// Datetime string to sac header
void timestamp_to_reference_headers(const char* raw_timestamp, SAC::SacStream& sac);
}

#endif