//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
// This is used to serialize/deserialize program settings
// Previously this was called pssp_projects.hpp
// But that was inappropriate as the projects are much bigger
// than what MessagePack can easily handle (settings will be handled like this though
// so I'm keeping it around as is for now, will need to modify when projects are finished
// being implemented!)
// Definitions of Misc Structs/Classes/Functions
#include "pssp_misc.hpp"
#include "pssp_program_settings.hpp"
// Uses sqlite3 for projects
#include "pssp_projects.hpp"
// Where we will have all our window functions defined
#include "pssp_windows.hpp"
// SQLite3 library
#include <sqlite3.h>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
// std::clamp
#include <algorithm>
// std::getenv()
#include <cstdlib>
// path stuff
#include <filesystem>
// std::ref, needed to pass by reference to a thread (can't use & to pass by
// reference in this situation)
#include <functional>
#include <shared_mutex>
// String-stream for mixing types
#include <sstream>
// string_view, only used for the welcome window at the moment...
#include <string_view>
//-----------------------------------------------------------------------------
// End include statements
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Current Focus
//-----------------------------------------------------------------------------
// Overarching focus: Testing and Stability
//=============================================================================
// Main focus 1: Stability
// This cannot be a buggy mess that is randomly crashing if the user manages to
// select menu options at just the right time. Data races need to be prevented.
// Part of this is in `Quick focus 2` below, but this is a bigger goal than
// just speed. To be most useful, stability must triumph over speed.
//=============================================================================
// Main focus 2: Testing
// I think we would really benefit from introducing unit testing
// into the mix, to make this easier in the future (it is hard to remember
// to test everything after every bug-fix).
// It looks like Catch2 would be a nice light testing framework.
//
// https://github.com/catchorg/Catch2
// https://stackoverflow.com/questions/52273110/how-do-i-write-a-unit-test-in-c
//
// It should be easy to integrate and start using. Of course there will be a ramp
// up time of introducing it into the mix that will slow developement of new
// features. But it will make life infinitely easier in the future as this
// grows.
//
//=============================================================================
// Quick focus 1: std::vector<int> => std::set<int>
// Why? Set is automatically sorted and unique, with insert O(logn) and find O(logn)
//
// Vector is not sorted nor unique by default (additional overhead on my end).
// insert/remove on end is O(1), internal insert/remove is O(n). Find is O(n)
// plus the additional work of sorting them.
//
// I tend to use std::vector<int> for data_ids, but data_ids must be unique
// and having them sorted makes finding fast
//
// That means std::set<int> would be better in this scenario. Especially for
// large datasets.
//-----------------------------------------------------------------------------
// Quick focus 2: Revised locking scheme.
// Currently, to prevent data races I have a very tight locking-scheme.
// Unfortunately, this is slowing things down quite dramatically.
//
// I need to think of a better way, ideally I want locks to be held for the minimum
// amount of time possible, I also need to avoid "lock-inversion" to help minimize
// the chance of deadlock.
// Obviously being slow is better than getting deadlocked, but it was quite stable
// prior to fixing the data races found by the thread sanitizer. It was a lot faster
// too. That leads me to believe that the locks are being given too much time (out of
// poor design on my part).
//-----------------------------------------------------------------------------
// End Current Focus
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Known Bugs
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
// End Known Bugs
//-----------------------------------------------------------------------------

