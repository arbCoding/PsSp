#include "pssp_datetime.hpp"

namespace pssp
{
//------------------------------------------------------------------------
// Left-pad integers
//------------------------------------------------------------------------
std::string left_pad_integers(int n, int width)
{
    std::ostringstream oss{};
    oss << std::setw(width) << std::setfill('0') << n;
    return oss.str();
}
//------------------------------------------------------------------------
// End Left-pad integers
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Days per month
//------------------------------------------------------------------------
int days_per_month(int year, int month)
{
    // Standard days in month for non-leap years
    constexpr int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int days{days_in_month[month]};
    if (month == 1)
    {
        // February, check if it is a leap year
        if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
        {
            // Is a leap year
            ++days;
        }
    }
    return days;   
}
//------------------------------------------------------------------------
// End Days per month
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// YMD to Day of Year
//------------------------------------------------------------------------
int ymd_2_doy(int year, int month, int day)
{
    int doy{0};
    for (int i{0}; i < month; ++i)
    {
        doy += days_per_month(year, i);
    }
    doy += day;
    return doy;
}
//------------------------------------------------------------------------
// End YMD to Day of Year
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Day of Year to Month and Day of Month
//------------------------------------------------------------------------
std::string doy_2_ymd(int year, int doy)
{
    int month{0};
    int days{0};
    // Infinite loop if we don't break out of it
    while (true)
    {
        days = days_per_month(year, month);
        // Subtract whole month of days from the doy
        if (doy >= days)
        {
            doy -= days;
            ++month;
        }
        else 
        {
            // We're done removing months
            break;
        }
    }
    std::ostringstream oss{};
    oss << left_pad_integers(year, 4);
    oss << '-';
    oss << left_pad_integers(month + 1, 2);
    oss << '-';
    oss << left_pad_integers(doy, 2);
    // YYYY-MM-DD
    return oss.str();
}
//------------------------------------------------------------------------
// End Day of Year to Month and Day of Month
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Make datetime
//------------------------------------------------------------------------
std::string sac_reference_time(SAC::SacStream& sac)
{
    std::ostringstream oss{};
    oss << doy_2_ymd(sac.nzyear, sac.nzjday);
    oss << ' ';
    oss << left_pad_integers(sac.nzhour, 2);
    oss << ':';
    oss << left_pad_integers(sac.nzmin, 2);
    oss << ':';
    oss << left_pad_integers(sac.nzsec, 2);
    oss << '.';
    oss << left_pad_integers(sac.nzmsec, 3);
    return oss.str();
}
//------------------------------------------------------------------------
// End Make datetime
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Given a timestamp string, populate the SAC reference time headers
//------------------------------------------------------------------------
void timestamp_to_reference_headers(const char* raw_timestamp, SAC::SacStream& sac)
{
    std::string timestamp{raw_timestamp};
    // Extracting components from timestamp string
    std::istringstream iss(timestamp);
    std::string s_year{}, s_month{}, s_day{}, s_hour{}, s_minute{}, s_second{}, s_milliseconds{};
    std::getline(iss, s_year, '-');
    std::getline(iss, s_month, '-');
    std::getline(iss, s_day, ' ');
    std::getline(iss, s_hour, ':');
    std::getline(iss, s_minute, ':');
    std::getline(iss, s_second, '.');
    std::getline(iss, s_milliseconds);
    if (!s_year.empty()) { sac.nzyear = std::stoi(s_year); }
    int month{std::stoi(s_month)};
    int day{std::stoi(s_day)};
    sac.nzjday = ymd_2_doy(sac.nzyear, month, day);
    sac.nzmsec = std::stoi(s_milliseconds);
    sac.nzhour = std::stoi(s_hour);
    sac.nzmin = std::stoi(s_minute);
    sac.nzsec = std::stoi(s_second);
}
//------------------------------------------------------------------------
// End Given a timestamp string, populate the SAC reference time headers
//------------------------------------------------------------------------
}