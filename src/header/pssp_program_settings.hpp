#ifndef PSSP_PROGRAM_SETTINGS_HPP_20230610
#define PSSP_PROGRAM_SETTINGS_HPP_20230610

//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
// Dear ImGui header
#include <imgui.h>
// MessagePack, https://msgpack.org/
// Linux and MacOS
//#include <msgpack.hpp>
// Windows
//#include <msgpack.h>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
// Filesystem stuff
#include <filesystem>
// File IO
#include <fstream>
// std::cout
#include <iostream>
// std::string
#include <string>
//-----------------------------------------------------------------------------
// End Include statements
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Begin pssp namespace
//-----------------------------------------------------------------------------
namespace pssp
{
//-----------------------------------------------------------------------------
// Struct definitions
//-----------------------------------------------------------------------------
// These are per window settings
enum window_state { show, frozen, update, hide };
struct WindowSettings
{
    // X position in pixels (left to right)
    int pos_x{0};
    // Y position in pixels (top to bottom)
    int pos_y{0};
    // Window width in pixels
    int width{100};
    // Window height in pixels
    int height{100}; 
    // Flag that determines whether the window should be shown or not
    bool show{false};
    // if false, position and size get set
    bool is_set{false};
    // Window flags for Dear ImGui
    ImGuiWindowFlags img_flags{};
    // Window title, '##' is used to prevent Dear ImGui crashes
    std::string title{"##"};
    // State enum for the window (default stable = don't update information)
    window_state state{hide};
    // To allow msgpack to handle the components.
    //MSGPACK_DEFINE(pos_x, pos_y, width, height, show, is_set, img_flags, title)
};
// Settings for all windows (except main menu bar and status bar)
struct AllWindowSettings
{
    // Window with welcome message
    WindowSettings welcome{395, 340, 525, 60, false, false, ImGuiWindowFlags_NoCollapse
    | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar
    | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav, "Welcome##"};
    // FPS tracker window
    WindowSettings fps{1, 756, 60, 55, false, false, ImGuiWindowFlags_NoCollapse 
    | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar
    | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav, "FPS##"};
    // Header info for a single SAC file
    WindowSettings header{1, 25, 285, 730, false, false, ImGuiWindowFlags{}, "SAC Header##"};
    // Plot of a single sac file (time-series only)
    WindowSettings plot_1c{287, 25, 1150, 340, false, false, ImGuiWindowFlags{}, "Seismogram##"};
    // Plot real/imag spectrum of SAC file
    WindowSettings spectrum_1c{288, 368, 1150, 340, false, false, ImGuiWindowFlags{}, "Spectrum##"};
    // List of sac_1c's, allows user to select specific one in memory
    WindowSettings sac_files{287, 495, 375, 312, false, false, ImGuiWindowFlags{}, "SAC Files##"};
    // Small window providing access to lowpass filter options
    WindowSettings lowpass{508, 297, 231, 120, false, false, ImGuiWindowFlags_NoScrollbar 
    | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav, "Lowpass Options##"};
    // Small window providing access to highpass filter options
    WindowSettings highpass{508, 297, 231, 120, false, false, ImGuiWindowFlags_NoScrollbar 
    | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav, "Highpass Options##"};
    // Small window providing access to bandpass filter options
    WindowSettings bandpass{508, 297, 276, 148, false, false, ImGuiWindowFlags_NoScrollbar 
    | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav, "Bandpass Options##"};
    WindowSettings bandreject{508, 297, 276, 148, false, false, ImGuiWindowFlags_NoScrollbar
    | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav, "Bandreject Options##"};
    // File Dialog positions
    WindowSettings file_dialog{337, 150, 750, 450};
    // Name checkpoints
    WindowSettings name_checkpoint{508, 297, 276, 148, false, false, ImGuiWindowFlags_AlwaysAutoResize, "Checkpoint Name##"};
    // Notes for current checkpoint
    WindowSettings notes_checkpoint{508, 297, 276, 148, false, false, ImGuiWindowFlags_AlwaysAutoResize, "Checkpoint Notes##"};
    WindowSettings processing_history{665, 573, 773, 234, false, false, ImGuiWindowFlags{}, "History##"};
    // Data-pool options
    WindowSettings data_pool_options{508, 297, 231, 120, false, false, ImGuiWindowFlags_NoScrollbar 
    | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav, "DataPool Options##"};
    //MSGPACK_DEFINE(welcome, fps, header, plot_1c, spectrum_1c, sac_files, lowpass, highpass, bandpass, bandreject, file_dialog, name_checkpoint, notes_checkpoint, processing_history);
};
// Whether a certain menu is allow or not
struct MenuAllowed
{
    // File menu
    bool file_menu{true};
    bool open_1c{false};
    bool open_dir{false};
    bool save_1c{false};
    bool exit{true};
    // Project Menu
    bool project_menu{true};
    bool new_project{true};
    bool load_project{true};
    bool unload_project{true};
    // Checkpoint Menu
    bool checkpoint_menu{true};
    bool new_checkpoint{true};
    bool load_checkpoint{true};
    bool delete_checkpoint{true};
    // Options Menu
    bool options_menu{false};
    bool data_pool_options_menu{false};
    // Window Menu
    bool window_menu{true};
    bool center_windows{false};
    bool save_layout{false};
    bool reset_windows{true};
    bool welcome{true};
    bool fps{true};
    bool sac_header{false};
    bool plot_1c{false};
    bool plot_spectrum_1c{false};
    bool sac_deque{false};
    // Processing Menu
    bool processing_menu{false};
    bool rmean{false};
    bool rtrend{false};
    bool lowpass{false};
    bool highpass{false};
    bool bandpass{false};
    bool bandreject{false};
    // Picking Menu
    bool picking_menu{false};
    // Batch Menu
    bool batch_menu{false};
};
//-----------------------------------------------------------------------------
// End Struct definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Class definitions
//-----------------------------------------------------------------------------
//------------------------------------------------------------------------
// Project class
//------------------------------------------------------------------------
class ProgramSettings
{
    //--------------------------------------------------------------------
    // Project class description
    //--------------------------------------------------------------------
    // What is a "Project"?
    // 
    // A Project is composed of data and the results of processing that data
    //
    // For our puroses, the data is simple. People read in SAC files. These
    // SAC files are time-series (they can be other things, but for now I'm
    // focused on taking in raw time-series data at the start [assuming project
    // starts from freshly obtained seismographic time-series recordings]).
    //
    // A relational database would be quite convenient.
    //
    // This provides us our data provenance information
    // Data File Table
    // data_id, datetime-added, source, file_name
    //  data_id: unique data-id (integer) that is automatically determined
    //  source: the data source (original directory, IRIS webserv, whatever)
    //  file_name: the file_name
    //  datetime-added: we want to timestamp when data is first introduced to
    //      analysis (YYYY-MM-DD HH:mm:ss) <- I don't think we need to be
    //      more specific than that
    //
    // This provides our information on checkpoints
    // It also provides a mechanism for a user to prevent a specific checkpoint
    // from disappearing unless they manually delete it
    // Checkpoint Table
    // checkpoint_id, checkpoint_name, datetime-added, n_files, bool_auto, bool_cull
    //  checkpoint_id: unique identifier
    //  checkpoint_name: name of the checkpoint
    //      default is auto
    //      user defined is default user, or their chosen name
    //  n_files: How many SAC files are in the checkpoint
    //  bool_auto: automatic or user-defined
    //  bool_cull: true if we're allowed to automatically cull it (true default for auto, false default for user)
    //
    // This provides our checkpoint specific information
    // Checkpoint Data Table
    // checkpoint_name, original_data_id, checkpoint_data_id
    //  checkpoint_name: the name of the checkpoint
    //  original_data_id: the data_id of the source file
    //  checkpoint_data_id: the data_id within the checkpoint (this allows us to uniquely define the modified data)
    //
    // So if a file gets deleted from a project, it gets deleted from all checkpoints
    // If a file gets deleted from a checkpoint, it remains in the project
    // This provides a mechanism by which the user can decide what to do with
    // the files that are in the project, but not the current checkpoint.
    // It also proides a way to see what the most recent checkpoint containing the file is called
    // and when it was made (to aid the decision).
    //
    // If a checkpoint gets deleted, every row in the data table related to the checkpoint
    // gets deleted and the row for that checkpoint in the checkpoint table gets deleted
    // (along with the checkpoint's directory and everything inside it)
    //--------------------------------------------------------------------
    // End project class description
    //--------------------------------------------------------------------

