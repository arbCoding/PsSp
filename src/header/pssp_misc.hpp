#ifndef PSSP_MISC_HPP_20230610
#define PSSP_MISC_HPP_20230610

// Silence OpenGL deprecation warnings on compile
#define GL_SILENCE_DEPRECATION

//-----------------------------------------------------------------------------
// Include statments
//-----------------------------------------------------------------------------
#include "pssp_data_pool.hpp"
#include "pssp_data_trees.hpp"
// pssp::FFTWPlanPool class
#include "pssp_fftw_planpool.hpp"
#include "pssp_program_settings.hpp"
// pssp::Project class
#include "pssp_projects.hpp"
// Spectral proccesing functionality
#include "pssp_spectral.hpp"
// pssp::ThreadPool
#include "pssp_threadpool.hpp"
// GLFW graphical backend
#include <GLFW/glfw3.h>
// Dear ImGui and Graphical Backends
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// ImPlot add-on for Dear ImGui
#include <implot.h>
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <atomic>
#include <filesystem>
#include <mutex>
#include <shared_mutex>
#include <sstream>
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
// Enum for program state
enum class program_state{ in, out, processing, idle };

struct ProgramStatus
{
    std::shared_mutex program_mutex{};
    std::atomic<program_state> state{program_state::idle};
    // How many tasks have been completed
    std::atomic<int> tasks_completed{0};
    // How many tasks do we need to do in order to complete
    std::atomic<int> total_tasks{0};
    // Program progress, handy to keep around
    // Values > 1.0 hide the progress bar!
    std::atomic<float> progress{1.1f};
    // Our thread pool
    ThreadPool thread_pool{};
    // Our FFTW Plan pool
    FFTWPlanPool fftw_planpool{};
    std::string status_message{};
    // Our Data pool
    DataPool data_pool{};
    // Our Project
    Project project{};
    // The data_id of the active sac file
    int data_id{-1};
};
// Struct for Frames Per Second info
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
void update_fps(fps_info& fps, const ImGuiIO& io);
// Removes the selected SacStream from the deque
//void cleanup_sac(Project& project, std::deque<sac_1c>& sac_deque, int& selected, bool& clear);
// Calculates real/imaginary spectrum of sac_1c object
void calc_spectrum(ProgramStatus& program_status, sac_1c& visual_sac, sac_1c& spectrum);
// Calculate the mean of a series of doubles
double calc_mean(const std::vector<double>& data_vector);
// Remove mean from sac_1c object
void remove_mean(ProgramStatus& program_status, int data_id);
// Remove mean from all sac_1c objects in a deque
void batch_remove_mean(ProgramStatus& program_status);
// Remove trend from a sac_1c object
void remove_trend(ProgramStatus& program_status, int data_id);
// Remove trend from all sac_1c objects in a deque
void batch_remove_trend(ProgramStatus& program_status);
// Lowpass one sac_1c
void apply_lowpass(ProgramStatus& program_status, int data_id, const FilterOptions& lowpass_options);
// Lowpass all sac_1c's in a deque
void batch_apply_lowpass(ProgramStatus& program_status, const FilterOptions& lowpass_options);
// Highpass one sac_1c
void apply_highpass(ProgramStatus& program_status, int data_id, const FilterOptions& highpass_options);
// Highpass all sac_1c's in a deque
void batch_apply_highpass(ProgramStatus& program_status, const FilterOptions& highpass_options);
// Bandpass one sac_1c
void apply_bandpass(ProgramStatus& program_status, int data_id, const FilterOptions& bandpass_options);
// Bandpass all sac_1c's in a deque
void batch_apply_bandpass(ProgramStatus& program_status, const FilterOptions& bandpass_options);
// Read in a single sac file and add it to the DataPool
void read_sac(ProgramStatus& program_status, const std::filesystem::path& file_name);
// Read all SAC files in a directory
void scan_and_read_dir(ProgramStatus& program_status, const std::filesystem::path& directory);
// Setup the graphical backends
const char* setup_gl();
// Start the graphical backends, create ImGui and ImPlot contexts and get the ImGuiIO
// stuff (Fonts, other things I'm sure)
ImGuiIO& start_graphics(GLFWwindow* window, const char* glsl_version, const std::filesystem::path& program_path);
// Cleanly destroy everything, performed just before program ends
void end_graphics(GLFWwindow* window);
// Helper program for errors with glfw
void glfw_error_callback(int error, const char *description);
// Ran at beginning of new frame draw cycle
void prep_newframe();
// Ran at end of new frame draw cycle
void finish_newframe(GLFWwindow* window, ImVec4 clear_color);
// Add all each datapoint ot a checkpoint
void checkpoint_data(ProgramStatus& program_status, const int data_id, const int checkpoint_id);
// Unload the project
void unload_data(ProgramStatus& program_status);
// Load a single bit of data
void load_2_data_pool(ProgramStatus& program_status, const int data_id);
// Load an existing project
void load_data(ProgramStatus& program_status, const std::filesystem::path& project_file, int checkpoint_id);
// Shitty lowpass for testing
void lowpass(FFTWPlanPool& plan_pool, std::shared_ptr<sac_1c> sac_ptr, int order, double cutoff);
// Shitty highpass for testing
void highpass(FFTWPlanPool& plan_pool, std::shared_ptr<sac_1c> sac_ptr, int order, double cutoff);
// Shitty bandpass for testing
void bandpass(FFTWPlanPool& plan_pool, std::shared_ptr<sac_1c> sac_ptr, int order, double lowpass, double highpass);
// Shitty bandreject for testing
void bandreject(FFTWPlanPool& plan_pool, std::shared_ptr<sac_1c> sac_ptr, int order, double lowreject, double highreject);
// Write a checkpoint (need to set project.checkpoint_name first!)
void write_checkpoint(ProgramStatus& program_status, bool author, bool cull);
// Delete checkpoint data for a data_id (allows to be parallel)
void delete_data_id_checkpoint(ProgramStatus& program_status, Project& project, int checkpoint_id, int data_id);
// Delete a checkpoint
void delete_checkpoint(ProgramStatus& program_status, Project& project, int checkpoint_id);
//-----------------------------------------------------------------------------
// End Misc function forward declarations
//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// End pssp namespace
//-----------------------------------------------------------------------------
#endif
