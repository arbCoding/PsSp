#ifndef PSSP_DATETIME_HPP_20230610
#define PSSP_DATETIME_HPP_20230610

//-----------------------------------------------------------------------------
// Include statments
//-----------------------------------------------------------------------------
#include <sac_stream.hpp>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <iomanip>
#include <sstream>
#include <string>
//-----------------------------------------------------------------------------
// End Include statments
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
// Okay, so here is where things stand at the moment. I wrote this originally as
// a short little bit for taking the SAC format's various different separate
// date and time headers and combined them into a string in a fairly standard
// datetime format of YYYY-DD-MM HH:mm:SS.sss
// (I guess technically it'd be more standard to do YYYY-DD-MMTHH:mm:SS.sss) but I
// prefer to separate with a space instead of a 'T'
// And going between that and the SAC headers because I wanted to display an actually
// useful day (Julian Date [What I prefer to call Day of Year or DoY] is not very useful,
// though I understand that when the standard was first written they really needed to
// minimize the amount of space the header data took, so having 1 integer for combined month and day of month was
// more useful than to separate them, IT IS 2023 though, that concern is nonsense these days! The header
// is essentially neglegible size compared the data vector unless the timeseries is only 100 or so
// data points long)
//
// Now as it stands, I kind of need (or rather want, would prefer, whatever) more
// powerful tools for working withs datetimes in general.
//
// Things like comparing datetime objects (are they equal, less than, or greater than),
// converting between formats (what if I want to align signals by time of day [to see if
// a suspect signal is perhaps due to regular daytime traffic, for example]), or figure out
// the distance (temporal) between two times, how far apart are they?
// This is going to have profound use for my plotting functionality, among many other future prospects,
// so I really ought to redo this in a general sense. Perhaps a new github for a dedicated timeseries
// library? I know boost has libraries for this, but in my experience boost is regularly broken
// because even if one chunk of boost hasn't updated, it could depend on another chunk that has updated and be borked.
// Or other shenanigans. While I can generally trust standard lib stuff, boost [in my experience] has been a headache
// Hence why I even wrote this little bit by itself...
//
// And being able to handle dates, times, and datetimes will be useful for more than just timeseries analysis
// so it is kind of general enough to be a separate project unto itself.
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Begin pssp namespace
//-----------------------------------------------------------------------------
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
std::string sac_reference_time(const SAC::SacStream& sac);
// Datetime string to sac header
void timestamp_to_reference_headers(const char* raw_timestamp, SAC::SacStream& sac);
}
//-----------------------------------------------------------------------------
// End pssp namespace
//-----------------------------------------------------------------------------

#endif