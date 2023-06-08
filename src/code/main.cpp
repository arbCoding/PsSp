//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
// This is used to serialize/deserialize program settings
// Previously this was called pssp_projects.hpp
// But that was inappropriate as the projects are much bigger
// than what MessagePack can easily handle (settings will be handled like this though
// so I'm keeping it around as is for now, will need to modify when projects are finished
// being implemented!)
#include "pssp_program_settings.hpp"
// Where we will have all our window functions defined
#include "pssp_windows.hpp"
// Definitions of Misc Structs/Classes/Functions
#include "pssp_misc.hpp"
// Uses sqlite3 for projects
#include "pssp_projects.hpp"
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
// std::string_view
#include <string_view>
// Path stuff
#include <filesystem>
// std::clamp
#include <algorithm>
// std::ref, needed to pass by reference to a thread (can't use & to pass by
// reference in this situation)
#include <functional>
// String-stream for mixing types
#include <sstream>
// std::getenv()
#include <cstdlib>
// SQLite3 library
#include <sqlite3.h>
//-----------------------------------------------------------------------------
// End include statements
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Current Focus
//-----------------------------------------------------------------------------
// Data display and migrating away from direct access to the sac_deque.
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
void sqliteLogCallback(void* data, int errCode, const char* message)
{
    // Cast the data pointer to an ofstream object
    std::ofstream& logFile{*reinterpret_cast<std::ofstream*>(data)};

    // Write the SQLite warning message to the log file
    logFile << "SQLite Warning (" << errCode << "): " << message << std::endl;
}
//-----------------------------------------------------------------------------
// End SQLite3 error-logging
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function to handle logic of program state
//-----------------------------------------------------------------------------
void handle_program_state(ProgramStatus& program_status, ProgramSettings& current_settings, Project& current_project, std::deque<sac_1c>& sac_deque)
{
    // If we're done with the task, we need to shift over to the idle state
    if (program_status.tasks_completed == program_status.total_tasks)
    {
        program_status.state.store(idle);
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
        case in:
            program_status.status_message = "Reading data in...";
            // We're reading in files
            // So most windows are hidden
            current_settings.window_settings.welcome.state = show;
            current_settings.window_settings.fps.state = show;
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
        case out:
            program_status.status_message = "Writing data out...";
            // We're writing out files
            // So some windows are hidden
            current_settings.window_settings.welcome.state = show;
            current_settings.window_settings.fps.state = show;
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
        case processing:
            program_status.status_message = "Processing data...";
            // We're currently processing data
            // So most windows are hidden
            current_settings.window_settings.welcome.state = show;
            current_settings.window_settings.fps.state = show;
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
        case idle:
            program_status.status_message = "Idle";
            // We're idle
            // Any window can be shown
            current_settings.window_settings.welcome.state = show;
            current_settings.window_settings.fps.state = show;
            if (sac_deque.size() > 0)
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
            if (current_project.is_project)
            {
                current_settings.menu_allowed.open_1c = true;
                current_settings.menu_allowed.open_dir = true;
                if (sac_deque.size() > 0) { current_settings.menu_allowed.save_1c = true; } else { current_settings.menu_allowed.save_1c = false; }
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
            current_settings.menu_allowed.window_menu = true;
            current_settings.menu_allowed.center_windows = true;
            current_settings.menu_allowed.save_layout = true;
            current_settings.menu_allowed.reset_windows = true;
            current_settings.menu_allowed.welcome = true;
            current_settings.menu_allowed.fps = true;
            break;
        default:
            // We're in an unknown state
            program_status.state.store(idle);
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
    ImGuiIO& io = pssp::start_graphics(window, glsl_version, program_path);
    //---------------------------------------------------------------------------
    // End Initialization
    //---------------------------------------------------------------------------

    //---------------------------------------------------------------------------
    // Misc Draw loop variables
    //---------------------------------------------------------------------------
    // Default color for clearing the screen
    ImVec4 clear_color = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
    pssp::ProgramSettings current_settings{};
    // IT WORKS! THAT TOOK WAY TOO LONG!
    // Testing making a new proto-project
    current_settings.create_new_project(std::filesystem::current_path());
    // Testing loading a new proto-project
    current_settings.load_project(std::filesystem::current_path() / current_settings.md_file);
    pssp::fps_info fps_tracker{};
    std::string_view welcome_message{"Welcome to Passive-source Seismic-processing (PsSP)!"};
    pssp::AllFilterOptions af_settings{};
    pssp::ProgramStatus program_status{};
    // Time-series
    std::deque<pssp::sac_1c> sac_deque;
    // Spectrum (only 1 for now)
    pssp::sac_1c spectrum;
    // Which sac-file is active
    int active_sac{};
    bool clear_sac{false};
    //---------------------------------------------------------------------------
    // End Misc Draw loop variables
    //---------------------------------------------------------------------------
    // Make a fresh project
    pssp::Project project{};
    //---------------------------------------------------------------------------
    // Draw loop
    //---------------------------------------------------------------------------
    // The draw loop is ran EVERY FRAME, so be careful to make the stuff in here safe
    // (minimize spurious work, be thread-safe, etc.)
    while (!glfwWindowShouldClose(window))
    {
        // Each frame, we need to check the program's state
        // to determine what we are and are not allowed to do
        handle_program_state(program_status, current_settings, project, sac_deque);
        // Do we need to remove a sac_1c from the sac_deque?
        cleanup_sac(project, sac_deque, active_sac, clear_sac);
        // Start the frame
        pssp::prep_newframe();
        status_bar(program_status);
        main_menu_bar(window, current_settings.window_settings, current_settings.menu_allowed, af_settings, program_status, sac_deque, active_sac, project);
        // Show the Welcome window if appropriate
        window_welcome(current_settings.window_settings.welcome, welcome_message);
        update_fps(fps_tracker, io);
        // Show the FPS window if appropriate
        window_fps(fps_tracker, current_settings.window_settings.fps);
        // Only if there are files in the sac_deque
        if (sac_deque.size() > 0)
        {
          // This fixes the issue of deleting all sac_1cs in the deque
          // loading new ones, and then trying to access the -1 element
          if (active_sac < 0) { active_sac = 0; } else if (active_sac >= static_cast<int>(sac_deque.size())) { active_sac = sac_deque.size() - 1; }
          window_sac_header(current_settings.window_settings.header, sac_deque[active_sac]);
          // Show processing history window is appropriate
          window_processing_history(current_settings.window_settings.processing_history, project, sac_deque[active_sac].data_id);
          // Show the Sac Plot window if appropriate
          window_plot_sac(current_settings.window_settings.plot_1c, sac_deque, active_sac);
          // Show Checkpoint naming window if appropriate
          window_name_checkpoint(current_settings.window_settings.name_checkpoint, program_status, project, sac_deque);
          // Show Checkpoint note window if appropriate
          window_notes_checkpoint(current_settings.window_settings.notes_checkpoint, project);
          // Show the Sac Spectrum window if appropriate
          // We need to see if the FFT needs to be calculated (don't want to do it
          // every frame)
          if (current_settings.window_settings.spectrum_1c.show)
          {
            // This logic needs to be modified so that we have a better mechanism to avoid
            // calculating this when it isn't desired
              bool compare_names{true};
              {
                  std::shared_lock<std::shared_mutex> lock_spectrum(spectrum.mutex_);
                  std::shared_lock<std::shared_mutex> lock_sac(sac_deque[active_sac].mutex_);
                  compare_names = (spectrum.file_name == sac_deque[active_sac].file_name);
              }
              // If they're not the same, then calculate the FFT
              if (!compare_names) { calc_spectrum(sac_deque[active_sac], spectrum); }
          }
          // Finally plot the spectrum
          window_plot_spectrum(current_settings.window_settings.spectrum_1c, spectrum);
          // Show the Sac List window if appropriate
          window_sac_deque(current_settings.window_settings, current_settings.menu_allowed, sac_deque, spectrum, active_sac, clear_sac);
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
                program_status.thread_pool.enqueue(pssp::batch_apply_lowpass, std::ref(project), std::ref(program_status), std::ref(sac_deque), std::ref(af_settings.lowpass));
                af_settings.lowpass.apply_batch = false;
            }
            else
            {
                program_status.tasks_completed = 0;
                program_status.total_tasks = 1;
                program_status.thread_pool.enqueue(pssp::apply_lowpass, std::ref(project), std::ref(program_status), std::ref(sac_deque[active_sac]), std::ref(af_settings.lowpass));
            }
            af_settings.lowpass.apply_filter = false;
        }
        else if (af_settings.highpass.apply_filter)
        {
            if (af_settings.highpass.apply_batch)
            {
                program_status.thread_pool.enqueue(pssp::batch_apply_highpass, std::ref(project), std::ref(program_status), std::ref(sac_deque), std::ref(af_settings.highpass));
                af_settings.highpass.apply_batch = false;
            }
            else
            {
                program_status.tasks_completed = 0;
                program_status.total_tasks = 1;
                program_status.thread_pool.enqueue(pssp::apply_highpass, std::ref(project), std::ref(program_status), std::ref(sac_deque[active_sac]), std::ref(af_settings.highpass));
            }
            af_settings.highpass.apply_filter = false;
        }
        else if (af_settings.bandpass.apply_filter)
        {
            if (af_settings.bandpass.apply_batch)
            {
                program_status.thread_pool.enqueue(pssp::batch_apply_bandpass, std::ref(project), std::ref(program_status), std::ref(sac_deque), std::ref(af_settings.bandpass));
                af_settings.bandpass.apply_batch = false;
            }
            else
            {
                program_status.tasks_completed = 0;
                program_status.total_tasks = 1;
                program_status.thread_pool.enqueue(pssp::apply_bandpass, std::ref(project), std::ref(program_status), std::ref(sac_deque[active_sac]), std::ref(af_settings.bandpass));
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