namespace pssp
{
//-----------------------------------------------------------------------------
// SQLite3 error-logging
//-----------------------------------------------------------------------------
// Suppress the error/warning logs from cout and instead direct them to a file
void sqliteLogCallback(unsigned char* data, int errCode, const char* message)
{
    // Cast the data pointer to an ofstream object
    std::ofstream& logFile{*std::bit_cast<std::ofstream*>(data)};

    // Write the SQLite warning message to the log file
    logFile << "SQLite Warning (" << errCode << "): " << message << std::endl;
}
//-----------------------------------------------------------------------------
// End SQLite3 error-logging
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function to handle logic of program state
//-----------------------------------------------------------------------------
void handle_program_state(ProgramStatus& program_status, ProgramSettings& current_settings, std::vector<int>& data_ids, sac_1c& visual_sac)
{
    // If we're done with the task, we need to shift over to the idle state
    if (program_status.tasks_completed >= program_status.total_tasks)
    {
        program_status.state.store(program_state::idle);
        program_status.progress = 1.1f;
    }
    else
    {
        program_status.progress = static_cast<float>(program_status.tasks_completed) / static_cast<float>(program_status.total_tasks);
    }
    program_state current_state{program_status.state.load()};
    // The program state determines what we are allowed to do
    switch (current_state)
    {
        case program_state::in:
            program_status.status_message = "Reading data in...";
            // We're reading in files
            // So most windows are hidden
            current_settings.window_settings.welcome.state = show;
            current_settings.window_settings.fps.state = show;
            current_settings.window_settings.data_pool_options.state = hide;
            current_settings.window_settings.header.state = hide;
            current_settings.window_settings.plot_1c.state = hide;
            current_settings.window_settings.spectrum_1c.state = hide;
            current_settings.window_settings.sac_files.state = hide;
            current_settings.window_settings.lowpass.state = hide;
            current_settings.window_settings.highpass.state = hide;
            current_settings.window_settings.bandpass.state = hide;
            current_settings.window_settings.bandreject.state = hide;
            current_settings.window_settings.file_dialog.state = hide;
            current_settings.window_settings.name_checkpoint.state = hide;
            current_settings.window_settings.notes_checkpoint.state = hide;
            current_settings.window_settings.processing_history.state = hide;
            // Most menus are also hidden
            current_settings.menu_allowed.file_menu = false;
            current_settings.menu_allowed.open_1c = false;
            current_settings.menu_allowed.open_dir = false;
            current_settings.menu_allowed.save_1c = false;
            current_settings.menu_allowed.exit = false;
            current_settings.menu_allowed.project_menu = false;
            current_settings.menu_allowed.new_project = false;
            current_settings.menu_allowed.load_project = false;
            current_settings.menu_allowed.unload_project = false;
            current_settings.menu_allowed.checkpoint_menu = false;
            current_settings.menu_allowed.new_checkpoint = false;
            current_settings.menu_allowed.load_checkpoint = false;
            current_settings.menu_allowed.delete_checkpoint = false;
            current_settings.menu_allowed.options_menu = true;
            current_settings.menu_allowed.data_pool_options_menu = false;
            current_settings.menu_allowed.window_menu = true;
            current_settings.menu_allowed.center_windows = true;
            current_settings.menu_allowed.save_layout = true;
            current_settings.menu_allowed.reset_windows = true;
            current_settings.menu_allowed.welcome = true;
            current_settings.menu_allowed.fps = true;
            current_settings.menu_allowed.sac_header = false;
            current_settings.menu_allowed.plot_1c = false;
            current_settings.menu_allowed.plot_spectrum_1c = false;
            current_settings.menu_allowed.sac_deque = false;
            current_settings.menu_allowed.processing_menu = false;
            current_settings.menu_allowed.rmean = false;
            current_settings.menu_allowed.rtrend = false;
            current_settings.menu_allowed.lowpass = false;
            current_settings.menu_allowed.highpass = false;
            current_settings.menu_allowed.bandpass = false;
            current_settings.menu_allowed.bandreject = false;
            current_settings.menu_allowed.picking_menu = false;
            current_settings.menu_allowed.batch_menu = false;
            break;
        case program_state::out:
            program_status.status_message = "Writing data out...";
            // We're writing out files
            // So some windows are hidden
            current_settings.window_settings.welcome.state = show;
            current_settings.window_settings.fps.state = show;
            current_settings.window_settings.data_pool_options.state = hide;
            current_settings.window_settings.header.state = show;
            current_settings.window_settings.plot_1c.state = show;
            current_settings.window_settings.spectrum_1c.state = show;
            current_settings.window_settings.sac_files.state = show;
            current_settings.window_settings.lowpass.state = hide;
            current_settings.window_settings.highpass.state = hide;
            current_settings.window_settings.bandpass.state = hide;
            current_settings.window_settings.bandreject.state = hide;
            current_settings.window_settings.file_dialog.state = hide;
            current_settings.window_settings.name_checkpoint.state = hide;
            current_settings.window_settings.notes_checkpoint.state = hide;
            current_settings.window_settings.processing_history.state = show;
            // Some menus are also hidden
            current_settings.menu_allowed.file_menu = false;
            current_settings.menu_allowed.open_1c = false;
            current_settings.menu_allowed.open_dir = false;
            current_settings.menu_allowed.save_1c = false;
            current_settings.menu_allowed.exit = false;
            current_settings.menu_allowed.project_menu = false;
            current_settings.menu_allowed.new_project = false;
            current_settings.menu_allowed.load_project = false;
            current_settings.menu_allowed.unload_project = false;
            current_settings.menu_allowed.checkpoint_menu = false;
            current_settings.menu_allowed.new_checkpoint = false;
            current_settings.menu_allowed.load_checkpoint = false;
            current_settings.menu_allowed.delete_checkpoint = false;
            current_settings.menu_allowed.options_menu = true;
            current_settings.menu_allowed.data_pool_options_menu = false;
            current_settings.menu_allowed.window_menu = true;
            current_settings.menu_allowed.center_windows = true;
            current_settings.menu_allowed.save_layout = true;
            current_settings.menu_allowed.reset_windows = true;
            current_settings.menu_allowed.welcome = true;
            current_settings.menu_allowed.fps = true;
            current_settings.menu_allowed.sac_header = true;
            current_settings.menu_allowed.plot_1c = true;
            current_settings.menu_allowed.plot_spectrum_1c = true;
            current_settings.menu_allowed.sac_deque = true;
            current_settings.menu_allowed.processing_menu = false;
            current_settings.menu_allowed.rmean = false;
            current_settings.menu_allowed.rtrend = false;
            current_settings.menu_allowed.lowpass = false;
            current_settings.menu_allowed.highpass = false;
            current_settings.menu_allowed.bandpass = false;
            current_settings.menu_allowed.bandreject = false;
            current_settings.menu_allowed.picking_menu = false;
            current_settings.menu_allowed.batch_menu = false;
            break;
        case program_state::processing:
            program_status.status_message = "Processing data...";
            // We're currently processing data
            // So most windows are hidden
            current_settings.window_settings.welcome.state = show;
            current_settings.window_settings.fps.state = show;
            current_settings.window_settings.data_pool_options.state = hide;
            current_settings.window_settings.header.state = hide;
            current_settings.window_settings.plot_1c.state = hide;
            current_settings.window_settings.spectrum_1c.state = hide;
            current_settings.window_settings.sac_files.state = hide;
            current_settings.window_settings.lowpass.state = hide;
            current_settings.window_settings.highpass.state = hide;
            current_settings.window_settings.bandpass.state = hide;
            current_settings.window_settings.bandreject.state = hide;
            current_settings.window_settings.file_dialog.state = hide;
            current_settings.window_settings.name_checkpoint.state = hide;
            current_settings.window_settings.notes_checkpoint.state = hide;
            current_settings.window_settings.processing_history.state = hide;
            // Most menus are also hidden
            current_settings.menu_allowed.file_menu = true;
            current_settings.menu_allowed.open_1c = false;
            current_settings.menu_allowed.open_dir = false;
            current_settings.menu_allowed.save_1c = false;
            current_settings.menu_allowed.exit = true;
            current_settings.menu_allowed.project_menu = false;
            current_settings.menu_allowed.new_project = false;
            current_settings.menu_allowed.load_project = false;
            current_settings.menu_allowed.unload_project = false;
            current_settings.menu_allowed.checkpoint_menu = false;
            current_settings.menu_allowed.new_checkpoint = false;
            current_settings.menu_allowed.load_checkpoint = false;
            current_settings.menu_allowed.delete_checkpoint = false;
            current_settings.menu_allowed.options_menu = true;
            current_settings.menu_allowed.data_pool_options_menu = false;
            current_settings.menu_allowed.window_menu = true;
            current_settings.menu_allowed.center_windows = true;
            current_settings.menu_allowed.save_layout = true;
            current_settings.menu_allowed.reset_windows = true;
            current_settings.menu_allowed.welcome = true;
            current_settings.menu_allowed.fps = true;
            current_settings.menu_allowed.sac_header = false;
            current_settings.menu_allowed.plot_1c = false;
            current_settings.menu_allowed.plot_spectrum_1c = false;
            current_settings.menu_allowed.sac_deque = false;
            current_settings.menu_allowed.processing_menu = false;
            current_settings.menu_allowed.rmean = false;
            current_settings.menu_allowed.rtrend = false;
            current_settings.menu_allowed.lowpass = false;
            current_settings.menu_allowed.highpass = false;
            current_settings.menu_allowed.bandpass = false;
            current_settings.menu_allowed.bandreject = false;
            current_settings.menu_allowed.picking_menu = false;
            current_settings.menu_allowed.batch_menu = false;
            break;
        case program_state::idle:
            // We're idle
            program_status.status_message = "Idle";
            // If the project has been updated then we need to copy the data_ids
            if (program_status.project.updated)
            {
                std::shared_lock lock_project(program_status.project.mutex);
                data_ids = program_status.project.current_data_ids;
                // Sort the data_ids
                std::ranges::sort(data_ids);
                // Flag the update has been dealt with
                program_status.project.updated = false;
                // Try to get a deep copy of the original object
                std::shared_ptr<sac_1c> sac_ptr{program_status.data_pool.get_ptr(program_status.project, program_status.data_id, program_status.project.checkpoint_id_)};
                if (sac_ptr)
                {
                    std::scoped_lock lock_sac{sac_ptr->mutex_};
                    visual_sac = *sac_ptr;
                }
            }
            // Empty the FFTW plan_pool, no need to keep it pre-filled.
            if (program_status.fftw_planpool.n_plans() != 0) { program_status.fftw_planpool.empty_pool(); }
            if (program_status.data_pool.n_data() > program_status.data_pool.max_data)
            { program_status.thread_pool.enqueue(reduce_data_pool, std::ref(program_status)); }
            // Any window can be shown
            current_settings.window_settings.welcome.state = show;
            current_settings.window_settings.fps.state = show;
            current_settings.window_settings.data_pool_options.state = show;
            if (!data_ids.empty())
            {
                // Windows
                current_settings.window_settings.header.state = show;
                current_settings.window_settings.plot_1c.state = show;
                current_settings.window_settings.spectrum_1c.state = show;
                current_settings.window_settings.sac_files.state = show;
                current_settings.window_settings.lowpass.state = show;
                current_settings.window_settings.highpass.state = show;
                current_settings.window_settings.bandpass.state = show;
                current_settings.window_settings.bandreject.state = show;
                current_settings.window_settings.processing_history.state = show;
                // Menus
                current_settings.menu_allowed.sac_header = true;
                current_settings.menu_allowed.plot_1c = true;
                current_settings.menu_allowed.plot_spectrum_1c = true;
                current_settings.menu_allowed.sac_deque = true;
                current_settings.menu_allowed.processing_menu = true;
                current_settings.menu_allowed.rmean = true;
                current_settings.menu_allowed.rtrend = true;
                current_settings.menu_allowed.lowpass = true;
                current_settings.menu_allowed.highpass = true;
                current_settings.menu_allowed.bandpass = true;
                current_settings.menu_allowed.bandreject = true;
                current_settings.menu_allowed.picking_menu = true;
                current_settings.menu_allowed.batch_menu = true;
            }
            else 
            {
                // Windows
                current_settings.window_settings.header.state = hide;
                current_settings.window_settings.plot_1c.state = hide;
                current_settings.window_settings.spectrum_1c.state = hide;
                current_settings.window_settings.sac_files.state = hide;
                current_settings.window_settings.lowpass.state = hide;
                current_settings.window_settings.highpass.state = hide;
                current_settings.window_settings.bandpass.state = hide;
                current_settings.window_settings.bandreject.state = hide;
                current_settings.window_settings.processing_history.state = hide;
                // Menus
                current_settings.menu_allowed.sac_header = false;
                current_settings.menu_allowed.plot_1c = false;
                current_settings.menu_allowed.plot_spectrum_1c = false;
                current_settings.menu_allowed.sac_deque = false;
                current_settings.menu_allowed.processing_menu = false;
                current_settings.menu_allowed.rmean = false;
                current_settings.menu_allowed.rtrend = false;
                current_settings.menu_allowed.lowpass = false;
                current_settings.menu_allowed.highpass = false;
                current_settings.menu_allowed.bandpass = false;
                current_settings.menu_allowed.bandreject = false;
                current_settings.menu_allowed.picking_menu = false;
                current_settings.menu_allowed.batch_menu = false;
            }
            current_settings.window_settings.file_dialog.state = show;
            // All menus are also shown
            current_settings.menu_allowed.file_menu = true;
            if (program_status.project.is_project)
            {
                current_settings.menu_allowed.open_1c = true;
                current_settings.menu_allowed.open_dir = true;
                if (!data_ids.empty()) { current_settings.menu_allowed.save_1c = true; } else { current_settings.menu_allowed.save_1c = false; }
                current_settings.menu_allowed.new_project = false;
                current_settings.menu_allowed.load_project = false;
                current_settings.menu_allowed.unload_project = true;
                current_settings.menu_allowed.checkpoint_menu = true;
                current_settings.menu_allowed.new_checkpoint = true;
                current_settings.menu_allowed.load_checkpoint = true;
                current_settings.menu_allowed.delete_checkpoint = true;
                current_settings.window_settings.name_checkpoint.state = show;
                current_settings.window_settings.notes_checkpoint.state = show;
            }
            else 
            {
                // If we're not in a project and we're idle, empty the pool!
                if (program_status.data_pool.n_data() > 0) { program_status.data_pool.empty_pool(); }
                current_settings.menu_allowed.open_1c = false;
                current_settings.menu_allowed.open_dir = false;
                current_settings.menu_allowed.save_1c = false;
                current_settings.menu_allowed.new_project = true;
                current_settings.menu_allowed.load_project = true;
                current_settings.menu_allowed.unload_project = false;
                current_settings.menu_allowed.checkpoint_menu = false;
                current_settings.menu_allowed.new_checkpoint = false;
                current_settings.menu_allowed.load_checkpoint = false;
                current_settings.menu_allowed.delete_checkpoint = false;
                current_settings.window_settings.name_checkpoint.state = hide;
                current_settings.window_settings.notes_checkpoint.state = hide;
            }
            current_settings.menu_allowed.exit = true;
            current_settings.menu_allowed.project_menu = true;
            current_settings.menu_allowed.options_menu = true;
            current_settings.menu_allowed.data_pool_options_menu = true;
            current_settings.menu_allowed.window_menu = true;
            current_settings.menu_allowed.center_windows = true;
            current_settings.menu_allowed.save_layout = true;
            current_settings.menu_allowed.reset_windows = true;
            current_settings.menu_allowed.welcome = true;
            current_settings.menu_allowed.fps = true;
            break;
        default:
            // We're in an unknown state
            program_status.state.store(program_state::idle);
            break;
    }
}
//-----------------------------------------------------------------------------
// End Function to handle logic of program state
//-----------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
// Main has high cyclomatic complexity, should be refactored.
int main(int arg_count, char* arg_array[])
{
    // Prevent unused variable warning for arg_count, it is required if I want
    // arg_array, but I don't actually need it for anything else
    (void) arg_count;
    // Location of the program
    std::filesystem::path program_path(arg_array[0]);
    // Full path
    program_path = std::filesystem::canonical(program_path);
    // We're assuming that the font is in the same directory as the program
    program_path = program_path.parent_path();
    // Open the log file
    std::ofstream sq3_log_file{program_path / "sqlite.log"};
    // Configure SQLite to redirect warning messages to the log file
    sqlite3_config(SQLITE_CONFIG_LOG, pssp::sqliteLogCallback, &sq3_log_file);
    //---------------------------------------------------------------------------
    // Initialization
    //---------------------------------------------------------------------------
    // Check to make sure GLFW can startup
    glfwSetErrorCallback(pssp::glfw_error_callback);
    if (!glfwInit()) { std::abort(); }
    // Setup the graphics library and get the version
    const char* glsl_version = pssp::setup_gl();
    // Setup the GLFW window
    GLFWwindow* window = glfwCreateWindow(1024, 720, "Passive-source Seismic-processing", nullptr, nullptr);
    // Start the graphics backends and create the ImGui and ImPlot contexts
    ImGuiIO const& io = pssp::start_graphics(window, glsl_version, program_path);
    //---------------------------------------------------------------------------
    // End Initialization
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Misc Draw loop variables
    //---------------------------------------------------------------------------
    // Default color for clearing the screen
    auto clear_color = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
    pssp::ProgramSettings current_settings{};
    // pssp_program_settings.cpp does not work in Windows.
    // Honestly I may dump it as it doesn't have much use anymore
    // Testing making a new proto-project
    //current_settings.create_new_project(std::filesystem::current_path());
    // Testing loading a new proto-project
    //current_settings.load_project(std::filesystem::current_path() / current_settings.md_file);
    pssp::fps_info fps_tracker{};
    std::string_view welcome_message{"Welcome to Passive-source Seismic-processing (PsSP)!"};
    pssp::AllFilterOptions af_settings{};
    pssp::ProgramStatus program_status{};
    // Make sure we meet the minimum requied amount of data in the data-pool to function without
    // deadlocking! This logic will belong elsewhere as well, if I allow the user to manually
    // alter the size of the data-pool (cannot go below the number of threads in the threadpool!)
    if (program_status.data_pool.max_data < program_status.thread_pool.n_threads_total())
    {
        program_status.data_pool.max_data = program_status.thread_pool.n_threads_total();
    }
    // Spectrum (only 1 for now)
    pssp::sac_1c spectrum{};
    // Will pass this by reference for the sake of plotting and what-not
    pssp::sac_1c time_series{};
    // This is for calculating the spectrum which then gets down-sampled
    pssp::sac_1c spectrum_time_series{};
    // Which sac-file is active
    int active_sac{};
    bool clear_sac{false};
    bool update_spectrum{false};
    // The data_ids will be used to replace the sac_deque
    // we can pass the data_ids to different functions and use that to
    // access the data from the data pool (part of program_status)
    std::vector<int> data_ids{};
    //---------------------------------------------------------------------------
    // End Misc Draw loop variables
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Draw loop
    //---------------------------------------------------------------------------
    // The draw loop is ran EVERY FRAME, so be careful to make the stuff in here safe
    // (minimize spurious work, be thread-safe, etc.)
    while (!glfwWindowShouldClose(window))
    {
        // Each frame, we need to check the program's state
        // to determine what we are and are not allowed to do
        handle_program_state(program_status, current_settings, data_ids, time_series);
        // Do we need to remove a sac_1c from the sac_deque?
        //cleanup_sac(program_status.project, sac_deque, active_sac, clear_sac);
        // Start the frame
        pssp::prep_newframe();
        status_bar(program_status);
        main_menu_bar(window, current_settings.window_settings, current_settings.menu_allowed, af_settings, program_status, active_sac);
        // Show the Welcome window if appropriate
        window_welcome(current_settings.window_settings.welcome, welcome_message);
        update_fps(fps_tracker, io);
        // Show the FPS window if appropriate
        window_fps(fps_tracker, current_settings.window_settings.fps);
        // Show the DataPool Options window if appropriate
        window_data_pool_options(current_settings.window_settings.data_pool_options, program_status);
        // If we're idle and there is data to show, we can show it
        pssp::program_state current_state{program_status.state.load()};
        if (current_state == pssp::program_state::idle && !data_ids.empty())
        {
          if (active_sac < 0) { active_sac = 0; } else if (active_sac >= static_cast<int>(data_ids.size())) { active_sac = 0; }
          if (program_status.data_id != data_ids[active_sac])
          {
            program_status.data_id = data_ids[active_sac];
            update_spectrum = true;
            std::shared_ptr<pssp::sac_1c> tmp_ptr = program_status.data_pool.get_ptr(program_status.project, program_status.data_id, program_status.project.checkpoint_id_);
            // Attempting to make two separate deep copies
            time_series = *tmp_ptr;
            spectrum_time_series = *tmp_ptr;
          }
          if (time_series.sac.npts > 1000) {  downsample_4_plotting(time_series); }
          window_sac_header(current_settings.window_settings.header, spectrum_time_series);
          // Show processing history window is appropriate
          window_processing_history(current_settings.window_settings.processing_history, program_status.project, data_ids[active_sac]);
          // Show the Sac Plot window if appropriate
          window_plot_sac(current_settings.window_settings.plot_1c, time_series);
          // Show Checkpoint naming window if appropriate
          window_name_checkpoint(current_settings.window_settings.name_checkpoint, program_status);
          // Show Checkpoint note window if appropriate
          window_notes_checkpoint(current_settings.window_settings.notes_checkpoint, program_status.project);
          // Show the Sac Spectrum window if appropriate
          // We need to see if the FFT needs to be calculated (don't want to do it
          // every frame)
          if (current_settings.window_settings.spectrum_1c.show && update_spectrum)
          {
            // If they're not the same, then calculate the FFT
            calc_spectrum(program_status, spectrum_time_series, spectrum);
            if (spectrum.sac.npts > 1000) { downsample_4_plotting(spectrum); spectrum.sac.delta = spectrum_time_series.sac.delta; }
            update_spectrum = false;
          }
          // Finally plot the spectrum
          window_plot_spectrum(current_settings.window_settings.spectrum_1c, program_status.project.is_project, spectrum);
          // Show a list of available data and allow the user to select the data they want to look at
          window_data_list(program_status, current_settings.window_settings, current_settings.menu_allowed, active_sac, clear_sac);
          window_lowpass_options(current_settings.window_settings.lowpass, af_settings.lowpass);
          window_highpass_options(current_settings.window_settings.highpass, af_settings.highpass);
          window_bandpass_options(current_settings.window_settings.bandpass, af_settings.bandpass);
        }
        else
        {
            // Disallow menu options that require sac files
            current_settings.menu_allowed.save_1c = false;
            current_settings.menu_allowed.sac_deque = false;
            current_settings.menu_allowed.sac_header = false;
            current_settings.menu_allowed.plot_1c = false;
            current_settings.menu_allowed.plot_spectrum_1c = false;
            current_settings.menu_allowed.processing_menu = false;
            current_settings.menu_allowed.lowpass = false;
            current_settings.menu_allowed.highpass = false;
            current_settings.menu_allowed.bandpass = false;
            current_settings.menu_allowed.rmean = false;
            current_settings.menu_allowed.rtrend = false;
            current_settings.menu_allowed.batch_menu = false;
            spectrum.file_name = "";
        }
        // Finish the frame
        pssp::finish_newframe(window, clear_color);
        //-------------------------------------------------------------------------
        // Queue Filtering tasks
        //-------------------------------------------------------------------------
        // Queue up filters if required
        if (af_settings.lowpass.apply_filter)
        {
            if (af_settings.lowpass.apply_batch)
            {
                program_status.thread_pool.enqueue(pssp::batch_apply_lowpass, std::ref(program_status), std::ref(af_settings.lowpass));
                af_settings.lowpass.apply_batch = false;
            }
            else
            {
                program_status.tasks_completed = 0;
                program_status.total_tasks = 1;
                program_status.thread_pool.enqueue(pssp::apply_lowpass, std::ref(program_status), std::ref(data_ids[active_sac]), std::ref(af_settings.lowpass));
            }
            af_settings.lowpass.apply_filter = false;
        }
        else if (af_settings.highpass.apply_filter)
        {
            if (af_settings.highpass.apply_batch)
            {
                program_status.thread_pool.enqueue(pssp::batch_apply_highpass, std::ref(program_status), std::ref(af_settings.highpass));
                af_settings.highpass.apply_batch = false;
            }
            else
            {
                program_status.tasks_completed = 0;
                program_status.total_tasks = 1;
                program_status.thread_pool.enqueue(pssp::apply_highpass, std::ref(program_status), std::ref(data_ids[active_sac]), std::ref(af_settings.highpass));
            }
            af_settings.highpass.apply_filter = false;
        }
        else if (af_settings.bandpass.apply_filter)
        {
            if (af_settings.bandpass.apply_batch)
            {
                program_status.thread_pool.enqueue(pssp::batch_apply_bandpass, std::ref(program_status), std::ref(af_settings.bandpass));
                af_settings.bandpass.apply_batch = false;
            }
            else
            {
                program_status.tasks_completed = 0;
                program_status.total_tasks = 1;
                program_status.thread_pool.enqueue(pssp::apply_bandpass, std::ref(program_status), std::ref(data_ids[active_sac]), std::ref(af_settings.bandpass));
            }
            af_settings.bandpass.apply_filter = false;
        }
        else if (af_settings.bandreject.apply_filter)
        {
            // Not yet implemented
        }
        //-------------------------------------------------------------------------
        // End Queue Filtering tasks
        //-------------------------------------------------------------------------
    }
    //---------------------------------------------------------------------------
    // End draw loop
    //---------------------------------------------------------------------------
    pssp::end_graphics(window);
    // Close the log file
    sq3_log_file.close();
    // End program
    return 0;
}
//-----------------------------------------------------------------------------
// End Main
//-----------------------------------------------------------------------------

