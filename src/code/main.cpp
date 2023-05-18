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
// User projects, see ./src/header/pssp_projects.hpp and 
// ./src/implementation/pssp_projects.cpp for details.
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
    pssp::Project project("test", program_path);

    //---------------------------------------------------------------------------
    // Draw loop
    //---------------------------------------------------------------------------
    // The draw loop is ran EVERY FRAME, so be careful to make the stuff in here safe
    // (minimize spurious work, be thread-safe, etc.)
    while (!glfwWindowShouldClose(window))
    {
        // Do we need to remove a sac_1c from the sac_deque?
        cleanup_sac(sac_deque, active_sac, clear_sac);
        // Start the frame
        pssp::prep_newframe();
        pssp::status_bar(program_status);
        pssp::main_menu_bar(window, current_settings.window_settings, current_settings.menu_allowed, af_settings, program_status, sac_deque, active_sac, project);
        // Show the Welcome window if appropriate
        pssp::window_welcome(current_settings.window_settings.welcome, welcome_message);
        pssp::update_fps(fps_tracker, io);
        // Show the FPS window if appropriate
        pssp::window_fps(fps_tracker, current_settings.window_settings.fps);
        // Doesn't matter if files are in the sac_deque or not
        {
            std::shared_lock<std::shared_mutex> lock_program(program_status.program_mutex);
            if (program_status.is_idle)
            {
                current_settings.menu_allowed.open_1c = true;
                current_settings.menu_allowed.open_dir = true;
            }
            else
            {
                current_settings.menu_allowed.open_1c = false;
                current_settings.menu_allowed.open_dir = false;
            }
        }
        // Only if there are files in the sac_deque
        if (sac_deque.size() > 0)
        {
            // Some things require the program to be idle
            {
              std::shared_lock<std::shared_mutex> lock_program(program_status.program_mutex);
              if (program_status.is_idle)
              {
                  current_settings.menu_allowed.save_1c = true;
                  current_settings.menu_allowed.batch_menu = true;
                  current_settings.menu_allowed.processing_menu = true;
                  current_settings.menu_allowed.lowpass = true;
                  current_settings.menu_allowed.highpass = true;
                  current_settings.menu_allowed.bandpass = true;
                  current_settings.menu_allowed.rmean = true;
                  current_settings.menu_allowed.rtrend = true;
              }
              else
              {
                  current_settings.menu_allowed.save_1c = false;
                  current_settings.menu_allowed.batch_menu = false;
                  current_settings.menu_allowed.processing_menu = false;
                  current_settings.menu_allowed.lowpass = false;
                  current_settings.menu_allowed.highpass = false;
                  current_settings.menu_allowed.bandpass = false;
                  current_settings.menu_allowed.rmean = false;
                  current_settings.menu_allowed.rtrend = false;
              }
          }
          // Allow menu options that require sac files
          current_settings.menu_allowed.sac_deque = true;
          current_settings.menu_allowed.sac_header = true;
          current_settings.menu_allowed.plot_1c = true;
          current_settings.menu_allowed.plot_spectrum_1c = true;
          // This fixes the issue of deleting all sac_1cs in the deque
          // loading new ones, and then trying to access the -1 element
          if (active_sac < 0) { active_sac = 0; }
          pssp::window_sac_header(program_status, current_settings.window_settings.header, sac_deque[active_sac]);
          // Show the Sac Plot window if appropriate
          pssp::window_plot_sac(current_settings.window_settings.plot_1c, sac_deque, active_sac);
          // Show the Sac Spectrum window if appropriate
          // We need to see if the FFT needs to be calculated (don't want to do it
          // every frame)
          if (current_settings.window_settings.spectrum_1c.show)
          {
              bool compare_names{true};
              {
                  std::shared_lock<std::shared_mutex> lock_spectrum(spectrum.mutex_);
                  std::shared_lock<std::shared_mutex> lock_sac(sac_deque[active_sac].mutex_);
                  compare_names = (spectrum.file_name == sac_deque[active_sac].file_name);
              }
              // If they're not the same, then calculate the FFT
              if (!compare_names) { pssp::calc_spectrum(sac_deque[active_sac], spectrum); }
          }
          // Finally plot the spectrum
          pssp::window_plot_spectrum(current_settings.window_settings.spectrum_1c, spectrum);
          // Show the Sac List window if appropriate
          pssp::window_sac_deque(current_settings.window_settings, current_settings.menu_allowed, program_status, sac_deque, spectrum, active_sac, clear_sac);
          pssp::window_lowpass_options(program_status, current_settings.window_settings.lowpass, af_settings.lowpass);
          pssp::window_highpass_options(program_status, current_settings.window_settings.highpass, af_settings.highpass);
          pssp::window_bandpass_options(program_status, current_settings.window_settings.bandpass, af_settings.bandpass);
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
            std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
            program_status.fileio.count = 0;
            program_status.fileio.is_processing = true;
            if (af_settings.lowpass.apply_batch)
            {
                program_status.thread_pool.enqueue(pssp::batch_apply_lowpass, std::ref(program_status), std::ref(sac_deque), std::ref(af_settings.lowpass));
                af_settings.lowpass.apply_batch = false;
            }
            else
            {
                program_status.fileio.total = 1;
                program_status.thread_pool.enqueue(pssp::apply_lowpass, std::ref(program_status.fileio), std::ref(sac_deque[active_sac]), std::ref(af_settings.lowpass));
            }
            af_settings.lowpass.apply_filter = false;
        }
        else if (af_settings.highpass.apply_filter)
        {
            std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
            program_status.fileio.count = 0;
            program_status.fileio.is_processing = true;
            if (af_settings.highpass.apply_batch)
            {
                program_status.thread_pool.enqueue(pssp::batch_apply_highpass, std::ref(program_status), std::ref(sac_deque), std::ref(af_settings.highpass));
                af_settings.highpass.apply_batch = false;
            }
            else
            {
                program_status.fileio.total = 1;
                program_status.thread_pool.enqueue(pssp::apply_highpass, std::ref(program_status.fileio), std::ref(sac_deque[active_sac]), std::ref(af_settings.highpass));
            }
            af_settings.highpass.apply_filter = false;
        }
        else if (af_settings.bandpass.apply_filter)
        {
            std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
            program_status.fileio.count = 0;
            program_status.fileio.is_processing = true;
            if (af_settings.bandpass.apply_batch)
            {
                program_status.thread_pool.enqueue(pssp::batch_apply_bandpass, std::ref(program_status), std::ref(sac_deque), std::ref(af_settings.bandpass));
                af_settings.bandpass.apply_batch = false;
            }
            else
            {
                program_status.fileio.total = 1;
                program_status.thread_pool.enqueue(pssp::apply_bandpass, std::ref(program_status.fileio), std::ref(sac_deque[active_sac]), std::ref(af_settings.bandpass));
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
    // End program
    return 0;
}
//-----------------------------------------------------------------------------
// End Main
//-----------------------------------------------------------------------------
