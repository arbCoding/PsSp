#ifndef PSSP_PROJECTS_HPP
#define PSSP_PROJECTS_HPP

#include "sac_stream.hpp"
#include <ios>
#include <sqlite3.h>
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
//
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

/*
//-------------------------------------------------------------------------
// SQLite3 startup
//-------------------------------------------------------------------------
// Make a connection
sqlite3* sq3_connection{};
int sq3_result{sqlite3_open_v2("./test.db", &sq3_connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
| SQLITE_OPEN_FULLMUTEX, nullptr)};
if (sq3_result != SQLITE_OK) { return sq3_result; }
// Create a single table
std::string create_table_sql = R"(
    CREATE TABLE original_data (
        id INTEGER PRIMARY KEY,
        data TEXT,
        added DATETIME DEFAULT CURRENT_TIMESTAMP
    );
)";
char* sq3_error_message{};
sq3_result = sqlite3_exec(sq3_connection, create_table_sql.c_str(), nullptr, nullptr, &sq3_error_message);
// Failed to create table, close connection
if (sq3_result != SQLITE_OK) { sqlite3_close_v2(sq3_connection); return sq3_result; }
std::string insert_sql{"INSERT INTO original_data (data) VALUES ('Some data')"};
sq3_result = sqlite3_exec(sq3_connection, insert_sql.c_str(), 0, 0, 0);
sqlite3_close_v2(sq3_connection);
//-------------------------------------------------------------------------
// End SQLite3 startup
//-------------------------------------------------------------------------
*/

namespace pssp
{
class Project
{
    private:
        // Name of the project, only gets set once
        std::string_view name_{};
        // Path to the database file
        std::filesystem::path path_{};
        //---
        // Left-pad integers
        //---
        std::string left_pad_integers(int n, int width)
        {
            std::ostringstream oss{};
            oss << std::setw(width) << std::setfill('0') << n;
            return oss.str();
        }
        //---
        // End Left-pad integers
        //---

        //---
        // Make datetime
        //---
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
            return oss.str();
        }
        //---
        // End Make datetime
        //---
    public:
        // Connection to the database
        sqlite3* sq3_connection{};
        int sq3_result{};
        char* sq3_error_message{};
        //---
        // Parameterized Constructor
        //---
        Project(std::string name, std::filesystem::path base_path) : name_{name}, path_{base_path / (name + ".db")}
        {
            // Create a new connection
            sq3_result = sqlite3_open_v2(path_.c_str(), &sq3_connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
            // Set the journal mode to WAL
            sq3_result = sqlite3_exec(sq3_connection, "PRAGMA journal_mode=WAL", nullptr, nullptr, &sq3_error_message);
            // Create a base data table
            std::string_view sq3_create = R"(
                CREATE TABLE base_data (
                    base_id INTEGER PRIMARY KEY,
                    source TEXT,
                    added DATETIME DEFAULT CURRENT_TIMESTAMP,
                    sample_rate REAL,
                    begin_time REAL,
                    end_time REAL,
                    origin_time REAL,
                    a_time REAL,
                    t0_time REAL,
                    t1_time REAL,
                    t2_time REAL,
                    t3_time REAL,
                    t4_time REAL,
                    t5_time REAL,
                    t6_time REAL,
                    t7_time REAL,
                    t8_time REAL,
                    t9_time REAL,
                    fini_time REAL,
                    stla REAL,
                    stlo REAL,
                    stel REAL,
                    stdp REAL,
                    evla REAL,
                    evlo REAL,
                    evel REAL,
                    evdp REAL,
                    mag REAL,
                    dist REAL,
                    az REAL,
                    baz REAL,
                    gcarc REAL,
                    cmpaz REAL,
                    cmpinc REAL,
                    reference_time DATETIME,
                    npts INTEGER,
                    origin_id INTEGER,
                    event_id INTEGER,
                    mag_type INTEGER,
                    station TEXT,
                    event TEXT,
                    origin_text TEXT,
                    a_text TEXT,
                    t0_text TEXT,
                    t1_text TEXT,
                    t2_text TEXT,
                    t3_text TEXT,
                    t4_text TEXT,
                    t5_text TEXT,
                    t6_text TEXT,
                    t7_text TEXT,
                    t8_text TEXT,
                    t9_text TEXT,
                    fini_text TEXT,
                    component TEXT,
                    network TEXT,
                    instrument TEXT,
                    data BLOB
                );
            )";
            sq3_result = sqlite3_exec(sq3_connection, sq3_create.data(), nullptr, nullptr, &sq3_error_message);
        }
        //---
        // End Parameterized Constructor
        //---

        //---
        // Destructor
        //---
        ~Project() { sqlite3_close_v2(sq3_connection); }
        //---
        // End Destructor
        //---

