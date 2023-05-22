#ifndef PSSP_PROJECTS_HPP
#define PSSP_PROJECTS_HPP

#include "sac_io.hpp"
#include "sac_stream.hpp"
#include <ios>
#include <sqlite3.h>
// String comparisons in C++ suck, boost adds needed functionality!
#include <boost/algorithm/string.hpp>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <filesystem>
#include <iostream>
#include <sstream>
// Needed for formatting datetime string
// std::tm, std::get_time, std::put_time
#include <iomanip>
#include <chrono>
#include <ctime>

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
// A project will be maintained as an SQLite3 database
// For now we're going to use SQLite3 in serialized mode, meaning that
// a single connection to the database is needed.
//
// This single connection can be passed to multiple threads at once
// and sqlite3 will handle the synchronization and what not
//
// It allows multiple threads to read at once, but only one thread may write
// at a time. If multiple threads are trying to write, they wait their turns.
// It threads are trying to read while a thread is writing, they must wait.
//
// We must be careful to avoid deadlock situations (a thread needs to write,
// but never gets to because other threads are constantly reading)
//
// We may also need to handle the connection dropping (hopefully a non-issue, we'll
// see).
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ToDo
//-----------------------------------------------------------------------------
// So, we can load base data and place it inside a database on disk
// We cannot however load an existing database from a file on disk
//
// Files that are being worked on exist in memory (at the start of a project that
// is all of the base data files).
//
// Files that are unloaded, stay in the database with a removed-on datetime
// so that people can see when they removed a file from their database
//
// We'll need a mechanism to fully purge a file from the database (instead of tracking
// up to when it was removed)
//
// We still obviously need a file manipulation record (track filters, trimming, etc).
// That needs an action column ("function parameters...") action datetime stamp, checkpoint tag
//
// We need a checkpoint list table
// Checkpoint id (automatic), checkpoint name, bool-auto, bool-cull, checkpoint datetime, n_files, spawn checkpoint id (what checkpoint built this), bool-deleted
// By keeping track fo which checkpoint spawned one, we can track the origin of a checkpoint (allow us to follow the chain of file manipulations with a non-linear path)
// By keeping the checkpoint id even after the checkpoint is deleted we do not loose the heritage of a checkpoint (just drop the checkpoint's unique table)
//
// We need a checkpoint table
// This is basically a base_data table, except the header values may be different and obviously the data1 (and possibly data2, though we assume this is empty for now
// because evenly sampled time-series are the default) are likely to be different
//-----------------------------------------------------------------------------
// End ToDo
//-----------------------------------------------------------------------------

