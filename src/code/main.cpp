//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
// This is where the project Class gets defined
#include "pssp_projects.hpp"
// Where we will have all our window functions defined
#include "pssp_windows.hpp"
// Definitions of Misc Structs/Classes/Functions
#include "pssp_misc.hpp"
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
// TODO
//-----------------------------------------------------------------------------
// Updated on 15 May 2023 (too lazy to renumber daily)
// 1) Bandreject filter
// 2) Data-request/download from IRIS (or other server)
// 3) Mapping of data (implot has a nifty map example code)
// 4) Sorting of data in sac_deque
//  4a) By filename
//  4b) By component
//  4c) By station
//  4d) By event station distance
//  4e) By azimuth/back-azimuth
//  4f) By eventid
//  4g) ???? Basically any header variable
// 5) Geometric calculations (gcarc, az, baz, dist)
// 6) 3C geometric calculations (rotate components)
// 7) Manual arrival time picking
// 8) Automatic arrival time picking (STA/LTA time-series, A1C, STA/LTA spectrogram)
// 9) Spectrogram
// 10) Grouping data
//  10a) Three-component
//  10b) Array/sub-array
//  10c) Event
//  10d) Manually via the user, can provide their own name
// 11) Help menu
// 12) Center windows functionality
// 13) Overwrite layout functionality
// 14) Advanced plots (record section, particle motion)
// 15) Deconvolution (instrument response, source wavelet)
//  15a) Spectral division with water-level
//  15b) Spectral division with static shift
//  15c) Iterative time-domain
// 16) Reload all data
// 17) Unload all data
// 18) Data-processing logs (probably easier than doing projects...)
// 19) Generic basic waveforms (for exploring effects of processing flow)
//  19a) Dirac Delta function
//  19b) Dirac Delta-comb function
//  19c) Boxcar function
//  19d) Triangle function
//  19e) Gaussian
//  19f) Sombrero function
// 20) Keyboard shortcuts for common operations
// 21) Tab-key navigation between components in window
// 22) User note's log (can write their own notes on what they're doing)
// 23) Datetime functionality
// 24) Migrate all SAC stuff from floats to doubles (while maintaining read/write compatibility)
// 25) Don't use std::cout or std::cerr, use exceptions and then try-catch blocks to
//    check for exceptions
// 26) Plotting appropriately down-sampled seismograms
//    The plot window has a width in pixels, there is no need to show with a >1 ratio of
//    data-points to pixels (literally couldn't see the difference)
//    The xlimits on the window tell us how much of our data is shown, if the data/pixel
//    ratio >1, down-sample by taking every-other point, if density if <0.5 then upsample
//    (unless the window is bigger than the available data)
//    Can then tweak the ratio bounds for when the swap should happen to try to keep things
//    smooth and clean looking
//    Could possibly have 3 versions (one denser than shown, the shown, one less dense than shown)
//    and depending on the change in zoom level we either pop the front or the back and insert
//    a new one in it's place (to make the transition feel fluid instead of jagged)
//    The reason to even bother with this is that plotting something with 150,000 data-points
//    tanks my framerate from ~120 fps on my macbook pro to ~40 fps, even though at most, I can see
//    2560 pixels wide (retina display), and the plot is only ~60% the width of my screen tops.
//    So those frames are dropping for stuff I can't even see anyway!
//    We'll need to make sure the plotted data is within the plot-frame plus a bit extra on each side
//    if data exists there, we'll also need to shift it into the correct place
// 27) Currently, the way to prevent crashing when processing is happening is to hide the
//    "Sac List" window. The reason it to prevent the user selecting data that is not accessible yet.
//    I would prefer to gray-out the window and make the options non-selectable during processing.
// 28) FFTW is not thread-safe. It seems to have the possibility to be thread-safe if compiled with the
//    correct flag. Because on MacOS I use the default provided by homebrew, I don't have that.
//    Either I should look into an FFT library that is thread-safe or streamline compiling FFTW as thread-safe.
//  28a) It may not be necessary, performing bandpass (FFT + gain calculation + IFFT) on 700 files on a single-thread
//    using FFTW takes about as long as reading in those 700 files across 7 threads. I suspect this will be a bit
//    more awkward on a more powerful machine?
//    On my Linux machine, reading 700 files takes ~2-3 seconds, doing a bandpass on them all is about 10 seconds
//    On my Mac laptop, reading 700 files takes ~12 seconds, doing a bandpass on them all is about 11 seconds
//  28b) An FFTW plan pool may make it possible to use FFTW in a multi-threaded capacity without needing its
//    thread-safe compilation pattern (which they advocate against using: https://www.fftw.org/fftw3_doc/Thread-safety.html)
//    Idea is to have a pool of plans for different sized input vector.
//    If a vector needs one of a size that doesn't exist, it gets created.
//    If a vector needs one that does exist, if it is unused it takes it, otherwise it waits for it to be free.
//    At the end, all plans get destroyed. It reduces the overhead of repeatidly creating/destroying plans
//    and makes it thread-safe.
//  28c) Possibly a wrapper class for FFTW, maybe that'll make it actually thread-safe...
// 29) Program logging.
//-----------------------------------------------------------------------------
// End TODO
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
    pssp::Project current_project{};
    // IT WORKS! THAT TOOK WAY TOO LONG!
    // Testing making a new proto-project
    current_project.create_new_project(std::filesystem::current_path());
    // Testing loading a new proto-project
    current_project.load_project(std::filesystem::current_path() / current_project.md_file);
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
        pssp::main_menu_bar(window, current_project.window_settings, current_project.menu_allowed, af_settings, program_status, sac_deque, active_sac);
        // Show the Welcome window if appropriate
        pssp::window_welcome(current_project.window_settings.welcome, welcome_message);
        pssp::update_fps(fps_tracker, io);
        // Show the FPS window if appropriate
        pssp::window_fps(fps_tracker, current_project.window_settings.fps);
        // Doesn't matter if files are in the sac_deque or not
        {
            std::shared_lock<std::shared_mutex> lock_program(program_status.program_mutex);
            if (program_status.is_idle)
            {
                current_project.menu_allowed.open_1c = true;
                current_project.menu_allowed.open_dir = true;
            }
            else
            {
                current_project.menu_allowed.open_1c = false;
                current_project.menu_allowed.open_dir = false;
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
                  current_project.menu_allowed.save_1c = true;
                  current_project.menu_allowed.batch_menu = true;
                  current_project.menu_allowed.processing_menu = true;
                  current_project.menu_allowed.lowpass = true;
                  current_project.menu_allowed.highpass = true;
                  current_project.menu_allowed.bandpass = true;
                  current_project.menu_allowed.rmean = true;
                  current_project.menu_allowed.rtrend = true;
              }
              else
              {
                  current_project.menu_allowed.save_1c = false;
                  current_project.menu_allowed.batch_menu = false;
                  current_project.menu_allowed.processing_menu = false;
                  current_project.menu_allowed.lowpass = false;
                  current_project.menu_allowed.highpass = false;
                  current_project.menu_allowed.bandpass = false;
                  current_project.menu_allowed.rmean = false;
                  current_project.menu_allowed.rtrend = false;
              }
          }
          // Allow menu options that require sac files
          current_project.menu_allowed.sac_deque = true;
          current_project.menu_allowed.sac_header = true;
          current_project.menu_allowed.plot_1c = true;
          current_project.menu_allowed.plot_spectrum_1c = true;
          // This fixes the issue of deleting all sac_1cs in the deque
          // loading new ones, and then trying to access the -1 element
          if (active_sac < 0) { active_sac = 0; }
          pssp::window_sac_header(program_status, current_project.window_settings.header, sac_deque[active_sac]);
          // Show the Sac Plot window if appropriate
          pssp::window_plot_sac(current_project.window_settings.plot_1c, sac_deque, active_sac);
          // Show the Sac Spectrum window if appropriate
          // We need to see if the FFT needs to be calculated (don't want to do it
          // every frame)
          if (current_project.window_settings.spectrum_1c.show)
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
          pssp::window_plot_spectrum(current_project.window_settings.spectrum_1c, spectrum);
          // Show the Sac List window if appropriate
          pssp::window_sac_deque(current_project.window_settings, current_project.menu_allowed, program_status, sac_deque, spectrum, active_sac, clear_sac);
          pssp::window_lowpass_options(program_status, current_project.window_settings.lowpass, af_settings.lowpass);
          pssp::window_highpass_options(program_status, current_project.window_settings.highpass, af_settings.highpass);
          pssp::window_bandpass_options(program_status, current_project.window_settings.bandpass, af_settings.bandpass);
        }
        else
        {
            // Disallow menu options that require sac files
            current_project.menu_allowed.save_1c = false;
            current_project.menu_allowed.sac_deque = false;
            current_project.menu_allowed.sac_header = false;
            current_project.menu_allowed.plot_1c = false;
            current_project.menu_allowed.plot_spectrum_1c = false;
            current_project.menu_allowed.processing_menu = false;
            current_project.menu_allowed.lowpass = false;
            current_project.menu_allowed.highpass = false;
            current_project.menu_allowed.bandpass = false;
            current_project.menu_allowed.rmean = false;
            current_project.menu_allowed.rtrend = false;
            current_project.menu_allowed.batch_menu = false;
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