        //---
        // Add base data
        //---
        // Because we store the data vector as a double instead ofa float, the data-size is roughly double
        // but we no longer lose precision upon saving
        void add_base_data(SAC::SacStream& sac, std::string file_path)
        {
            std::string_view sq3_add_base = "INSERT INTO base_data (sample_rate, begin_time, end_time, origin_time, a_time, t0_time, t1_time, t2_time, t3_time, t4_time, t5_time, t6_time, t7_time, t8_time, t9_time, fini_time, stla, stlo, stel, stdp, evla, evlo, evel, evdp, mag, dist, az, baz, gcarc, cmpaz, cmpinc, npts, origin_id, event_id, mag_type, station, event, origin_text, a_text, t0_text, t1_text, t2_text, t3_text, t4_text, t5_text, t6_text, t7_text, t8_text, t9_text, fini_text, component, network, instrument, data, reference_time, source) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
            sqlite3_stmt* sq3_statement{};
            sq3_result = sqlite3_prepare_v2(sq3_connection, sq3_add_base.data(), -1, &sq3_statement, nullptr);
            //std::cout << "Prepared sq3_statement: " << sq3_result << '\n';
            // Now to bind each object separately
            sq3_result = sqlite3_bind_double(sq3_statement, 1, sac.delta);
            sq3_result = sqlite3_bind_double(sq3_statement, 2, sac.b);
            sq3_result = sqlite3_bind_double(sq3_statement, 3, sac.e);
            sq3_result = sqlite3_bind_double(sq3_statement, 4, sac.o);
            sq3_result = sqlite3_bind_double(sq3_statement, 5, sac.a);
            sq3_result = sqlite3_bind_double(sq3_statement, 6, sac.t0);
            sq3_result = sqlite3_bind_double(sq3_statement, 7, sac.t1);
            sq3_result = sqlite3_bind_double(sq3_statement, 8, sac.t2);
            sq3_result = sqlite3_bind_double(sq3_statement, 9, sac.t3);
            sq3_result = sqlite3_bind_double(sq3_statement, 10, sac.t4);
            sq3_result = sqlite3_bind_double(sq3_statement, 11, sac.t5);
            sq3_result = sqlite3_bind_double(sq3_statement, 12, sac.t6);
            sq3_result = sqlite3_bind_double(sq3_statement, 13, sac.t7);
            sq3_result = sqlite3_bind_double(sq3_statement, 14, sac.t8);
            sq3_result = sqlite3_bind_double(sq3_statement, 15, sac.t9);
            sq3_result = sqlite3_bind_double(sq3_statement, 16, sac.f);
            sq3_result = sqlite3_bind_double(sq3_statement, 17, sac.stla);
            sq3_result = sqlite3_bind_double(sq3_statement, 18, sac.stlo);
            sq3_result = sqlite3_bind_double(sq3_statement, 19, sac.stel);
            sq3_result = sqlite3_bind_double(sq3_statement, 20, sac.stdp);
            sq3_result = sqlite3_bind_double(sq3_statement, 21, sac.evla);
            sq3_result = sqlite3_bind_double(sq3_statement, 22, sac.evlo);
            sq3_result = sqlite3_bind_double(sq3_statement, 23, sac.evel);
            sq3_result = sqlite3_bind_double(sq3_statement, 24, sac.evdp);
            sq3_result = sqlite3_bind_double(sq3_statement, 25, sac.mag);
            sq3_result = sqlite3_bind_double(sq3_statement, 26, sac.dist);
            sq3_result = sqlite3_bind_double(sq3_statement, 27, sac.az);
            sq3_result = sqlite3_bind_double(sq3_statement, 28, sac.baz);
            sq3_result = sqlite3_bind_double(sq3_statement, 29, sac.gcarc);
            sq3_result = sqlite3_bind_double(sq3_statement, 30, sac.cmpaz);
            sq3_result = sqlite3_bind_double(sq3_statement, 31, sac.cmpinc);
            sq3_result = sqlite3_bind_int(sq3_statement, 32, sac.npts);
            sq3_result = sqlite3_bind_int(sq3_statement, 33, sac.norid);
            sq3_result = sqlite3_bind_int(sq3_statement, 34, sac.nevid);
            sq3_result = sqlite3_bind_int(sq3_statement, 35, sac.imagtyp);
            sq3_result = sqlite3_bind_text(sq3_statement, 36, sac.kstnm.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 37, sac.kevnm.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 38, sac.ko.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 39, sac.ka.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 40, sac.kt0.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 41, sac.kt1.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 42, sac.kt2.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 43, sac.kt3.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 44, sac.kt4.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 45, sac.kt5.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 46, sac.kt6.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 47, sac.kt7.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 48, sac.kt8.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 49, sac.kt9.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 50, sac.kf.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 51, sac.kcmpnm.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 52, sac.knetwk.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 53, sac.kinst.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_blob(sq3_statement, 54, sac.data1.data(), sac.data1.size() * sizeof(double), SQLITE_STATIC);
            std::string ref_time{sac_reference_time(sac)};
            sq3_result = sqlite3_bind_text(sq3_statement, 55, ref_time.c_str(), -1, SQLITE_STATIC);
            sq3_result = sqlite3_bind_text(sq3_statement, 56, file_path.c_str(), -1, SQLITE_STATIC);
            // Execute the statement
            sq3_result = sqlite3_step(sq3_statement);
            //std::cout << "Adding sac-file to database: " << sq3_result << '\n';
            // Finalize the statement
            sqlite3_finalize(sq3_statement);
        }
        //---
        // End Add base data
        //---

};
};

#endif
