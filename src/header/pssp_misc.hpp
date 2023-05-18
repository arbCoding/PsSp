#ifndef PSSP_MISC_HPP
#define PSSP_MISC_HPP

// Silence OpenGL deprecation warnings on compile
#define GL_SILENCE_DEPRECATION

//-----------------------------------------------------------------------------
// Include statments
//-----------------------------------------------------------------------------
// pssp::ThreadPool
#include "pssp_threadpool.hpp"
// pssp::Project class
#include "pssp_projects.hpp"
// SAC namespace filters
#include <sac_spectral.hpp>
// Dear ImGui and Graphical Backends
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// ImPlot add-on for Dear ImGui
#include <implot.h>
// GLFW graphical backend
#include <GLFW/glfw3.h>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <atomic>
#include <shared_mutex>
#include <deque>
#include <filesystem>
//-----------------------------------------------------------------------------
// End Include statments
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
// Having everything in main.cpp began to really convolute the appearance of the
// main program's logic.
// To make this easier, I'm splitting off structs/Class/functions and what-not
// to this new header/implementation combo. This is stuff that at the moment
// seems to have no real place to belong.
//
// Other Header/Implementation combos to consider are:
// * pssp_projects.hpp/.cpp
//      User projects related structs/classes/functions
// * pssp_threadpool.hpp (no .cpp for this)
//      ThreadPool class for multi-threaded fun
// * pssp_windows.hpp/.cpp
//      These are the Dear ImGui windows (and ImPlot plots) that the program
//      displays. These are ran every frame on the main thread.
//      Anything you want to run in the ThreadPool needs to be passed to the
//      animation loop with a flag stating it is ready to be passed to task
//      queue
// * sac_spectral.hpp/.cpp
//      FFT, iFFT, and filters on SAC::SacStream objects (or sac_1c structs)
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Begin pssp namespace
//-----------------------------------------------------------------------------
namespace pssp
{
//-----------------------------------------------------------------------------
// Custom structs
//-----------------------------------------------------------------------------
struct FileIO
{
    std::atomic<int> count{0};
    std::atomic<int> total{0};
    // Used to flag if we're reading or not 
    std::atomic<bool> is_reading{false};
    // Used to flag if we're processing data or not
    std::atomic<bool> is_processing{false};
    std::shared_mutex mutex_{};
};
//
struct ProgramStatus
{
    std::atomic<float> progress{1.1f};
    std::shared_mutex program_mutex{};
    FileIO fileio{};
    // Flag to specify if we're idle or doing something else
    std::atomic<bool> is_idle{true};
    // Our thread pool
    ThreadPool thread_pool{};
};
//
struct fps_info
{ 
    float prev_time{0.0f};
    float current_time{0.f};
    float current_interval{0.f};
    int frame_count{0};
    float fps{0.0f};
    // How often we update the fps tracker
    float reporting_interval{0.2f};
    std::mutex mutex_{};
};

struct sac_1c
{
    std::string file_name{};
    SAC::SacStream sac{};
    std::shared_mutex mutex_{};

    sac_1c() : file_name(), sac(), mutex_() {}
    // Copy constructor
    sac_1c(const sac_1c& other)
    {
        file_name = other.file_name;
        sac = other.sac;
        // Don't copy the mutex
    }
    // Assignment operator
    sac_1c& operator=(const sac_1c& other)
    {
        if (this != &other)
        {
            file_name = other.file_name;
            sac = other.sac;
            // Don't assign the mutex
        }
        return *this;
    }
};
// Struct for filters
struct FilterOptions
{
    // Filter order
    int order{1};
    // Limits on order
    int min_order{1};
    int max_order{10};
    // Limits on filter frequencies
    float min_freq{0.0f};
    // Two freqs for bandpass
    // If using lowpass use freq_low
    float freq_low{1.0f};
    // If using highpass use freq_high
    float freq_high{5.0f};
    // Keyboard step interval
    float freq_step{0.1f};
    // Do we apply the filter at all
    bool apply_filter{false};
    // Do we apply filter to a batch file
    bool apply_batch{false};
    std::shared_mutex mutex_{};
};
// Holds the options for all filters, just to make life easier
struct AllFilterOptions
{
    FilterOptions lowpass{};
    FilterOptions highpass{};
    FilterOptions bandpass{};
    FilterOptions bandreject{};
};
//-----------------------------------------------------------------------------
// End Custom structs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Misc function forward declarations
//-----------------------------------------------------------------------------
// Updates the FPS tracker
void update_fps(fps_info& fps, ImGuiIO& io);
// Removes the selected SacStream from the deque
void cleanup_sac(std::deque<sac_1c>& sac_deque, int& selected, bool& clear);
// Calculates real/imaginary spectrum of sac_1c object
void calc_spectrum(sac_1c& sac, sac_1c& spectrum);
// Remove mean from sac_1c object
void remove_mean(FileIO& fileio, sac_1c& sac);
// Remove mean from all sac_1c objects in a deque
void batch_remove_mean(ProgramStatus& program_status, std::deque<sac_1c>& sac_deque);
// Remove trend from a sac_1c object
void remove_trend(FileIO& fileio, sac_1c& sac);
// Remove trend from all sac_1c objects in a deque
void batch_remove_trend(ProgramStatus& program_status, std::deque<sac_1c>& sac_deque);
// Turns out FFTW is not thread-safe and doesn't provide that on Mac
// I could compile it manually, but I don't want to
// So we're going to change how we do this, one function for solo
// One function for many
// Lowpass one sac_1c
void apply_lowpass(FileIO& fileio, sac_1c& sac, FilterOptions& lowpass_options);
// Lowpass all sac_1c's in a deque
void batch_apply_lowpass(ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, FilterOptions& lowpass_options);
// Highpass one sac_1c
void apply_highpass(FileIO& fileio, sac_1c& sac, FilterOptions& highpass_options);
// Highpass all sac_1c's in a deque
void batch_apply_highpass(ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, FilterOptions& highpass_options);
// Bandpass one sac_1c
void apply_bandpass(FileIO& fileio, sac_1c& sac, FilterOptions& bandpass_options);
// Bandpass all sac_1c's in a deque
void batch_apply_bandpass(ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, FilterOptions& bandpass_options);
// Read in a single sac_file
void read_sac_1c(std::deque<sac_1c>& sac_deque, FileIO& fileio, const std::string file_name, Project& project);
// Read all SAC files in a directory
void scan_and_read_dir(ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, std::filesystem::path directory, Project& project);
// Setup the graphical backends
const char* setup_gl();
// Start the graphical backends, create ImGui and ImPlot contexts and get the ImGuiIO
// stuff (Fonts, other things I'm sure)
ImGuiIO& start_graphics(GLFWwindow* window, const char* glsl_version, std::filesystem::path program_path);
// Cleanly destroy everything, performed just before program ends
void end_graphics(GLFWwindow* window);
// Helper program for errors with glfw
void glfw_error_callback(int error, const char *description);
// Ran at beginning of new frame draw cycle
void prep_newframe();
// Ran at end of new frame draw cycle
void finish_newframe(GLFWwindow* window, ImVec4 clear_color);
//-----------------------------------------------------------------------------
// End Misc function forward declarations
//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// End pssp namespace
//-----------------------------------------------------------------------------
#endif
