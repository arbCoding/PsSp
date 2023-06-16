#ifndef PSSP_PROJECTS_HPP_20230610
#define PSSP_PROJECTS_HPP_20230610

//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
#include "pssp_data_trees.hpp"
#include "pssp_datetime.hpp"
// String comparisons in C++ suck, boost adds needed functionality!
#include <boost/algorithm/string.hpp>
#include <sac_io.hpp>
#include <sac_stream.hpp>
// SQLite3 official library
#include <sqlite3.h>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <ios>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <sstream>
#include <thread>
#include <unordered_map> 
//-----------------------------------------------------------------------------
// End Include statements
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ToDo
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// End ToDo
//-----------------------------------------------------------------------------

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

namespace pssp
{
class Project
{
    private:
        // Name of the project, only gets set once
        std::string_view name_{};
        // Path to the database file
        std::filesystem::path path_{};
        // SQLite3 BLOB (Binary Large OBject) to std::vector<double>
        std::vector<double> blob_to_vector_double(sqlite3_stmt* blob_statement, int column_index);
        // Create provenance table
        // This only gets called upon creating a NEW project
        // This is ONLY in the on-disk database file
        // Not stored in memory
        void create_provenance_table();
        // Create checkpoint list table
        // This only gets called upon creating a NEW project
        // This is ONLY in the on-disk database file
        // Not stored in memory
        void create_checkpoint_list_table();
        // Create checkpoint table for data_id
        // This only gets called upon a piece of data being added to the project
        // This is stored on disk
        void create_data_checkpoint_table(int data_id);
        // Data processing information table for data_id
        // This only gets called upon a piece of data being added to the project
        // This is stored on disk by default, no need to keep data in memory (it is already there!)
        void create_data_processing_table(sqlite3* connection, int data_id);
        // Convenience function to mirror the tables between the file and memory
        void create_data_processing_table(int data_id);
        // Create a table for temporary data table
        // This is always on disk (for use when a project is too big
        // to be held entirely in memory)
        void create_temporary_data();
        // Add data provenance to data provenance table
        // This gets called ONLY when data gets added to a project
        // You get the data_id back upon insertion
        int add_data_provenance(const std::string& source);
        // Project connection initializers (file)
        void connect_file();
        // Memory
        void connect_memory();
        // Both
        void connect();
        // Disconnect from a sqlite3 connection
        void disconnect(sqlite3* connection);
        // Disconnect from both the file and memory connections
        void disconnect();
        // Create fresh tables for a brand new project
        void fresh_tables();
    public:
        // Checkpoint id
        int checkpoint_id_{0};
        // Connection to the database (file)
        sqlite3* sq3_connection_file{};
        // Conection to the database (memory)
        sqlite3* sq3_connection_memory{};
        int sq3_result{};
        char* sq3_error_message{};
        // Checkpoint notes
        std::string checkpoint_notes{};
        std::string checkpoint_name{};
        std::string checkpoint_timestamp{};
        bool clear_name{false};
        bool clear_notes{false};
        bool copy_name{true};
        bool copy_notes{true};
        bool is_project{false};
        // Vector of data_id's
        std::vector<int> current_data_ids{};
        // Boolean to flag an update
        std::atomic<bool> updated{false};
        std::shared_mutex mutex{};
        // Empty constructor
        Project() {};
        // Parameterized constructor
        Project(std::string name, std::filesystem::path base_path);
        // Connect to an existing project database
        void connect_2_existing(std::filesystem::path full_path);
        // Setter to modify the project object
        void new_project(std::string name, std::filesystem::path base_path);
        // Unload project
        void unload_project();
        // Destructor
        ~Project();
        // Add binary sac data to project
        int add_sac(SAC::SacStream& sac, const std::string& source);
        // Add checkpoint to checkpoint list table
        void write_checkpoint(bool author, bool cull);
        // Checkpoint a piece of data
        // This gets called when data gets added to a project
        // OR when a checkpoint gets made
        // This goes on file when data is added, or when a checkpoint happens
        void add_data_checkpoint(SAC::SacStream& sac, int data_id, bool processing = false);
        // When data is added, add a comment to the database on file
        // When a processing step is taken, add a comment to the database on memory
        // If the user performs a checkpoint, we'll append this to the table on file and clear memory
        // If the user loads a checkpoint, we'll clear memory
        // If the user unloads a project, we'll clear memory
        void add_data_processing(sqlite3* connection, int data_id, std::string processing_comment);
        // Clear in-memory processing meta-data
        void clear_processing_memory(int data_id);
        // Append in-memory processing meta-data to appropriate on-disk tables
        void move_processing_mem_2_disk(int data_id);
        // Get all checkpoint-ids
        std::vector<int> get_checkpoint_ids();
        // Get most recent checkpoint-id
        int get_latest_checkpoint_id();
        // Get all the data_ids
        std::vector<int> get_data_ids();
        // Get data_ids for a specific checkpoint
        std::vector<int> get_data_ids_for_checkpoint(int checkpoint_id);
        // Get data_ids for the current checkpoint
        std::vector<int> get_data_ids_for_current_checkpoint();
        // Given a checkpoint_id and data_id, build a SacStream object from its representation in the project database
        SAC::SacStream load_sacstream_from_checkpoint(int data_id, int checkpoint_id);
        // Get a sacstream for the current checkpoint_id
        SAC::SacStream load_sacstream(int data_id);
        // Specifically load from the temporary_data table
        SAC::SacStream load_sacstream_from_temporary(const int data_id, const int checkpoint_id);
        // Get a sacstream from the temporary_data table if it exists
        // if not, get it from the checkpoint table
        SAC::SacStream load_temporary_sacstream(const int data_id, const int checkpoint_id, const bool from_checkpoint = false);
        // Get source name from provenance table
        std::string get_source(int data_id);
        // Checkpoint_id setter
        void set_checkpoint_id(int checkpoint_id);
        // Path getter
        std::filesystem::path get_path();
        // Delete checkpoint from list
        void delete_checkpoint_from_list(int checkpoint_id);
        // Delete checkpoint_id data for data_id
        void delete_data_id_checkpoint(int data_id, int checkpoint_id);
        // Issue VACUUM command to database
        void vacuum();
        // Get meta-data for checkpoint (name, comments, creation timestamp)
        std::unordered_map<std::string, std::string> get_checkpoint_metadata(int checkpoint_id);
        // Get meta-data for current checkpoint
        std::unordered_map<std::string, std::string> get_current_checkpoint_metadata();
        // Checkpoint checkpoint lineage (history)
        std::vector<int> get_checkpoint_lineage(int checkpoint_id);
        // Given checkpoint_id and data_id, give me the processing history
        // including the processing that has yet to be checkpointed (in-memory)
        std::string get_processing_history(int data_id, int checkpoint_id);
        // Get the history for data_id and the current checkpoint_id
        std::string get_current_processing_history(int data_id);
        // Clear the temporary data table
        void clear_temporary_data();
        // Store the information in the temporary table
        void store_in_temporary_data(SAC::SacStream& sac, int data_id);
};
}

#endif