namespace pssp
{
class Project
{
    private:
        // Name of the project, only gets set once
        std::string_view name_{};
        // Path to the database file
        std::filesystem::path path_{};
        //----------------------------------------------------------------
        // Left-pad integers
        //----------------------------------------------------------------
        std::string left_pad_integers(int n, int width)
        {
            std::ostringstream oss{};
            oss << std::setw(width) << std::setfill('0') << n;
            return oss.str();
        }
        //----------------------------------------------------------------
        // End Left-pad integers
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Make datetime
        //----------------------------------------------------------------
        std::string sac_reference_time(SAC::SacStream& sac)
        {
            std::ostringstream oss{};
            oss << left_pad_integers(sac.nzyear, 4);
            oss << '-';
            oss << left_pad_integers(sac.nzjday, 3);
            oss << ' ';
            oss << left_pad_integers(sac.nzhour, 2);
            oss << ':';
            oss << left_pad_integers(sac.nzmin, 2);
            oss << ':';
            oss << left_pad_integers(sac.nzsec, 2);
            oss << '.';
            oss << left_pad_integers(sac.nzmsec, 3);
            std::string tmp{oss.str()};
            std::tm datetime{};
            std::istringstream iss(tmp);
            iss >> std::get_time(&datetime, "%Y-%j %H:%M:%S");
            // Extract fractional seconds for the original datetime string
            int frac_secs{};
            if (iss.peek() == '.') { iss.ignore(); iss >> frac_secs; }
            // Chrono duration for the fracitonal seconds
            std::chrono::duration<int, std::ratio<1, 1000>> frac_dur{frac_secs};
            std::ostringstream formatted_datetime{};
            formatted_datetime << std::put_time(&datetime, "%Y-%m-%d %H:%M:%S");
            // Append fractional seconds
            formatted_datetime << '.' << std::setfill('0') << std::setw(3) << frac_dur.count();
            return formatted_datetime.str();
        }
        //----------------------------------------------------------------
        // End Make datetime
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Create base_data table
        //----------------------------------------------------------------
        // Create a base data table
        void create_base_data_table(sqlite3* db_connection)
        {
            // Times in seconds are relative to reference time defined by the nzSTUFF headers
            // Going to keep all time-series headers (no spectral, no xy(z) nonsense)
            std::ostringstream oss{};
            oss << "CREATE TABLE base_data (";
            oss << "base_id INTEGER PRIMARY KEY, "; // 1 (automatically generated id)
            oss << "source TEXT, "; // 2 (if from filesystem, string path, prior to filename)
            oss << "file TEXT, "; // 3 (whatever.SAC filename)
            oss << "added DATETIME DEFAULT CURRENT_TIMESTAMP, "; // 4 (automatic UTC timestamp)
            oss << "delta REAL, "; // 5 (sac.delta, sample rate)
            oss << "b REAL, "; // 6 (sac.b, begin time in seconds)
            oss << "e REAL, "; // 7 (sac.e, end time in seconds)
            oss << "o REAL, "; // 8 (sac.o, origin time in seconds)
            oss << "a REAL, "; // 9 (sac.a, first arrival pick time in seconds)
            oss << "t0 REAL, "; // 10 (sac.t0, user time in seconds)
            oss << "t1 REAL, "; // 11
            oss << "t2 REAL, "; // 12
            oss << "t3 REAL, "; // 13
            oss << "t4 REAL, "; // 14
            oss << "t5 REAL, "; // 15
            oss << "t6 REAL, "; // 16
            oss << "t7 REAL, "; // 17
            oss << "t8 REAL, "; // 18
            oss << "t9 REAL, "; // 19
            oss << "f REAL, "; // 20 (sac.f, fini time in seconds [for finite source])
            oss << "resp0 REAL, "; // 21 (sac.resp0, instrument response parameter)
            oss << "resp1 REAL, "; // 22
            oss << "resp2 REAL, "; // 23
            oss << "resp3 REAL, "; // 24
            oss << "resp4 REAL, "; // 25
            oss << "resp5 REAL, "; // 26
            oss << "resp6 REAL, "; // 27
            oss << "resp7 REAL, "; // 28
            oss << "resp8 REAL, "; // 29
            oss << "resp9 REAL, "; // 30
            oss << "stla REAL, "; // 31 (sac.stla, station latitude, degrees, north positive)
            oss << "stlo REAL, "; // 32 (sac.stlo, station longitude, degrees, east positive)
            oss << "stel REAL, "; // 33 (sac.stel, station elevation m a.s.l.)
            oss << "stdp REAL, "; // 34 (sac.stdp, station depth below surface, meters)
            oss << "evla REAL, "; // 35 (sac.evla, event latitude)
            oss << "evlo REAL, "; // 36 (sac.evlo, event longitude)
            oss << "evel REAL, "; // 37 (sac.evel, event elevation)
            oss << "evdp REAL, "; // 38 (sac.evdp, event depth below surface, kilometers [previous meters])
            oss << "mag REAL, "; // 39 (sac.mag, event magnitude)
            oss << "user0 REAL, "; // 40 (sac.user0, user-defined storage)
            oss << "user1 REAL, "; // 41
            oss << "user2 REAL, "; // 42
            oss << "user3 REAL, "; // 43
            oss << "user4 REAL, "; // 44
            oss << "user5 REAL, "; // 45
            oss << "user6 REAL, "; // 46
            oss << "user7 REAL, "; // 47
            oss << "user8 REAL, "; // 48
            oss << "user9 REAL, "; // 49
            oss << "dist REAL, "; // 50 (sac.dist, station-event distance, kilometers)
            oss << "az REAL, "; // 51 (sac.az, station-event azimuth, degrees)
            oss << "baz REAL, "; // 52 (sac.baz, event-station azimuth, degrees)
            oss << "gcarc REAL, "; // 53 (sac.gcarc, station_event great circle-arc distance, degrees)
            oss << "depmin REAL, "; // 54 (sac.depmin, minimum amplitude)
            oss << "depmen REAL, "; // 55 (sac.depmen, mean amplitude)
            oss << "depmax REAL, "; // 56 (sac.depmax, maximum amplitude)
            oss << "cmpaz REAL, "; // 57 (sac.cmpaz, component azimuth, degrees clockwise from North)
            oss << "cmpinc REAL, "; // 58 (sac.cmpinc, component incident angle, degrees from upward vertical (incident 0 = -90 dip), (dip 0 = incident 90)
            oss << "reference_time DATETIME, "; // 59 (take all sac.nzStuff headers and format into a string, then convert to standard format)
            oss << "norid INTEGER, "; // 60 (sac.norid, origin id)
            oss << "nevid INTEGER, "; // 61 (sac.nevid, event id)
            oss << "npts INTEGER, "; // 62 (sac.npts, number of points in time-series)
            oss << "nwfid INTEGER, "; // 63 (sac.nwfid, waveform id)
            oss << "iftype INTEGER, "; // 64 (sac.iftype, type of file (should always be 1 = ITIME = Time-series!))
            oss << "idep INTEGER, "; // 65 (sac.idep, amplitude type, see SAC::SacStream for details)
            oss << "iztype INTEGER, "; // 66 (sac.iztype, reference time equivalent, see SAC::SacStream for details)
            oss << "iinst INTEGER, "; // 67 (sac.iints, type of recording instrument)
            oss << "istreg INTEGER, "; // 68 (sac.istreg, station geographic region)
            oss << "ievreg INTEGER, "; // 69 (sac.ievreg, event geographic region)
            oss << "ievtyp INTEGER, "; // 70 (sac.ievtyp, type of event)
            oss << "iqual INTEGER, "; // 71 (sac.iqual, data quality)
            oss << "isynth INTEGER, "; // 72 (sac.isynth, synthetic data flag)
            oss << "imagtyp INTEGER, "; // 73 (sac.imagtyp, type of magnitude)
            oss << "imagsrc INTEGER, "; // 74 (sac.imagsrc, source of magnitude information)
            oss << "ibody INTEGER, "; // 75 (sac.ibody, body/spheroid definition)
            // True/False = 1/0
            oss << "leven INTEGER, "; // 76 (sac.leven, evenly-spaced flag)
            oss << "lpspol INTEGER, "; // 77 (sac.lpspol, positive polarity (left-hand reul NEZ [North-East-Up]))
            // Skip lovrok and lcalda because they're stupider headers than normal
            oss << "kstnm TEXT, "; // 78 (sac.kstnm, station name)
            oss << "kevnm TEXT, "; // 79 (sac.kevnm, event name)
            oss << "khole TEXT, "; // 80 (sac.khole, hole identifier or location ID)
            oss << "ko TEXT, "; // 81 (sac.ko, origin time text)
            oss << "ka TEXT, "; // 82 (sac.ka, first arrival time text)
            oss << "kt0 TEXT, "; // 83 (sac.kt0, user time text)
            oss << "kt1 TEXT, "; // 84
            oss << "kt2 TEXT, "; // 85
            oss << "kt3 TEXT, "; // 86
            oss << "kt4 TEXT, "; // 87
            oss << "kt5 TEXT, "; // 88
            oss << "kt6 TEXT, "; // 89
            oss << "kt7 TEXT, "; // 90
            oss << "kt8 TEXT, "; // 91
            oss << "kt9 TEXT, "; // 92
            oss << "kf TEXT, "; // 93 (sac.kf, fini text)
            oss << "kuser0 TEXT, "; // 94 (sac.kuser0, user text storage)
            oss << "kuser1 TEXT, "; // 95
            oss << "kuser2 TEXT, "; // 96
            oss << "kcmpnm TEXT, "; // 97 (sac.kcmpnm, compnent name)
            oss << "knetwk TEXT, "; // 98 (sac.knetwk, network name)
            // Skip kdatrd, stupid
            oss << "kinst TEXT, "; // 99 (sac.kinst, generic recording instrument name)
            oss << "data1 BLOB, "; // 100 (sac.data1, time-series)
            oss << "data2 BLOB);"; // 101 (sac.data1, if unevenly sampled, these are the sample times)
            std::string sq3_create_base_data{oss.str()};
            sq3_result = sqlite3_exec(db_connection, sq3_create_base_data.c_str(), nullptr, nullptr, &sq3_error_message);
        }
        //----------------------------------------------------------------
        // End create base_data table
        //----------------------------------------------------------------
    public:
        // Connection to the database (file)
        sqlite3* sq3_connection_file{};
        // Connection to the database (memory)
        sqlite3* sq3_connection_memory{};
        int sq3_result{};
        char* sq3_error_message{};
        //----------------------------------------------------------------
        // Parameterized Constructor
        //----------------------------------------------------------------
        Project(std::string name, std::filesystem::path base_path) : name_{name}, path_{base_path / (name + ".db")}
        {
            // Create a new connection
            sq3_result = sqlite3_open_v2(path_.c_str(), &sq3_connection_file, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
            // Set the journal mode to WAL
            sq3_result = sqlite3_exec(sq3_connection_file, "PRAGMA journal_mode=WAL", nullptr, nullptr, &sq3_error_message);
            // Create a new connection
            sq3_result = sqlite3_open_v2(":memory:", &sq3_connection_memory, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
            // Set the journal mode to WAL
            sq3_result = sqlite3_exec(sq3_connection_memory, "PRAGMA journal_mode=WAL", nullptr, nullptr, &sq3_error_message);
            // Database stored on system
            create_base_data_table(sq3_connection_file);
            // Database maintained in memory
            //create_base_data_table(sq3_connection_memory);
        }
        //----------------------------------------------------------------
        // End Parameterized Constructor
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Destructor
        //----------------------------------------------------------------
        ~Project() { sqlite3_close_v2(sq3_connection_file); sqlite3_close_v2(sq3_connection_memory); }
        //----------------------------------------------------------------
        // End Destructor
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Add base data
        //----------------------------------------------------------------
        // Because we store the data vector as a double instead ofa float, the data-size is roughly double
        // but we no longer lose precision upon saving
        void add_base_data_SacStream(SAC::SacStream& sac, std::filesystem::path file_path)
        {
            // Build the insert statement
            std::ostringstream oss{};
            oss << "INSERT INTO base_data (";
            oss << "source, "; // 1
            oss << "file, "; // 2
            oss << "delta, "; // 3
            oss << "b, "; // 4
            oss << "e, "; // 5
            oss << "o, "; // 6
            oss << "a, "; // 7
            oss << "t0, "; // 8
            oss << "t1, "; // 9
            oss << "t2, "; // 10
            oss << "t3, "; // 11
            oss << "t4, "; // 12
            oss << "t5, "; // 13
            oss << "t6, "; // 14
            oss << "t7, "; // 15
            oss << "t8, "; // 16
            oss << "t9, "; // 17
            oss << "f, "; // 18
            oss << "resp0, "; // 19
            oss << "resp1, "; // 20
            oss << "resp2, "; // 21
            oss << "resp3, "; // 22
            oss << "resp4, "; // 23
            oss << "resp5, "; // 24
            oss << "resp6, "; // 25
            oss << "resp7, "; // 26
            oss << "resp8, "; // 27
            oss << "resp9, "; // 28
            oss << "stla, "; // 29
            oss << "stlo, "; // 30
            oss << "stel, "; // 31
            oss << "stdp, "; // 32
            oss << "evla, "; // 33
            oss << "evlo, "; // 34
            oss << "evel, "; // 35
            oss << "evdp, "; // 36
            oss << "mag, "; // 37
            oss << "user0, "; // 38
            oss << "user1, "; // 39
            oss << "user2, "; // 40
            oss << "user3, "; // 41
            oss << "user4, "; // 42
            oss << "user5, "; // 43
            oss << "user6, "; // 44
            oss << "user7, "; // 45
            oss << "user8, "; // 46
            oss << "user9, "; // 47
            oss << "dist, "; // 48
            oss << "az, "; // 49
            oss << "baz, "; // 50
            oss << "gcarc, "; // 51
            oss << "depmin, "; // 52
            oss << "depmen, "; // 53
            oss << "depmax, "; // 54
            oss << "cmpaz, "; // 55
            oss << "cmpinc, "; // 56
            oss << "reference_time, "; // 57
            oss << "norid, "; // 58
            oss << "nevid, "; // 59
            oss << "npts, "; // 60
            oss << "nwfid, "; // 61
            oss << "iftype, "; // 62
            oss << "idep, "; // 63
            oss << "iztype, "; // 64
            oss << "iinst, "; // 65
            oss << "istreg, "; // 66
            oss << "ievreg, "; // 67
            oss << "ievtyp, "; // 68
            oss << "iqual, "; // 69
            oss << "isynth, "; // 70
            oss << "imagtyp, "; // 71
            oss << "imagsrc, "; // 72
            oss << "ibody, "; // 73
            oss << "leven, "; // 74
            oss << "lpspol, "; // 75
            oss << "kstnm, "; // 76
            oss << "kevnm, "; // 77
            oss << "khole, "; // 78
            oss << "ko, "; // 79
            oss << "ka, "; // 80
            oss << "kt0, "; // 81
            oss << "kt1, "; // 82
            oss << "kt2, "; // 83
            oss << "kt3, "; // 84
            oss << "kt4, "; // 85
            oss << "kt5, "; // 86
            oss << "kt6, "; // 87
            oss << "kt7, "; // 88
            oss << "kt8, "; // 89
            oss << "kt9, "; // 90
            oss << "kf, "; // 91
            oss << "kuser0, "; // 92
            oss << "kuser1, "; // 93
            oss << "kuser2, "; // 94
            oss << "kcmpnm, "; // 95
            oss << "knetwk, "; // 96
            oss << "kinst, "; // 97
            oss << "data1, "; // 98
            oss << "data2)"; // 99
            oss << " Values (";
            for (int i{0}; i < 98; ++i)
            {
                oss << "?, ";
            }
            oss << "?)";
            std::string sq3_add_base_sac{oss.str()};
            sqlite3_stmt* sq3_statement{};
            sq3_result = sqlite3_prepare_v2(sq3_connection_file, sq3_add_base_sac.c_str(), -1, &sq3_statement, nullptr);
            // Bing values for the insert statement
            std::string filepath{file_path.parent_path().string()};
            sq3_result = sqlite3_bind_text(sq3_statement, 1, filepath.c_str(), -1, SQLITE_STATIC);
            std::string filename{file_path.filename().string()};
            sq3_result = sqlite3_bind_text(sq3_statement, 2, filename.c_str(), -1, SQLITE_STATIC);
            // If a header is unset, bind as null, otherwise bind with the actuall value
            // For now do unset double for all non-int numbers
            // then modify to unset_float for those that need it
            if (sac.delta == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 3); } else { sq3_result = sqlite3_bind_double(sq3_statement, 3, sac.delta); }
            if (sac.b == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 4); } else{ sq3_result = sqlite3_bind_double(sq3_statement, 4, sac.b); }
            if (sac.e == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 5); } else { sq3_result = sqlite3_bind_double(sq3_statement, 5, sac.e); }
            if (sac.o == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 6); } else { sq3_result = sqlite3_bind_double(sq3_statement, 6, sac.o); }
            if (sac.a == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 7); } else { sq3_result = sqlite3_bind_double(sq3_statement, 7, sac.a); }
            if (sac.t0 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 8); } else { sq3_result = sqlite3_bind_double(sq3_statement, 8, sac.t0); }
            if (sac.t1 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 9); } else { sq3_result = sqlite3_bind_double(sq3_statement, 9, sac.t1); }
            if (sac.t2 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 10); } else { sq3_result = sqlite3_bind_double(sq3_statement, 10, sac.t2); }
            if (sac.t3 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 11); } else { sq3_result = sqlite3_bind_double(sq3_statement, 11, sac.t3); }
            if (sac.t4 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 12); } else { sq3_result = sqlite3_bind_double(sq3_statement, 12, sac.t4); }
            if (sac.t5 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 13); } else { sq3_result = sqlite3_bind_double(sq3_statement, 13, sac.t5); }
            if (sac.t6 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 14); } else { sq3_result = sqlite3_bind_double(sq3_statement, 14, sac.t6); }
            if (sac.t7 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 15); } else { sq3_result = sqlite3_bind_double(sq3_statement, 15, sac.t7); }
            if (sac.t8 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 16); } else { sq3_result = sqlite3_bind_double(sq3_statement, 16, sac.t8); }
            if (sac.t9 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 17); } else { sq3_result = sqlite3_bind_double(sq3_statement, 17, sac.t9); }
            if (sac.f == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 18); } else { sq3_result = sqlite3_bind_double(sq3_statement, 18, sac.f); }
            if (sac.resp0 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 19); } else { sq3_result = sqlite3_bind_double(sq3_statement, 19, sac.resp0); }
            if (sac.resp1 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 20); } else { sq3_result = sqlite3_bind_double(sq3_statement, 20, sac.resp1); }
            if (sac.resp2 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 21); } else { sq3_result = sqlite3_bind_double(sq3_statement, 21, sac.resp2); }
            if (sac.resp3 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 22); } else { sq3_result = sqlite3_bind_double(sq3_statement, 22, sac.resp3); }
            if (sac.resp4 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 23); } else { sq3_result = sqlite3_bind_double(sq3_statement, 23, sac.resp4); }
            if (sac.resp5 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 24); } else { sq3_result = sqlite3_bind_double(sq3_statement, 24, sac.resp5); }
            if (sac.resp6 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 25); } else { sq3_result = sqlite3_bind_double(sq3_statement, 25, sac.resp6); }
            if (sac.resp7 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 26); } else { sq3_result = sqlite3_bind_double(sq3_statement, 26, sac.resp7); }
            if (sac.resp8 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 27); } else { sq3_result = sqlite3_bind_double(sq3_statement, 27, sac.resp8); }
            if (sac.resp9 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 28); } else { sq3_result = sqlite3_bind_double(sq3_statement, 28, sac.resp9); }
            if (sac.stla == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 29); } else { sq3_result = sqlite3_bind_double(sq3_statement, 29, sac.stla); }
            if (sac.stlo == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 30); } else { sq3_result = sqlite3_bind_double(sq3_statement, 30, sac.stlo); }
            if (sac.stel == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 31); } else { sq3_result = sqlite3_bind_double(sq3_statement, 31, sac.stel); }
            if (sac.stdp == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 32); } else { sq3_result = sqlite3_bind_double(sq3_statement, 32, sac.stdp); }
            if (sac.evla == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 33); } else { sq3_result = sqlite3_bind_double(sq3_statement, 33, sac.evla); }
            if (sac.evlo == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 34); } else { sq3_result = sqlite3_bind_double(sq3_statement, 34, sac.evlo); }
            if (sac.evel == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 35); } else { sq3_result = sqlite3_bind_double(sq3_statement, 35, sac.evel); }
            if (sac.evdp == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 36); } else { sq3_result = sqlite3_bind_double(sq3_statement, 36, sac.evdp); }
            if (sac.mag == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 37); } else { sq3_result = sqlite3_bind_double(sq3_statement, 37, sac.mag); }
            if (sac.user0 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 38); } else { sq3_result = sqlite3_bind_double(sq3_statement, 38, sac.user0); }
            if (sac.user1 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 39); } else { sq3_result = sqlite3_bind_double(sq3_statement, 39, sac.user1); }
            if (sac.user2 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 40); } else { sq3_result = sqlite3_bind_double(sq3_statement, 40, sac.user2); }
            if (sac.user3 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 41); } else { sq3_result = sqlite3_bind_double(sq3_statement, 41, sac.user3); }
            if (sac.user4 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 42); } else { sq3_result = sqlite3_bind_double(sq3_statement, 42, sac.user4); }
            if (sac.user5 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 43); } else { sq3_result = sqlite3_bind_double(sq3_statement, 43, sac.user5); }
            if (sac.user6 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 44); } else { sq3_result = sqlite3_bind_double(sq3_statement, 44, sac.user6); }
            if (sac.user7 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 45); } else { sq3_result = sqlite3_bind_double(sq3_statement, 45, sac.user7); }
            if (sac.user8 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 46); } else { sq3_result = sqlite3_bind_double(sq3_statement, 46, sac.user8); }
            if (sac.user9 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 47); } else { sq3_result = sqlite3_bind_double(sq3_statement, 47, sac.user9); }
            if (sac.dist == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 48); } else { sq3_result = sqlite3_bind_double(sq3_statement, 48, sac.dist); }
            if (sac.az == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 49); } else { sq3_result = sqlite3_bind_double(sq3_statement, 49, sac.az); }
            if (sac.baz == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 50); } else { sq3_result = sqlite3_bind_double(sq3_statement, 50, sac.baz); }
            if (sac.gcarc == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 51); } else { sq3_result = sqlite3_bind_double(sq3_statement, 51, sac.gcarc); }
            if (sac.depmin == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 52); } else { sq3_result = sqlite3_bind_double(sq3_statement, 52, sac.depmin); }
            if (sac.depmen == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 53); } else { sq3_result = sqlite3_bind_double(sq3_statement, 53, sac.depmen); }
            if (sac.depmax == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 54); } else { sq3_result = sqlite3_bind_double(sq3_statement, 54, sac.depmax); }
            if (sac.cmpaz == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 55); } else { sq3_result = sqlite3_bind_double(sq3_statement, 55, sac.cmpaz); }
            if (sac.cmpinc == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 56); } else { sq3_result = sqlite3_bind_double(sq3_statement, 56, sac.cmpinc); }
            std::string ref_time{sac_reference_time(sac)};
            // Need to handle if the nzSTUFF not set to make a null reference time
            sq3_result = sqlite3_bind_text(sq3_statement, 57, ref_time.c_str(), -1, SQLITE_STATIC);
            if (sac.norid == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 58); } else { sq3_result = sqlite3_bind_int(sq3_statement, 58, sac.norid); }
            if (sac.nevid == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 59); } else { sq3_result = sqlite3_bind_int(sq3_statement, 59, sac.nevid); }
            if (sac.npts == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 60); } else { sq3_result = sqlite3_bind_int(sq3_statement, 60, sac.npts); }
            if (sac.nwfid == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 61); } else { sq3_result = sqlite3_bind_int(sq3_statement, 61, sac.nwfid); }
            if (sac.iftype == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 62); } else { sq3_result = sqlite3_bind_int(sq3_statement, 62, sac.iftype); }
            if (sac.idep == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 63); } else { sq3_result = sqlite3_bind_int(sq3_statement, 63, sac.idep); }
            if (sac.iztype == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 64); } else { sq3_result = sqlite3_bind_int(sq3_statement, 64, sac.iztype); }
            if (sac.iinst == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 65); } else { sq3_result = sqlite3_bind_int(sq3_statement, 65, sac.iinst); }
            if (sac.istreg == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 66); } else { sq3_result = sqlite3_bind_int(sq3_statement, 66, sac.istreg); }
            if (sac.ievreg == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 67); } else { sq3_result = sqlite3_bind_int(sq3_statement, 67, sac.ievreg); }
            if (sac.ievtyp == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 68); } else { sq3_result = sqlite3_bind_int(sq3_statement, 68, sac.ievtyp); }
            if (sac.iqual == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 69); } else { sq3_result = sqlite3_bind_int(sq3_statement, 69, sac.iqual); }
            if (sac.istreg == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 70); } else { sq3_result = sqlite3_bind_int(sq3_statement, 70, sac.isynth); }
            if (sac.imagtyp == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 71); } else { sq3_result = sqlite3_bind_int(sq3_statement, 71, sac.imagtyp); }
            if (sac.imagsrc == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 72); } else { sq3_result = sqlite3_bind_int(sq3_statement, 72, sac.imagsrc); }
            if (sac.ibody == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 73); } else { sq3_result = sqlite3_bind_int(sq3_statement, 73, sac.ibody); }
            // If not set, they're 0 anyway
            sq3_result = sqlite3_bind_int(sq3_statement, 74, sac.leven);
            sq3_result = sqlite3_bind_int(sq3_statement, 75, sac.lpspol);
            std::string trim_unset_word{SAC::unset_word};
            boost::algorithm::trim(trim_unset_word);
            boost::algorithm::trim(sac.kstnm);
            if (sac.kstnm == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 76); } else { sq3_result = sqlite3_bind_text(sq3_statement, 76, sac.kstnm.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kevnm);
            if (sac.kevnm == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 77); } else { sq3_result = sqlite3_bind_text(sq3_statement, 77, sac.kevnm.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.khole);
            if (sac.khole == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 78); } else { sq3_result = sqlite3_bind_text(sq3_statement, 78, sac.khole.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.ko);
            if (sac.ko == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 79); } else { sq3_result = sqlite3_bind_text(sq3_statement, 79, sac.ko.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.ka);
            if (sac.ka == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 80); } else { sq3_result = sqlite3_bind_text(sq3_statement, 80, sac.ka.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt0);
            if (sac.kt0 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 81); } else { sq3_result = sqlite3_bind_text(sq3_statement, 81, sac.kt0.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt1);
            if (sac.kt1 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 82); } else { sq3_result = sqlite3_bind_text(sq3_statement, 82, sac.kt1.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt2);
            if (sac.kt2 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 83); } else { sq3_result = sqlite3_bind_text(sq3_statement, 83, sac.kt2.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt3);
            if (sac.kt3 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 84); } else { sq3_result = sqlite3_bind_text(sq3_statement, 84, sac.kt3.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt4);
            if (sac.kt4 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 85); } else { sq3_result = sqlite3_bind_text(sq3_statement, 85, sac.kt4.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt5);
            if (sac.kt5 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 86); } else { sq3_result = sqlite3_bind_text(sq3_statement, 86, sac.kt5.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt6);
            if (sac.kt6 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 87); } else { sq3_result = sqlite3_bind_text(sq3_statement, 87, sac.kt6.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt7);
            if (sac.kt7 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 88); } else { sq3_result = sqlite3_bind_text(sq3_statement, 88, sac.kt7.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt8);
            if (sac.kt8 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 89); } else { sq3_result = sqlite3_bind_text(sq3_statement, 89, sac.kt8.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt9);
            if (sac.kt9 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 90); } else { sq3_result = sqlite3_bind_text(sq3_statement, 90, sac.kt9.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kf);
            if (sac.kf == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 91); } else { sq3_result = sqlite3_bind_text(sq3_statement, 91, sac.kf.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kuser0);
            if (sac.kuser0 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 92); } else { sq3_result = sqlite3_bind_text(sq3_statement, 92, sac.kuser0.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kuser1);
            if (sac.kuser1 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 93); } else { sq3_result = sqlite3_bind_text(sq3_statement, 93, sac.kuser1.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kuser2);
            if (sac.kuser2 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 94); } else { sq3_result = sqlite3_bind_text(sq3_statement, 94, sac.kuser2.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kcmpnm);
            if (sac.kcmpnm == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 95); } else { sq3_result = sqlite3_bind_text(sq3_statement, 95, sac.kcmpnm.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.knetwk);
            if (sac.knetwk == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 96); } else { sq3_result = sqlite3_bind_text(sq3_statement, 96, sac.knetwk.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kinst);
            if (sac.kinst == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 97); } else { sq3_result = sqlite3_bind_text(sq3_statement, 97, sac.kinst.c_str(), -1, SQLITE_STATIC); }
            sq3_result = sqlite3_bind_blob(sq3_statement, 98, sac.data1.data(), sac.data1.size() * sizeof(double), SQLITE_STATIC);
            // This auto does null if empty
            sq3_result = sqlite3_bind_blob(sq3_statement, 99, sac.data2.data(), sac.data2.size() * sizeof(double), SQLITE_STATIC);
            // Execute the statement
            sq3_result = sqlite3_step(sq3_statement);
            // Finalize the statement
            sqlite3_finalize(sq3_statement);
        }
        //----------------------------------------------------------------
        // End Add base data
        //----------------------------------------------------------------

};
};

#endif