    //----------------------------------------------------------------------
    // Project private variables/methods
    //----------------------------------------------------------------------
    private:
    //----------------------------------------------------------------------
    // End project private variables/methods
    //----------------------------------------------------------------------

    //----------------------------------------------------------------------
    // Project public variables/methods
    //----------------------------------------------------------------------
    public:
        //---------------------------------------------------------------
        // Public variables
        //---------------------------------------------------------------
        AllWindowSettings window_settings{};
        MenuAllowed menu_allowed{};
        // Project base directory
        std::filesystem::path base_dir{};
        // Relative paths to project sub-directories
        // Raw data directory
        // Things can be added to this directory, or removed from it
        // Nothing is ever over-written
        std::filesystem::path raw_data_dir{"raw_data"};
        // This is the current working directory.
        // When we make a checkpoint, we save to the working_dir
        // and then copy it to the checkpoint_dir
        std::filesystem::path working_dir{"working_dir"};
        // This is where check-points get saved
        // Each check-point is a distinct directory
        // Checkpoints are named {custom}_{datetime}
        // {custom} allows the user to define a checkpoint name
        // A default one will be auto_{datetime}
        // We can have auto ones get developed on a timed-schedule
        // with a limit to the number that stay preserved
        std::filesystem::path checkpoints_dir{"check_points"};
        // Names of project specific files
        // Project meta-data file
        std::filesystem::path md_file{"project_md.msgpack"}; 
        //---------------------------------------------------------------
        // End Public variables
        //---------------------------------------------------------------

        //---------------------------------------------------------------
        // Public methods
        //---------------------------------------------------------------
        // Tell me where you want the project to go.
        // If the directory exists we refuse (return an exception to tell
        // the user the it exists and needs to be deleted)
        // If it doesn't exist, we make it, a raw_data_dir
        // and a checkpoints dir. We populate a project object
        // and do a first time write-out to project_mod.msgpack
        // We should track the datetime of the project creation
        // As well as the datetime of the last modification to the project
        // Which is dependent upon the currently active checkpoint or the
        // most recent save/addition of new data
        void create_new_project(std::filesystem::path new_base_dir);
        // Give me a project file and I'll load in the project information
        // As well as all the project data
        void load_project(std::filesystem::path md_full_path);
        // Will save the current project state to the given path
        // If the save_path exists, delete and recreate it (avoid any
        // project mixing).
        // It will write out all the sac_1c files
        void save_project(std::filesystem::path save_path);
        // Unload the currently project so that we can load or create a new
        // project fresh
        void clear_project();
        //---------------------------------------------------------------
        // End public methods
        //---------------------------------------------------------------

    //----------------------------------------------------------------------
    // End Project Public variables/methods
    //----------------------------------------------------------------------
};
//------------------------------------------------------------------------
// End Project class
//------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// End Class definitions
//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// End pssp namespace
//-----------------------------------------------------------------------------
#endif
