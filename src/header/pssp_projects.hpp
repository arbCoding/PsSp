#ifndef PSSP_PROJECTS_HPP
#define PSSP_PROJECTS_HPP

#include "sac_io.hpp"
#include "sac_stream.hpp"
#include "pssp_threadpool.hpp"
#include <ios>
#include <sqlite3.h>
// String comparisons in C++ suck, boost adds needed functionality!
#include <boost/algorithm/string.hpp>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <filesystem>
#include <iostream>
#include <sstream>

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
// TBD
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
        // Checkpoint id
        int checkpoint_id_{0};
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
        // Day of Year to Month and Day of Month
        //----------------------------------------------------------------
        std::string doy_2_ymd(int year, int doy)
        {
            // Standard days in month for non-leap years
            constexpr int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            // Being a leap year only matters is the DoY is >= 31 + 28 = 59
            int month{0};
            int days{0};
            // Infinite loop if we don't break out of it
            while (true)
            {
                days = days_in_month[month];
                if (month == 1)
                {
                    // February, so we care if leap year or not
                    // Check if it is a leap year
                    if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
                    {
                        // Is a leap year
                        ++days;
                    }
                }
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
        //----------------------------------------------------------------
        // End Day of Year to Month and Day of Month
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Make datetime
        //----------------------------------------------------------------
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
            // We cannot have this work in an OS-independent way
            // unless we handle this manually (unfortunately)
            // So I need to go from YYYY-JJJ to YYYY-MM-DD
            // manually while account for leap years
            return oss.str();
        }
        //----------------------------------------------------------------
        // End Make datetime
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Provenance table creation
        //----------------------------------------------------------------
        // This only gets called upon creating a NEW project
        // This is ONLY in the on-disk database file
        // Not stored in memory
        void create_provenance_table()
        {
            std::ostringstream oss{};
            oss << "CREATE TABLE provenance (";
            oss << "base_id INTEGER PRIMARY KEY, "; // 1 (automatically generated id)
            // If from file on filesystem, full canonical path for it
            // If from an FDSN (future), then that info
            // If internally generated (synthetic, new copy, whatever) then internal
            oss << "source TEXT, "; // 2
            oss << "added DATETIME DEFAULT CURRENT_TIMESTAMP, "; // 3 (automatic)
            oss << "removed DATETIME);"; // 4 (NULL if not removed, timestamp if removed)
            std::string sq3_string{oss.str()};
            sq3_result = sqlite3_exec(sq3_connection_file, sq3_string.c_str(), nullptr, nullptr, &sq3_error_message);
        }
        //----------------------------------------------------------------
        // End provenance table creation
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Checkpoint list table creation
        //----------------------------------------------------------------
        // This only gets called upon creating a NEW project
        // This is ONLY in the on-disk database file
        // Not stored in memory
        void create_checkpoint_list_table()
        {
            std::ostringstream oss{};
            oss << "CREATE TABLE checkpoints (";
            oss << "checkpoint_id INTEGER PRIMARY KEY, "; // 1 (automatically generated id)
            oss << "parent_id INTEGER, "; // 2 (id of parent checkpoint this spawned from)
            oss << "created DATETIME DEFAULT CURRENT_TIMESTAMP, "; // 3 (when the checkpoint was made)
            oss << "author INTEGER, "; // 4 (0 = automatic, 1 = user)
            oss << "cull INTEGER, "; // 5 (0 = can cull on limit hit, 1 = keep)
            oss << "removed DATETIME)"; // 7 (NULL = in data, a timestamp = when it was removed)
            std::string sq3_string{oss.str()};
            sq3_result = sqlite3_exec(sq3_connection_file, sq3_string.c_str(), nullptr, nullptr, &sq3_error_message);
        }
        //----------------------------------------------------------------
        // End checkpoint list table creation
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // data checkpoint table creation
        //----------------------------------------------------------------
        // This only gets called upon a piece of data being added to the project
        // This is stored on disk
        void create_data_checkpoint_table(int data_id)
        {
            std::ostringstream oss{};
            oss << "CREATE TABLE checkpoint_" << data_id << " (";
            oss << "checkpoint_id INTEGER, "; // 1, this is the checkpoint id the header is associated with
            // Now we need to include the ~100 header values...
            //------------------------------------------------------------
            // SAC headers
            //------------------------------------------------------------
            oss << "delta REAL, "; // 2 (sac.delta, sample rate)
            oss << "b REAL, "; // 3 (sac.b, begin time in seconds)
            oss << "e REAL, "; // 4 (sac.e, end time in seconds)
            oss << "o REAL, "; // 5 (sac.o, origin time in seconds)
            oss << "a REAL, "; // 6 (sac.a, first arrival pick time in seconds)
            oss << "t0 REAL, "; // 7 (sac.t0, user time in seconds)
            oss << "t1 REAL, "; // 8
            oss << "t2 REAL, "; // 9
            oss << "t3 REAL, "; // 10
            oss << "t4 REAL, "; // 11
            oss << "t5 REAL, "; // 12
            oss << "t6 REAL, "; // 13
            oss << "t7 REAL, "; // 14
            oss << "t8 REAL, "; // 15
            oss << "t9 REAL, "; // 16
            oss << "f REAL, "; // 17 (sac.f, fini time in seconds [for finite source])
            oss << "resp0 REAL, "; // 18 (sac.resp0, instrument response parameter)
            oss << "resp1 REAL, "; // 19
            oss << "resp2 REAL, "; // 20
            oss << "resp3 REAL, "; // 21
            oss << "resp4 REAL, "; // 22
            oss << "resp5 REAL, "; // 23
            oss << "resp6 REAL, "; // 24
            oss << "resp7 REAL, "; // 25
            oss << "resp8 REAL, "; // 26
            oss << "resp9 REAL, "; // 27
            oss << "stla REAL, "; // 28 (sac.stla, station latitude, degrees, north positive)
            oss << "stlo REAL, "; // 29 (sac.stlo, station longitude, degrees, east positive)
            oss << "stel REAL, "; // 30 (sac.stel, station elevation m a.s.l.)
            oss << "stdp REAL, "; // 31 (sac.stdp, station depth below surface, meters)
            oss << "evla REAL, "; // 32 (sac.evla, event latitude)
            oss << "evlo REAL, "; // 33 (sac.evlo, event longitude)
            oss << "evel REAL, "; // 34 (sac.evel, event elevation)
            oss << "evdp REAL, "; // 35 (sac.evdp, event depth below surface, kilometers [previous meters])
            oss << "mag REAL, "; // 36 (sac.mag, event magnitude)
            oss << "user0 REAL, "; // 37 (sac.user0, user-defined storage)
            oss << "user1 REAL, "; // 38
            oss << "user2 REAL, "; // 39
            oss << "user3 REAL, "; // 40
            oss << "user4 REAL, "; // 41
            oss << "user5 REAL, "; // 42
            oss << "user6 REAL, "; // 43
            oss << "user7 REAL, "; // 44
            oss << "user8 REAL, "; // 45
            oss << "user9 REAL, "; // 46
            oss << "dist REAL, "; // 47 (sac.dist, station-event distance, kilometers)
            oss << "az REAL, "; // 48 (sac.az, station-event azimuth, degrees)
            oss << "baz REAL, "; // 49 (sac.baz, event-station azimuth, degrees)
            oss << "gcarc REAL, "; // 50 (sac.gcarc, station_event great circle-arc distance, degrees)
            oss << "depmin REAL, "; // 51 (sac.depmin, minimum amplitude)
            oss << "depmen REAL, "; // 52 (sac.depmen, mean amplitude)
            oss << "depmax REAL, "; // 53 (sac.depmax, maximum amplitude)
            oss << "cmpaz REAL, "; // 54 (sac.cmpaz, component azimuth, degrees clockwise from North)
            oss << "cmpinc REAL, "; // 55 (sac.cmpinc, component incident angle, degrees from upward vertical (incident 0 = -90 dip), (dip 0 = incident 90)
            oss << "reference_time DATETIME, "; // 56 (take all sac.nzStuff headers and format into a string, then convert to standard format)
            oss << "norid INTEGER, "; // 57 (sac.norid, origin id)
            oss << "nevid INTEGER, "; // 58 (sac.nevid, event id)
            oss << "npts INTEGER, "; // 59 (sac.npts, number of points in time-series)
            oss << "nwfid INTEGER, "; // 60 (sac.nwfid, waveform id)
            oss << "iftype INTEGER, "; // 61 (sac.iftype, type of file (should always be 1 = ITIME = Time-series!))
            oss << "idep INTEGER, "; // 62 (sac.idep, amplitude type, see SAC::SacStream for details)
            oss << "iztype INTEGER, "; // 63 (sac.iztype, reference time equivalent, see SAC::SacStream for details)
            oss << "iinst INTEGER, "; // 64 (sac.iints, type of recording instrument)
            oss << "istreg INTEGER, "; // 65 (sac.istreg, station geographic region)
            oss << "ievreg INTEGER, "; // 66 (sac.ievreg, event geographic region)
            oss << "ievtyp INTEGER, "; // 67 (sac.ievtyp, type of event)
            oss << "iqual INTEGER, "; // 68 (sac.iqual, data quality)
            oss << "isynth INTEGER, "; // 69 (sac.isynth, synthetic data flag)
            oss << "imagtyp INTEGER, "; // 70 (sac.imagtyp, type of magnitude)
            oss << "imagsrc INTEGER, "; // 71 (sac.imagsrc, source of magnitude information)
            oss << "ibody INTEGER, "; // 72 (sac.ibody, body/spheroid definition)
            // True/False = 1/0
            oss << "leven INTEGER, "; // 73 (sac.leven, evenly-spaced flag)
            oss << "lpspol INTEGER, "; // 74 (sac.lpspol, positive polarity (left-hand reul NEZ [North-East-Up]))
            // Skip lovrok and lcalda because they're stupider headers than normal
            oss << "kstnm TEXT, "; // 75 (sac.kstnm, station name)
            oss << "kevnm TEXT, "; // 76 (sac.kevnm, event name)
            oss << "khole TEXT, "; // 77 (sac.khole, hole identifier or location ID)
            oss << "ko TEXT, "; // 78 (sac.ko, origin time text)
            oss << "ka TEXT, "; // 79 (sac.ka, first arrival time text)
            oss << "kt0 TEXT, "; // 80 (sac.kt0, user time text)
            oss << "kt1 TEXT, "; // 81
            oss << "kt2 TEXT, "; // 82
            oss << "kt3 TEXT, "; // 83
            oss << "kt4 TEXT, "; // 84
            oss << "kt5 TEXT, "; // 85
            oss << "kt6 TEXT, "; // 86
            oss << "kt7 TEXT, "; // 87
            oss << "kt8 TEXT, "; // 88
            oss << "kt9 TEXT, "; // 89
            oss << "kf TEXT, "; // 90 (sac.kf, fini text)
            oss << "kuser0 TEXT, "; // 91 (sac.kuser0, user text storage)
            oss << "kuser1 TEXT, "; // 92
            oss << "kuser2 TEXT, "; // 93
            oss << "kcmpnm TEXT, "; // 94 (sac.kcmpnm, compnent name)
            oss << "knetwk TEXT, "; // 95 (sac.knetwk, network name)
            // Skip kdatrd, stupid
            oss << "kinst TEXT, "; // 96 (sac.kinst, generic recording instrument name)
            //------------------------------------------------------------
            // End SAC headers
            //------------------------------------------------------------
            oss << "data1 BLOB, "; // 97 (sac.data1, time-series) (null if not checkpoint)
            oss << "data2 BLOB);"; // 98 (sac.data1, if unevenly sampled, these are the sample times) (null if evenly sampled or not a checkpoint)
            std::string sq3_string{oss.str()};
            sq3_result = sqlite3_exec(sq3_connection_file, sq3_string.c_str(), nullptr, nullptr, &sq3_error_message);
            if (sq3_result != SQLITE_OK)
            {
                std::cout << sqlite3_errmsg(sq3_connection_file) << '\n';
            }
        }
        //----------------------------------------------------------------
        // End data checkpoint table creation
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // data processing table creation
        //----------------------------------------------------------------
        // This only gets called upon a piece of data being added to the project
        // This is stored on disk by default, no need to keep data in memory (it is already there!)
        void create_data_processing_table(int data_id)
        {
            std::ostringstream oss{};
            oss << "CREATE TABLE processing_" << data_id << " (";
            oss << "checkpoint_id INTEGER, "; // 1, this is the checkpoint id the data is associated with
            oss << "comment TEXT);"; // 2 (on processing it is the action and parameters, on checkpoint it is CHECKPOINT)
            std::string sq3_string{oss.str()};
            sq3_result = sqlite3_exec(sq3_connection_file, sq3_string.c_str(), nullptr, nullptr, &sq3_error_message);
        }
        //----------------------------------------------------------------
        // End data processing table creation
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Add data provenance
        //----------------------------------------------------------------
        // This gets called ONLY when data gets added to a project
        // You get the data_id back upon insertion
        int add_data_provenance(const std::string& source)
        {
            std::ostringstream oss{};
            oss << "INSERT INTO provenance (source) VALUES (?)";
            std::string sq3_string{oss.str()};
            sqlite3_stmt* sq3_statement{};
            sq3_result = sqlite3_prepare_v2(sq3_connection_file, sq3_string.c_str(), -1, &sq3_statement, nullptr);
            sq3_result = sqlite3_bind_text(sq3_statement, 1, source.c_str(), -1, SQLITE_STATIC);
            // Execute the statement
            sq3_result = sqlite3_step(sq3_statement);
            // Get the ID of the newly inserted data
            // Normally it is an sqlite3_int64, but I want a normal integer for now
            int data_id{static_cast<int>(sqlite3_last_insert_rowid(sq3_connection_file))};
            // Finalize the statement
            sqlite3_finalize(sq3_statement);
            return data_id;
        }
        //----------------------------------------------------------------
        // End Add data provenance
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Project connection initializer
        //----------------------------------------------------------------
        void connect()
        {
            // Create a new connection
            sq3_result = sqlite3_open_v2(path_.c_str(), &sq3_connection_file, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
            // Set the journal mode to WAL
            sq3_result = sqlite3_exec(sq3_connection_file, "PRAGMA journal_mode=WAL", nullptr, nullptr, &sq3_error_message);
            // Create a new connection
            sq3_result = sqlite3_open_v2(":memory:", &sq3_connection_memory, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
            // Set the journal mode to WAL
            sq3_result = sqlite3_exec(sq3_connection_memory, "PRAGMA journal_mode=WAL", nullptr, nullptr, &sq3_error_message);
        }
        //----------------------------------------------------------------
        // End Project connection initializer
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // New project tables
        //----------------------------------------------------------------
        void fresh_tables()
        {
            // Create the base provenance and checkpoint tables
            create_provenance_table();
            create_checkpoint_list_table();
        }
        //----------------------------------------------------------------
        // End New project tables
        //----------------------------------------------------------------
    public:
        // Connection to the database (file)
        sqlite3* sq3_connection_file{};
        // Connection to the database (memory)
        sqlite3* sq3_connection_memory{};
        int sq3_result{};
        char* sq3_error_message{};
        // Empty constructor
        Project() {};
        //----------------------------------------------------------------
        // Parameterized Constructor
        //----------------------------------------------------------------
        Project(std::string name, std::filesystem::path base_path) : name_{name}, path_{base_path / (name + ".proj")}
        {
            connect();
            fresh_tables();
        }
        //----------------------------------------------------------------
        // End Parameterized Constructor
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Connect to existing project
        //----------------------------------------------------------------
        void connect_2_existing(std::filesystem::path full_path)
        {
            name_ = full_path.stem().string();
            path_ = full_path;
            connect();
        }
        //----------------------------------------------------------------
        // End Connect to existing project
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Setter to modify the project object
        //----------------------------------------------------------------
        void new_project(std::string name, std::filesystem::path base_path)
        {
            name_ = name;
            path_ = base_path / (name + ".proj");
            connect();
            fresh_tables();
        }
        //----------------------------------------------------------------
        // End setter to modify the project object
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Destructor
        //----------------------------------------------------------------
        ~Project() 
        {
            // Close connections
            sqlite3_close_v2(sq3_connection_file);
            sqlite3_close_v2(sq3_connection_memory);
            // Remove extraneous files (shm = shared memory, wal = write ahead log)
            std::filesystem::path shm_file{path_};
            shm_file += "-shm";
            std::filesystem::remove(shm_file);
            std::filesystem::path wal_file{path_};
            wal_file += "-wal";
            std::filesystem::remove(wal_file);
        }
        //----------------------------------------------------------------
        // End Destructor
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Add data to project
        //----------------------------------------------------------------
        int add_sac(SAC::SacStream& sac, const std::string& source)
        {
            int data_id{add_data_provenance(source)};
            create_data_checkpoint_table(data_id);
            add_data_checkpoint(sac, data_id);
            create_data_processing_table(data_id);
            add_data_processing(data_id, "ADD");
            return data_id;
        }
        //----------------------------------------------------------------
        // End Add data to project
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Write checkpoint
        //----------------------------------------------------------------
        void write_checkpoint(bool author, bool cull)
        {
            std::ostringstream oss{};
            oss << "INSERT INTO checkpoints (";
            oss << "parent_id, "; // 1
            oss << "author, "; // 2
            oss << "cull)"; // 3
            oss << " VALUES (?, ?, ?);";
            std::string sq3_string{oss.str()};
            sqlite3_stmt* sq3_statement{};
            sq3_result = sqlite3_prepare_v2(sq3_connection_file, sq3_string.c_str(), -1, &sq3_statement, nullptr);
            sq3_result = sqlite3_bind_int(sq3_statement, 1, checkpoint_id_);
            if (author) { sq3_result = sqlite3_bind_int(sq3_statement, 2, 1); } else { sq3_result = sqlite3_bind_int(sq3_statement, 2, 0); }
            if (cull) { sq3_result = sqlite3_bind_int(sq3_statement, 3, 1); } else { sq3_result = sqlite3_bind_int(sq3_statement, 3, 0); }
            // Execute the statement
            sq3_result = sqlite3_step(sq3_statement);
            // Get the ID of the newly inserted data
            // Normally it is an sqlite3_int64, but I want a normal integer for now
            checkpoint_id_ = static_cast<int>(sqlite3_last_insert_rowid(sq3_connection_file));
            // Finalize the statement
            sqlite3_finalize(sq3_statement);
        }
        //----------------------------------------------------------------
        // End Write checkpoint
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Add data checkpoint
        //----------------------------------------------------------------
        // This gets called when data gets added to a project
        // OR when a checkpoint gets made
        // This goes on file when data is added, or when a checkpoint happens
        void add_data_checkpoint(SAC::SacStream& sac, int data_id)
        {
            std::ostringstream oss{};
            oss << "INSERT INTO checkpoint_" << data_id << " (";
            oss << "delta, "; // 1
            oss << "b, "; // 2
            oss << "e, "; // 3
            oss << "o, "; // 4
            oss << "a, "; // 5
            oss << "t0, "; // 6
            oss << "t1, "; // 7
            oss << "t2, "; // 8
            oss << "t3, "; // 9
            oss << "t4, "; // 10
            oss << "t5, "; // 11
            oss << "t6, "; // 12
            oss << "t7, "; // 13
            oss << "t8, "; // 14
            oss << "t9, "; // 15
            oss << "f, "; // 16
            oss << "resp0, "; // 17
            oss << "resp1, "; // 18
            oss << "resp2, "; // 19
            oss << "resp3, "; // 20
            oss << "resp4, "; // 21
            oss << "resp5, "; // 22
            oss << "resp6, "; // 23
            oss << "resp7, "; // 24
            oss << "resp8, "; // 25
            oss << "resp9, "; // 26
            oss << "stla, "; // 27
            oss << "stlo, "; // 28
            oss << "stel, "; // 29
            oss << "stdp, "; // 30
            oss << "evla, "; // 31
            oss << "evlo, "; // 32
            oss << "evel, "; // 33
            oss << "evdp, "; // 34
            oss << "mag, "; // 35
            oss << "user0, "; // 36
            oss << "user1, "; // 37
            oss << "user2, "; // 38
            oss << "user3, "; // 39
            oss << "user4, "; // 40
            oss << "user5, "; // 41
            oss << "user6, "; // 42
            oss << "user7, "; // 43
            oss << "user8, "; // 44
            oss << "user9, "; // 45
            oss << "dist, "; // 46
            oss << "az, "; // 47
            oss << "baz, "; // 48
            oss << "gcarc, "; // 49
            oss << "depmin, "; // 50
            oss << "depmen, "; // 51
            oss << "depmax, "; // 52
            oss << "cmpaz, "; // 53
            oss << "cmpinc, "; // 54
            oss << "reference_time, "; // 55
            oss << "norid, "; // 56
            oss << "nevid, "; // 57
            oss << "npts, "; // 58
            oss << "nwfid, "; // 59
            oss << "iftype, "; // 60
            oss << "idep, "; // 61
            oss << "iztype, "; // 62
            oss << "iinst, "; // 63
            oss << "istreg, "; // 64
            oss << "ievreg, "; // 65
            oss << "ievtyp, "; // 66
            oss << "iqual, "; // 67
            oss << "isynth, "; // 68
            oss << "imagtyp, "; // 69
            oss << "imagsrc, "; // 70
            oss << "ibody, "; // 71
            oss << "leven, "; // 72
            oss << "lpspol, "; // 73
            oss << "kstnm, "; // 74
            oss << "kevnm, "; // 75
            oss << "khole, "; // 76
            oss << "ko, "; // 77
            oss << "ka, "; // 78
            oss << "kt0, "; // 79
            oss << "kt1, "; // 80
            oss << "kt2, "; // 81
            oss << "kt3, "; // 82
            oss << "kt4, "; // 83
            oss << "kt5, "; // 84
            oss << "kt6, "; // 85
            oss << "kt7, "; // 86
            oss << "kt8, "; // 87
            oss << "kt9, "; // 88
            oss << "kf, "; // 89
            oss << "kuser0, "; // 90
            oss << "kuser1, "; // 91
            oss << "kuser2, "; // 92
            oss << "kcmpnm, "; // 93
            oss << "knetwk, "; // 94
            oss << "kinst, "; // 95
            oss << "checkpoint_id, "; // 96
            oss << "data1, "; // 97
            oss << "data2)"; // 98
            oss << " Values (";
            for (int i{0}; i < 97; ++i)
            {
                oss << "?, ";
            }
            oss << "?);";
            std::string sq3_string{oss.str()};
            sqlite3_stmt* sq3_statement{};
            sq3_result = sqlite3_prepare_v2(sq3_connection_file, sq3_string.c_str(), -1, &sq3_statement, nullptr);
            // If a header is unset, bind as null, otherwise bind with the actuall value
            // For now do unset double for all non-int numbers
            // then modify to unset_float for those that need it
            if (sac.delta == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 1); } else { sq3_result = sqlite3_bind_double(sq3_statement, 1, sac.delta); }
            if (sac.b == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 2); } else{ sq3_result = sqlite3_bind_double(sq3_statement, 2, sac.b); }
            if (sac.e == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 3); } else { sq3_result = sqlite3_bind_double(sq3_statement, 3, sac.e); }
            if (sac.o == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 4); } else { sq3_result = sqlite3_bind_double(sq3_statement, 4, sac.o); }
            if (sac.a == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 5); } else { sq3_result = sqlite3_bind_double(sq3_statement, 5, sac.a); }
            if (sac.t0 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 6); } else { sq3_result = sqlite3_bind_double(sq3_statement, 6, sac.t0); }
            if (sac.t1 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 7); } else { sq3_result = sqlite3_bind_double(sq3_statement, 7, sac.t1); }
            if (sac.t2 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 8); } else { sq3_result = sqlite3_bind_double(sq3_statement, 8, sac.t2); }
            if (sac.t3 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 9); } else { sq3_result = sqlite3_bind_double(sq3_statement, 9, sac.t3); }
            if (sac.t4 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 10); } else { sq3_result = sqlite3_bind_double(sq3_statement, 10, sac.t4); }
            if (sac.t5 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 11); } else { sq3_result = sqlite3_bind_double(sq3_statement, 11, sac.t5); }
            if (sac.t6 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 12); } else { sq3_result = sqlite3_bind_double(sq3_statement, 12, sac.t6); }
            if (sac.t7 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 13); } else { sq3_result = sqlite3_bind_double(sq3_statement, 13, sac.t7); }
            if (sac.t8 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 14); } else { sq3_result = sqlite3_bind_double(sq3_statement, 14, sac.t8); }
            if (sac.t9 == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 15); } else { sq3_result = sqlite3_bind_double(sq3_statement, 15, sac.t9); }
            if (sac.f == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 16); } else { sq3_result = sqlite3_bind_double(sq3_statement, 16, sac.f); }
            if (sac.resp0 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 17); } else { sq3_result = sqlite3_bind_double(sq3_statement, 17, sac.resp0); }
            if (sac.resp1 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 18); } else { sq3_result = sqlite3_bind_double(sq3_statement, 18, sac.resp1); }
            if (sac.resp2 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 19); } else { sq3_result = sqlite3_bind_double(sq3_statement, 19, sac.resp2); }
            if (sac.resp3 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 20); } else { sq3_result = sqlite3_bind_double(sq3_statement, 20, sac.resp3); }
            if (sac.resp4 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 21); } else { sq3_result = sqlite3_bind_double(sq3_statement, 21, sac.resp4); }
            if (sac.resp5 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 22); } else { sq3_result = sqlite3_bind_double(sq3_statement, 22, sac.resp5); }
            if (sac.resp6 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 23); } else { sq3_result = sqlite3_bind_double(sq3_statement, 23, sac.resp6); }
            if (sac.resp7 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 24); } else { sq3_result = sqlite3_bind_double(sq3_statement, 24, sac.resp7); }
            if (sac.resp8 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 25); } else { sq3_result = sqlite3_bind_double(sq3_statement, 25, sac.resp8); }
            if (sac.resp9 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 26); } else { sq3_result = sqlite3_bind_double(sq3_statement, 26, sac.resp9); }
            if (sac.stla == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 27); } else { sq3_result = sqlite3_bind_double(sq3_statement, 27, sac.stla); }
            if (sac.stlo == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 28); } else { sq3_result = sqlite3_bind_double(sq3_statement, 28, sac.stlo); }
            if (sac.stel == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 29); } else { sq3_result = sqlite3_bind_double(sq3_statement, 29, sac.stel); }
            if (sac.stdp == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 30); } else { sq3_result = sqlite3_bind_double(sq3_statement, 30, sac.stdp); }
            if (sac.evla == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 31); } else { sq3_result = sqlite3_bind_double(sq3_statement, 31, sac.evla); }
            if (sac.evlo == SAC::unset_double) { sq3_result = sqlite3_bind_null(sq3_statement, 32); } else { sq3_result = sqlite3_bind_double(sq3_statement, 32, sac.evlo); }
            if (sac.evel == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 33); } else { sq3_result = sqlite3_bind_double(sq3_statement, 33, sac.evel); }
            if (sac.evdp == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 34); } else { sq3_result = sqlite3_bind_double(sq3_statement, 34, sac.evdp); }
            if (sac.mag == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 35); } else { sq3_result = sqlite3_bind_double(sq3_statement, 35, sac.mag); }
            if (sac.user0 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 36); } else { sq3_result = sqlite3_bind_double(sq3_statement, 36, sac.user0); }
            if (sac.user1 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 37); } else { sq3_result = sqlite3_bind_double(sq3_statement, 37, sac.user1); }
            if (sac.user2 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 38); } else { sq3_result = sqlite3_bind_double(sq3_statement, 38, sac.user2); }
            if (sac.user3 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 39); } else { sq3_result = sqlite3_bind_double(sq3_statement, 39, sac.user3); }
            if (sac.user4 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 40); } else { sq3_result = sqlite3_bind_double(sq3_statement, 40, sac.user4); }
            if (sac.user5 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 41); } else { sq3_result = sqlite3_bind_double(sq3_statement, 41, sac.user5); }
            if (sac.user6 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 42); } else { sq3_result = sqlite3_bind_double(sq3_statement, 42, sac.user6); }
            if (sac.user7 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 43); } else { sq3_result = sqlite3_bind_double(sq3_statement, 43, sac.user7); }
            if (sac.user8 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 44); } else { sq3_result = sqlite3_bind_double(sq3_statement, 44, sac.user8); }
            if (sac.user9 == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 45); } else { sq3_result = sqlite3_bind_double(sq3_statement, 45, sac.user9); }
            if (sac.dist == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 46); } else { sq3_result = sqlite3_bind_double(sq3_statement, 46, sac.dist); }
            if (sac.az == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 47); } else { sq3_result = sqlite3_bind_double(sq3_statement, 47, sac.az); }
            if (sac.baz == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 48); } else { sq3_result = sqlite3_bind_double(sq3_statement, 48, sac.baz); }
            if (sac.gcarc == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 49); } else { sq3_result = sqlite3_bind_double(sq3_statement, 49, sac.gcarc); }
            if (sac.depmin == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 50); } else { sq3_result = sqlite3_bind_double(sq3_statement, 50, sac.depmin); }
            if (sac.depmen == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 51); } else { sq3_result = sqlite3_bind_double(sq3_statement, 51, sac.depmen); }
            if (sac.depmax == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 52); } else { sq3_result = sqlite3_bind_double(sq3_statement, 52, sac.depmax); }
            if (sac.cmpaz == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 53); } else { sq3_result = sqlite3_bind_double(sq3_statement, 53, sac.cmpaz); }
            if (sac.cmpinc == SAC::unset_float) { sq3_result = sqlite3_bind_null(sq3_statement, 54); } else { sq3_result = sqlite3_bind_double(sq3_statement, 54, sac.cmpinc); }
            std::string ref_time{sac_reference_time(sac)};
            // Need to handle if the nzSTUFF not set to make a null reference time
            sq3_result = sqlite3_bind_text(sq3_statement, 55, ref_time.c_str(), -1, SQLITE_STATIC);
            if (sac.norid == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 56); } else { sq3_result = sqlite3_bind_int(sq3_statement, 56, sac.norid); }
            if (sac.nevid == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 57); } else { sq3_result = sqlite3_bind_int(sq3_statement, 57, sac.nevid); }
            if (sac.npts == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 58); } else { sq3_result = sqlite3_bind_int(sq3_statement, 58, sac.npts); }
            if (sac.nwfid == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 59); } else { sq3_result = sqlite3_bind_int(sq3_statement, 59, sac.nwfid); }
            if (sac.iftype == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 60); } else { sq3_result = sqlite3_bind_int(sq3_statement, 60, sac.iftype); }
            if (sac.idep == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 61); } else { sq3_result = sqlite3_bind_int(sq3_statement, 61, sac.idep); }
            if (sac.iztype == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 62); } else { sq3_result = sqlite3_bind_int(sq3_statement, 62, sac.iztype); }
            if (sac.iinst == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 63); } else { sq3_result = sqlite3_bind_int(sq3_statement, 63, sac.iinst); }
            if (sac.istreg == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 64); } else { sq3_result = sqlite3_bind_int(sq3_statement, 64, sac.istreg); }
            if (sac.ievreg == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 65); } else { sq3_result = sqlite3_bind_int(sq3_statement, 65, sac.ievreg); }
            if (sac.ievtyp == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 66); } else { sq3_result = sqlite3_bind_int(sq3_statement, 66, sac.ievtyp); }
            if (sac.iqual == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 67); } else { sq3_result = sqlite3_bind_int(sq3_statement, 67, sac.iqual); }
            if (sac.istreg == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 68); } else { sq3_result = sqlite3_bind_int(sq3_statement, 68, sac.isynth); }
            if (sac.imagtyp == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 69); } else { sq3_result = sqlite3_bind_int(sq3_statement, 69, sac.imagtyp); }
            if (sac.imagsrc == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 70); } else { sq3_result = sqlite3_bind_int(sq3_statement, 70, sac.imagsrc); }
            if (sac.ibody == SAC::unset_int) { sq3_result = sqlite3_bind_null(sq3_statement, 71); } else { sq3_result = sqlite3_bind_int(sq3_statement, 71, sac.ibody); }
            // If not set, they're 0 anyway
            sq3_result = sqlite3_bind_int(sq3_statement, 72, sac.leven);
            sq3_result = sqlite3_bind_int(sq3_statement, 73, sac.lpspol);
            std::string trim_unset_word{SAC::unset_word};
            boost::algorithm::trim(trim_unset_word);
            boost::algorithm::trim(sac.kstnm);
            if (sac.kstnm == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 74); } else { sq3_result = sqlite3_bind_text(sq3_statement, 74, sac.kstnm.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kevnm);
            if (sac.kevnm == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 75); } else { sq3_result = sqlite3_bind_text(sq3_statement, 75, sac.kevnm.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.khole);
            if (sac.khole == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 76); } else { sq3_result = sqlite3_bind_text(sq3_statement, 76, sac.khole.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.ko);
            if (sac.ko == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 77); } else { sq3_result = sqlite3_bind_text(sq3_statement, 77, sac.ko.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.ka);
            if (sac.ka == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 78); } else { sq3_result = sqlite3_bind_text(sq3_statement, 78, sac.ka.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt0);
            if (sac.kt0 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 79); } else { sq3_result = sqlite3_bind_text(sq3_statement, 79, sac.kt0.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt1);
            if (sac.kt1 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 80); } else { sq3_result = sqlite3_bind_text(sq3_statement, 80, sac.kt1.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt2);
            if (sac.kt2 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 81); } else { sq3_result = sqlite3_bind_text(sq3_statement, 81, sac.kt2.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt3);
            if (sac.kt3 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 82); } else { sq3_result = sqlite3_bind_text(sq3_statement, 82, sac.kt3.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt4);
            if (sac.kt4 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 83); } else { sq3_result = sqlite3_bind_text(sq3_statement, 83, sac.kt4.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt5);
            if (sac.kt5 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 84); } else { sq3_result = sqlite3_bind_text(sq3_statement, 84, sac.kt5.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt6);
            if (sac.kt6 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 85); } else { sq3_result = sqlite3_bind_text(sq3_statement, 85, sac.kt6.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt7);
            if (sac.kt7 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 86); } else { sq3_result = sqlite3_bind_text(sq3_statement, 86, sac.kt7.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt8);
            if (sac.kt8 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 87); } else { sq3_result = sqlite3_bind_text(sq3_statement, 87, sac.kt8.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kt9);
            if (sac.kt9 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 88); } else { sq3_result = sqlite3_bind_text(sq3_statement, 88, sac.kt9.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kf);
            if (sac.kf == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 89); } else { sq3_result = sqlite3_bind_text(sq3_statement, 89, sac.kf.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kuser0);
            if (sac.kuser0 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 90); } else { sq3_result = sqlite3_bind_text(sq3_statement, 90, sac.kuser0.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kuser1);
            if (sac.kuser1 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 91); } else { sq3_result = sqlite3_bind_text(sq3_statement, 91, sac.kuser1.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kuser2);
            if (sac.kuser2 == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 92); } else { sq3_result = sqlite3_bind_text(sq3_statement, 92, sac.kuser2.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kcmpnm);
            if (sac.kcmpnm == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 93); } else { sq3_result = sqlite3_bind_text(sq3_statement, 93, sac.kcmpnm.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.knetwk);
            if (sac.knetwk == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 94); } else { sq3_result = sqlite3_bind_text(sq3_statement, 94, sac.knetwk.c_str(), -1, SQLITE_STATIC); }
            boost::algorithm::trim(sac.kinst);
            if (sac.kinst == trim_unset_word) { sq3_result = sqlite3_bind_null(sq3_statement, 95); } else { sq3_result = sqlite3_bind_text(sq3_statement, 95, sac.kinst.c_str(), -1, SQLITE_STATIC); }
            sq3_result = sqlite3_bind_int(sq3_statement, 96, checkpoint_id_);
            sq3_result = sqlite3_bind_blob(sq3_statement, 97, sac.data1.data(), sac.data1.size() * sizeof(double), SQLITE_STATIC);
            // This auto does null if empty
            sq3_result = sqlite3_bind_blob(sq3_statement, 98, sac.data2.data(), sac.data2.size() * sizeof(double), SQLITE_STATIC);
            // Execute the statement
            sq3_result = sqlite3_step(sq3_statement);
            // Finalize the statement
            sqlite3_finalize(sq3_statement);
        }
        //----------------------------------------------------------------
        // End add data checkpoint
        //----------------------------------------------------------------

        //----------------------------------------------------------------
        // Add processing information
        //----------------------------------------------------------------
        // This gets called when:
        // Data gets added (ADD)
        // Data gets modified (processing)
        // A checkpoint gets made
        void add_data_processing(int data_id, std::string processing_comment)
        {
            std::ostringstream oss{};
            oss << "INSERT INTO processing_" << data_id << " (";
            oss << "checkpoint_id, "; // 1
            oss << "comment)"; // 2
            oss << " VALUES (?, ?);";
            std::string sq3_string{oss.str()};
            sqlite3_stmt* sq3_statement{};
            sq3_result = sqlite3_prepare_v2(sq3_connection_file, sq3_string.c_str(), -1, &sq3_statement, nullptr);
            sq3_result = sqlite3_bind_int(sq3_statement, 1, checkpoint_id_);
            sq3_result = sqlite3_bind_text(sq3_statement, 2, processing_comment.c_str(), -1, SQLITE_STATIC);
            // Execute the statement
            sq3_result = sqlite3_step(sq3_statement);
            // Finalize the statement
            sqlite3_finalize(sq3_statement);
        }
        //----------------------------------------------------------------
        // End addprocessing information
        //----------------------------------------------------------------
};
};

#endif
