// Silence OpenGL deprecation warnings on compile
#define GL_SILENCE_DEPRECATION
//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
// SAC:: spectral functions
#include "sac_spectral.hpp"
// SAC::SacStream class
#include <sac_stream.hpp>
// Dear ImGui header and backends
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// FileDialog add-on for Dear ImGui
#include <ImGuiFileDialog.h>
// ImPlot add-on for Dear ImGui
#include <implot.h>
// GLFW graphical backend
#include <GLFW/glfw3.h>
// Standard Library stuff, https://en.cppreference.com/
// std::cout, std::cerr
#include <iostream>
// mutex locks for thread-safe data access
#include <mutex>
// Shared mutex for locking modifications of file
// but not reading a file
#include <shared_mutex>
// std::string_view
#include <string>
// Path stuff
#include <filesystem>
// std::vector
#include <vector>
// std::clamp
#include <algorithm>
// std::future
// Used for handling asynchronous data manipulation
// without needing tons of mutex's
#include <future>
// std::async, multi-threaded work
#include <thread>
// Unsure if needed
//#include <iterator>
//-----------------------------------------------------------------------------
// End include statements
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// TODO
//-----------------------------------------------------------------------------
// 1) Bandreject filter
// 2) Lock program on batch jobs (read/write/process)
// 3) Progress window on batch jobs (read/write/process)
// 4) Data doesn't necessarily need to reside in memory at all times
//    Only need for processing/plotting/saving
//    Therefore, should implement a function to read and store only sac-header
//    information, and then read in data on the fly as needed.
//    (Chunks of data being in memory is ideal, that way we have a buffer that
//    acts basically like a sliding window for the I/O operations)
// 5) Threaded I/O (for reading/writing multiple files at a time)
// 6) Threaded processing (for processing multiple files at a time)
// 7) Undo/redo (that won't be fun...)
// 8) Projects (that won't be fun...)
// 9) Data-request/download from IRIS (or other server)
// 10) Mapping of data
// 11) Sorting of data in sac_vector
//  11a) By filename
//  11b) By component
//  11c) By station
//  11d) By event station distance
//  11e) By azimuth/back-azimuth
//  11f) By eventid
//  11g) ????
// 12) Batch filtering
// 13) Manual arrival time picking
// 14) Automatic arrival time picking (STA/LTA time-series, A1C, STA/LTA spectrogram)
// 15) Spectrogram
// 16) Grouping 3-component data
// 17) Help menu
// 18) Center windows functionality
// 19) Overwrite layout functionality
// 20) Advanced plots (record section, particle motion)
// 21) Deconvolution (instrument response, source wavelet)
//  21a) Spectral division with water-level
//  21b) Spectral division with static shift
//  21c) Iterative time-domain
// 22) Reload all data
// 23) Data-processing logs (probably easier than doing projects...)
// 24) Generic basic waveforms (for exploring effects of processing flow)
//  24a) Dirac Delta function
//  24b) Dirac Delta-comb function
//  24c) Boxcar function
//  24d) Triangle function
//  24e) Gaussian
//  24f) Sombrero function
// 25) Keyboard shortcuts for common operations
// 26) User note's log (can write their own notes on what they're doing)
//-----------------------------------------------------------------------------
// End TODO
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Known Bugs
//-----------------------------------------------------------------------------
//
//------------------------------------------------------------------------
// Template for bugs, info and what-not goes between these sub-dividers
//------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
// End Known Bugs
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
  bool to_read{false};
  bool started{false};
  int count{0};
  int total{0};
  std::vector<std::string> file_vector{};
};
struct ProgramStatus
{
  std::string message{"Idle"};
  float progress{1.1f};
  std::shared_mutex program_mutex{};
  FileIO fileio{};
};
// Per window settings
struct WindowSettings
{
  // X position (left to right)
  int pos_x{};
  // Y position (top to bottom)
  int pos_y{};
  int width{};
  int height{};
  bool show{true};
  // if false, position and size get set
  bool is_set{false};
};
// Settings for all windows
struct AllWindowSettings
{
  // Window with welcome message
  WindowSettings welcome_settings{395, 340, 525, 60};
  // FPS tracker window
  WindowSettings fps_settings{1, 756, 60, 55, false};
  // Header info for a single SAC file
  WindowSettings sac_header_settings{1, 25, 285, 730, false};
  // Plot of a single sac file (time-series only)
  WindowSettings sac_1c_plot_settings{287, 25, 1150, 340, false};
  // Plot real/imag spectrum of SAC file
  WindowSettings sac_1c_spectrum_plot_settings{288, 368, 1150, 340, false};
  // List of sac_1c's, allows user to select specific one in memory
  WindowSettings sac_vector_settings{287, 709, 347, 135, false};
  // Small window providing access to lowpass filter options
  WindowSettings sac_lp_options_settings{508, 297, 231, 120, false};
  // Small window providing access to highpass filter options
  WindowSettings sac_hp_options_settings{508, 297, 231, 120, false};
  // Small window providing access to bandpass filter options
  WindowSettings sac_bp_options_settings{508, 297, 276, 148, false};
  // File Dialog positions
  WindowSettings file_dialog_settings{337, 150, 750, 450};
};
// Settings for menu options
// Defines is a menu option is enabled or disabled (separate from whether
// a window is shown or hidden)
struct AllMenuSettings
{
  bool save_sac_1c{false};
  bool welcome{true};
  bool fps{true};
  bool sac_header{false};
  bool sac_1c_plot{false};
  bool sac_1c_spectrum_plot{false};
  bool sac_vector{false};
  bool sac_lp_options{false};
  bool sac_hp_options{false};
  bool sac_bp_options{false};
  bool sac_br_options{false};
  bool undo{false};
  bool redo{false};
  bool rmean{false};
  bool rtrend{false};
};
// Struct for handling fps tracking info
struct fps_info
{ 
  float prev_time{0.0f};
  float current_time{0.f};
  float current_interval{0.f};
  int frame_count{0};
  float fps{0.0f};
  // How often we update the fps tracker
  float reporting_interval{0.2f};
  std::mutex fps_mutex{};
};
// Struct for holding 1-component sac data
struct sac_1c
{
  std::string file_name{};
  SAC::SacStream sac{};
  std::shared_mutex sac_mutex{};

  sac_1c() : file_name(), sac(), sac_mutex() {}
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
struct filter_options
{
  // Filter order
  int order{1};
  // Limits on order
  int min_order{1};
  int max_order{10};
  // Limits on filter frequencies
  float min_freq{0.0f};
  // max_freq is Nyquist (here just an arbitrary value)
  float max_freq{10.0f};
  // Two freqs for bandpass
  // If using lowpass use freq_low
  float freq_low{1.0f};
  // If using highpass use freq_high
  float freq_high{5.0f};
  // Keyboard step interval
  float freq_step{0.1f};
};
//-----------------------------------------------------------------------------
// End custom structs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Misc functions
//-----------------------------------------------------------------------------
void update_fps(fps_info& fps, ImGuiIO& io)
{
  // Lock the fps_tracker
  std::lock_guard<std::mutex> guard(fps.fps_mutex);
  // Not using the lock_guard here
  // Increase time
  fps.current_time += io.DeltaTime;
  // Update the interval
  fps.current_interval = fps.current_time - fps.prev_time;
  // Increase frame_count
  ++fps.frame_count;
}

void cleanup_sac(std::vector<sac_1c>& sac_vector, int& selected, bool& clear)
{
  if (clear)
  {
    --selected;
    sac_vector.erase(sac_vector.begin() + selected + 1);
    if (selected < 0 && sac_vector.size() > 0)
    {
      selected = 0;
    }
    clear = false;
  }
}

void calc_spectrum(sac_1c& sac, sac_1c& spectrum)
{
  spectrum.sac_mutex.lock();
  sac.sac_mutex.lock();
  spectrum.sac = sac.sac;
  spectrum.file_name = sac.file_name;
  sac.sac_mutex.unlock();
  // Calculate the FFT
  SAC::fft_real_imaginary(spectrum.sac);
  spectrum.sac_mutex.unlock();
}

void remove_mean(sac_1c& sac)
{
  sac.sac_mutex.lock();
  double mean{0};
  // Check if the mean is already set
  if (sac.sac.depmen != SAC::unset_float)
  {
    mean = sac.sac.depmen;
  }
  else
  {
    // Need to calculate the mean...
     for (int i{0}; i < sac.sac.npts; ++i)
    {
      mean += sac.sac.data1[0];
    }
    mean /= static_cast<double>(sac.sac.npts);
  }
  // If out mean is zero, then we're done
  if (mean != 0.0 || sac.sac.depmen != 0.0f)
  {
    // Subtract the mean from ever data point
    for (int i{0}; i < sac.sac.npts; ++i)
    {
      sac.sac.data1[i] -= mean;
    }
    // The mean is zero
    sac.sac.depmen = 0.0f;
  }
  sac.sac_mutex.unlock();
}

void remove_trend(sac_1c& sac)
{
  sac.sac_mutex.lock();
  double mean_amplitude{0};
  // Static_cast just to be sure no funny business
  double mean_t{static_cast<double>(sac.sac.npts) / 2.0};
  // If depmen is not set, so no average to start from
  if (sac.sac.depmen == SAC::unset_float)
  {
    for (int i{0}; i < sac.sac.npts; ++i)
    {
      mean_amplitude += sac.sac.data1[0];
    }
    mean_amplitude /= static_cast<double>(sac.sac.npts);
  }
  else
  {
    mean_amplitude = sac.sac.depmen;
  }
  // Now to calculate the slope and y-intercept (y = amplitude)
  double numerator{0};
  double denominator{0};
  double t_diff{0};
  for (int i{0}; i < sac.sac.npts; ++i)
  {
    t_diff = i - mean_t;
    numerator += t_diff * (sac.sac.data1[i] - mean_amplitude);
    denominator += t_diff * t_diff;
  }
  double slope{numerator / denominator};
  double amplitude_intercept{mean_amplitude - (slope * mean_t)};
  // Now to remove the linear trend from the data
  for (int i{0}; i < sac.sac.npts; ++i)
  {
    sac.sac.data1[i] -= (slope * i) + amplitude_intercept;
  }
  sac.sac_mutex.unlock();
}
//-----------------------------------------------------------------------------
// End Misc functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Graphical Backend functions
//-----------------------------------------------------------------------------
// Setup the graphicaly libraries, figure out version info depending on OS
const char* setup_gl()
{
  // Unfortunately it seems that we need to use preprocessor macros to handle this
  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char* glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
  return glsl_version;
}
// Start the graphical backends, create ImGui and ImPlot contexts and get the ImGuiIO
// stuff (Fonts, other things I'm sure)
ImGuiIO& start_graphics(GLFWwindow* window, const char* glsl_version)
{
  if (window == nullptr)
  {
    std::abort();
  }
  // Maximize the window
  glfwMaximizeWindow(window);
  glfwMakeContextCurrent(window);
  // Turn on VSync (or off)
  glfwSwapInterval(1);
  // Create ImGui and ImPlot contexts
  ImGui::CreateContext();
  ImPlot::CreateContext();
  // Get the ImGui IO stuff (fonts and what-not)
  ImGuiIO& io = ImGui::GetIO();
  (void) io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  //----------------------------------------------------------------------
  // Resize fonts
  //----------------------------------------------------------------------
  // Using FontGlobalScale makes for blurry fonts
  //io.FontGlobalScale = 1.f;
  // Create a new font with a larger size
  ImFontAtlas* font_atlas = io.Fonts;
  ImFontConfig font_cfg;
  font_cfg.SizePixels = 18;
  ImFont* font = font_atlas->AddFontDefault(&font_cfg);
  io.FontDefault = font;
  //----------------------------------------------------------------------
  // End Resize fonts
  //----------------------------------------------------------------------
  // Dark mode FTW
  ImGui::StyleColorsDark();
  // Setup ImGui to use the GLFW and OpenGL backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
  return io;
}
// Cleanly destroy everything, performed just before program ends
void end_graphics(GLFWwindow* window)
{
  // Kill the GLFW and OpenGL backends
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  // Destroy the Contexts (free memory)
  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  // Destroy the window
  glfwDestroyWindow(window);
  glfwTerminate();
}
//-----------------------------------------------------------------------------
// End Graphical Backend functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// General GUI functions
//-----------------------------------------------------------------------------
// Helper program for errors with glfw
void glfw_error_callback(int error, const char *description)
{
  std::cerr << "GLFW Error " << error << ": " << description << '\n'; 
}
// Ran at beginning of new frame draw cycle
void prep_newframe()
{
  // Check for user input (mouse, keyboard, etc)
  glfwPollEvents();
  // Tell the backends to prepare for a new frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  // Tell Dear ImGui to prepare for a new frame
  ImGui::NewFrame();
}
// Ran at end of new frame draw cycle
void finish_newframe(GLFWwindow* window, ImVec4 clear_color)
{
  // Draw the update
  ImGui::Render();
  // OpenGl/GLFW frame buffer size
  // Tells us how large the encompassing window is
  // so that everything drawn gets scaled correctly
  int gl_display_w{};
  int gl_display_h{};
  // Check the frame buffer size (in case user resized the window)
  glfwGetFramebufferSize(window, &gl_display_w, &gl_display_h);
  // Pass it to opengl
  glViewport(0, 0, gl_display_w, gl_display_h);
  // Set color to clear screen
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
  // Clear the buffer
  glClear(GL_COLOR_BUFFER_BIT);
  // Tell the backend to render ImGui's update
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  // There are two buffers (back and front)
  // Swap to the newly drawn buffer
  glfwSwapBuffers(window);
}
//-----------------------------------------------------------------------------
// End General GUI functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// UI Windows
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------
// Status Bar
//------------------------------------------------------------------------
void status_bar(const char* status_message = "Idle", float progress = 0.0f)
{
  // Size and position
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
  ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetTextLineHeightWithSpacing() + (ImGui::GetStyle().FramePadding.y * 2.0f) + 10));
  ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - ImGui::GetTextLineHeightWithSpacing() - (ImGui::GetStyle().FramePadding.y * 2.0f) - 10));
  // Start the status bar
  ImGui::Begin("Status", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
  // Add status message
  ImGui::Text("%s", status_message);
  // Draw progress bar
  if (progress >= 0.0f && progress <= 1.0f)
  {
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100.0f);
    ImGui::ProgressBar(progress, ImVec2(100.0f, ImGui::GetTextLineHeight()));
  }
  ImGui::End();
  ImGui::PopStyleVar();
}
//------------------------------------------------------------------------
// End Status Bar
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Lowpass Filter Options Window
//------------------------------------------------------------------------
void window_lowpass_options(WindowSettings& window_settings, filter_options& lowpass_settings, sac_1c& sac, sac_1c& spectrum)
{
  if (window_settings.show)
  {
    if (!window_settings.is_set)
    {
      ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
      ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
      window_settings.is_set = true;
    }

    ImGui::Begin("Lowpass Options", &window_settings.show, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav);
    lowpass_settings.max_freq = static_cast<float>(0.5 / sac.sac.delta); // Nyquist
    ImGui::SetNextItemWidth(130);
    if (ImGui::InputFloat("Freq (Hz)", &lowpass_settings.freq_low, lowpass_settings.freq_step))
    {
      lowpass_settings.freq_low = std::clamp(lowpass_settings.freq_low, lowpass_settings.min_freq, lowpass_settings.max_freq);
    }
    ImGui::SetNextItemWidth(130);
    if (ImGui::InputInt("Order", &lowpass_settings.order))
    {
      lowpass_settings.order = std::clamp(lowpass_settings.order, lowpass_settings.min_order, lowpass_settings.max_order);
    }
    if (ImGui::Button("Ok"))
    {
      sac.sac_mutex.lock();
      SAC::lowpass(sac.sac, lowpass_settings.order, lowpass_settings.freq_low);
      sac.sac_mutex.unlock();
      calc_spectrum(sac, spectrum);
      window_settings.show = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
      window_settings.show = false;
    }
    ImGui::End();
  }
}
//------------------------------------------------------------------------
// End lowpass Filter Options Window
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Highpass Filter Options Window
//------------------------------------------------------------------------
void window_highpass_options(WindowSettings& window_settings, filter_options& highpass_settings, sac_1c& sac, sac_1c& spectrum)
{
  if (window_settings.show)
  {
    if (!window_settings.is_set)
    {
      ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
      ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
      window_settings.is_set = true;
    }

    ImGui::Begin("Highpass Options", &window_settings.show, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav);
    highpass_settings.max_freq = static_cast<float>(0.5 / sac.sac.delta); // Nyquist
    ImGui::SetNextItemWidth(130);
    if (ImGui::InputFloat("Freq (Hz)", &highpass_settings.freq_low, highpass_settings.freq_step))
    {
      highpass_settings.freq_low = std::clamp(highpass_settings.freq_low, highpass_settings.min_freq, highpass_settings.max_freq);
    }
    ImGui::SetNextItemWidth(130);
    if (ImGui::InputInt("Order", &highpass_settings.order))
    {
      highpass_settings.order = std::clamp(highpass_settings.order, highpass_settings.min_order, highpass_settings.max_order);
    }
    if (ImGui::Button("Ok"))
    {
      sac.sac_mutex.lock();
      SAC::highpass(sac.sac, highpass_settings.order, highpass_settings.freq_low);
      sac.sac_mutex.unlock();
      calc_spectrum(sac, spectrum);
      window_settings.show = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
      window_settings.show = false;
    }
    ImGui::End();
  }
}
//------------------------------------------------------------------------
// End highpass Filter Options Window
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Bandpass Filter Options Window
//------------------------------------------------------------------------
void window_bandpass_options(WindowSettings& window_settings, filter_options& bandpass_settings, sac_1c& sac, sac_1c& spectrum)
{
  if (window_settings.show)
  {
    if (!window_settings.is_set)
    {
      ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
      ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
      window_settings.is_set = true;
    }

    ImGui::Begin("Bandpass Options", &window_settings.show, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav);
    bandpass_settings.max_freq = static_cast<float>(0.5 / sac.sac.delta); // Nyquist
    ImGui::SetNextItemWidth(130);
    if (ImGui::InputFloat("Min Freq (Hz)", &bandpass_settings.freq_low, bandpass_settings.freq_step))
    {
      bandpass_settings.freq_low = std::clamp(bandpass_settings.freq_low, bandpass_settings.min_freq, bandpass_settings.max_freq);
    }
    ImGui::SetNextItemWidth(130);
    if (ImGui::InputFloat("Max Freq (Hz)", &bandpass_settings.freq_high, bandpass_settings.freq_step))
    {
      bandpass_settings.freq_high = std::clamp(bandpass_settings.freq_high, bandpass_settings.min_freq, bandpass_settings.max_freq);
    }
    ImGui::SetNextItemWidth(130);
    if (ImGui::InputInt("Order", &bandpass_settings.order))
    {
      bandpass_settings.order = std::clamp(bandpass_settings.order, bandpass_settings.min_order, bandpass_settings.max_order);
    }
    if (ImGui::Button("Ok"))
    {
      sac.sac_mutex.lock();
      SAC::bandpass(sac.sac, bandpass_settings.order, bandpass_settings.freq_low, bandpass_settings.freq_high);
      sac.sac_mutex.unlock();
      calc_spectrum(sac, spectrum);
      window_settings.show = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
      window_settings.show = false;
    }
    ImGui::End();
  }
}
//------------------------------------------------------------------------
// End bandpass Filter Options Window
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Main menu bar
//------------------------------------------------------------------------
void main_menu_bar(GLFWwindow* window, AllWindowSettings& allwindow_settings, AllMenuSettings& am_settings, ProgramStatus& program_status, std::vector<sac_1c>& sac_vector, int& active_sac)
{
  // Just to get rid of unused for now...
  (void) program_status;
  sac_1c sac{};
  ImGui::BeginMainMenuBar();
  // File menu
  if (ImGui::BeginMenu("File"))
  {
    if (ImGui::MenuItem("Open 1C"))
    {
      ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".SAC,.sac", ".", ImGuiFileDialogFlags_Modal);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Read a single SAC-file");
    }
    if (ImGui::MenuItem("Open Dir"))
    {
      ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey", "Choose Directory", nullptr, ".", ImGuiFileDialogFlags_Modal);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Read a directory full of SAC-files");
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Save 1C", nullptr, nullptr, am_settings.save_sac_1c))
    {
      ImGuiFileDialog::Instance()->OpenDialog("SaveFileDlgKey", "Save File", ".SAC,.sac", ".", ImGuiFileDialogFlags_Modal);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Save a single SAC-file");
    }
    if (ImGui::MenuItem("Exit"))
    {
      glfwSetWindowShouldClose(window, true);
    }
    ImGui::EndMenu();
  }
  // Edit menu
  if (ImGui::BeginMenu("Edit"))
  {
    if (ImGui::MenuItem("Undo", nullptr, nullptr, am_settings.undo))
    {
      // To be implemented at some point
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Not implemented");
    }
    if (ImGui::MenuItem("Redo", nullptr, nullptr, am_settings.redo))
    {
      // TO be implemented at some point
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Not implemented");
    }
    ImGui::EndMenu();
  }
  // Project Menu
  // Eventually projects will be supported
  // That will involve keeping track of files, actions, etc.
  // Place-holder for now as a promise and reminder of my intentions
  if (ImGui::BeginMenu("Project"))
  {
    ImGui::EndMenu();
  }
  // Options Menu
  // Changing fonts, their sizes, etc.
  if (ImGui::BeginMenu("Options"))
  {
    ImGui::EndMenu();
  }
  // Window menu
  if (ImGui::BeginMenu("Window"))
  {
    // Bring all windows to the center incase the layout got borked
    if (ImGui::MenuItem("Center Windows"))
    {
      // To be implemented at some point
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Center ALL windows. Not yet implemented");
    }
    // Change the default layout, if the user wants that
    if (ImGui::MenuItem("Save Layout"))
    {
      // To be implemented at some point
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Save current window layout as new default. Not yet implemented");
    }
    // Reset window positions incase something got lost
    if (ImGui::MenuItem("Reset Windows"))
    {
      allwindow_settings.welcome_settings.is_set = false;
      allwindow_settings.fps_settings.is_set = false;
      allwindow_settings.sac_header_settings.is_set = false;
      allwindow_settings.sac_1c_plot_settings.is_set = false;
      allwindow_settings.sac_1c_spectrum_plot_settings.is_set = false;
      allwindow_settings.sac_vector_settings.is_set = false;
      allwindow_settings.sac_lp_options_settings.is_set = false;
      allwindow_settings.sac_hp_options_settings.is_set = false;
      allwindow_settings.sac_bp_options_settings.is_set = false;
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Reset all windows to default position and size");
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Welcome", nullptr, nullptr, am_settings.welcome))
    {
      allwindow_settings.welcome_settings.show = true;
    }
    if (ImGui::MenuItem("FPS Tracker", nullptr, nullptr, am_settings.fps))
    {
      allwindow_settings.fps_settings.show = true;
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Frames Per Second display");
    }
    if (ImGui::MenuItem("Sac Header", nullptr, nullptr, am_settings.sac_header))
    {
      allwindow_settings.sac_header_settings.show = true;
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Displays SAC header values");
    }
    if (ImGui::MenuItem("Sac Plot 1C", nullptr, nullptr, am_settings.sac_1c_plot))
    {
      allwindow_settings.sac_1c_plot_settings.show = true;
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("1-component SAC plot");
    }
    if (ImGui::MenuItem("Spectrum Plot 1C", nullptr, nullptr, am_settings.sac_1c_spectrum_plot))
    {
      allwindow_settings.sac_1c_spectrum_plot_settings.show = true;
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("1-component SAC spectrogram (real/imaginary) plot");
    }
    if (ImGui::MenuItem("Sac List", nullptr, nullptr, am_settings.sac_vector))
    {
      allwindow_settings.sac_vector_settings.show = true;
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("List of SAC files currently loaded in memory");
    }
    ImGui::EndMenu();
  }
  // Open File Dialog (single SAC)
  ImVec2 maxSize = ImVec2(allwindow_settings.file_dialog_settings.width * 1.5, allwindow_settings.file_dialog_settings.height * 1.5);
  ImVec2 minSize = ImVec2(maxSize.x * 0.75f, maxSize.y * 0.75f);
  if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
  {
    // Read the SAC-File safely
    if (ImGuiFileDialog::Instance()->IsOk())
    {
      sac.sac_mutex.lock();
      sac.file_name = ImGuiFileDialog::Instance()->GetFilePathName();
      sac.sac = SAC::SacStream(sac.file_name);
      // Add it to the list!
      sac_vector.push_back(sac);
      // We should show the sac header window after loading a sac file (not before)
      allwindow_settings.sac_header_settings.show = true;
      allwindow_settings.sac_1c_plot_settings.show = true;
      allwindow_settings.sac_vector_settings.show = true;
      allwindow_settings.sac_1c_spectrum_plot_settings.show = true;
      sac.sac_mutex.unlock();
    }
    ImGuiFileDialog::Instance()->Close();
  }
  if (ImGuiFileDialog::Instance()->Display("ChooseDirDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
  {
    if (ImGuiFileDialog::Instance()->IsOk())
    {
      std::filesystem::path directory = ImGuiFileDialog::Instance()->GetFilePathName();
      // Iterate over files in directory
      program_status.program_mutex.lock();
      for (const auto& entry : std::filesystem::directory_iterator(directory))
      {
        // Check extension
        if (entry.path().extension() == ".sac" || entry.path().extension() == ".SAC")
        {
          program_status.fileio.file_vector.push_back(entry.path().string());
        }
        program_status.fileio.to_read = true;
        program_status.fileio.count = 0;
        program_status.program_mutex.unlock();
      }
    }
    ImGuiFileDialog::Instance()->Close();
  }
  // Save file dialog (single sac)
  if (ImGuiFileDialog::Instance()->Display("SaveFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
  {
    // Save the SAC-File safely
    if (ImGuiFileDialog::Instance()->IsOk())
    {
      sac_vector[active_sac].sac_mutex.lock();
      sac_vector[active_sac].sac.write(ImGuiFileDialog::Instance()->GetFilePathName());
      sac_vector[active_sac].sac_mutex.unlock();
    }
    ImGuiFileDialog::Instance()->Close();
  }
  if (ImGui::BeginMenu("Processing"))
  {
    if (ImGui::MenuItem("Remove Mean", nullptr, nullptr, am_settings.rmean))
    {
      remove_mean(sac_vector[active_sac]);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Remove mean value from active data.");
    }
    if (ImGui::MenuItem("Remove Trend", nullptr, nullptr, am_settings.rtrend))
    {
      remove_trend(sac_vector[active_sac]);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Remove linear trend from active data.");
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Lowpass", nullptr, nullptr, am_settings.sac_lp_options))
    {
      allwindow_settings.sac_lp_options_settings.show = true;
      allwindow_settings.sac_hp_options_settings.show = false;
      allwindow_settings.sac_bp_options_settings.show = false;
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Butterworth Lowpass filter active data.");
    }
    if (ImGui::MenuItem("Highpass", nullptr, nullptr, am_settings.sac_hp_options))
    {
      allwindow_settings.sac_lp_options_settings.show = false;
      allwindow_settings.sac_hp_options_settings.show = true;
      allwindow_settings.sac_bp_options_settings.show = false;
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Butterworth Highpass filter active data.");
    }
    if (ImGui::MenuItem("Bandpass", nullptr, nullptr, am_settings.sac_bp_options))
    {
      allwindow_settings.sac_lp_options_settings.show = false;
      allwindow_settings.sac_hp_options_settings.show = false;
      allwindow_settings.sac_bp_options_settings.show = true;
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Butterworth Bandpass filter active data.");
    }
    if (ImGui::MenuItem("Bandreject", nullptr, nullptr, am_settings.sac_br_options))
    {
      // To be implemented later
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Butterworth Bandreject filter active data. Not implemented");
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("Picking"))
  {
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("Batch"))
  {
    if (ImGui::MenuItem("Remove Mean", nullptr, nullptr, am_settings.rmean))
    {
      program_status.program_mutex.lock();
      program_status.message = "Removing Mean...";
      for (std::size_t i{0}; i < sac_vector.size(); ++i)
      {
        remove_mean(sac_vector[i]);
        program_status.progress = static_cast<float>(i) / static_cast<float>(sac_vector.size());
      }
      program_status.message = "Idle";
      program_status.progress = 0.0f;
      program_status.program_mutex.unlock();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Remove mean value from all data.");
    }
    if (ImGui::MenuItem("Remove Trend", nullptr, nullptr, am_settings.rtrend))
    {
      for (std::size_t i{0}; i < sac_vector.size(); ++i)
      {
        remove_trend(sac_vector[i]);
      }
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Remove trend value from all data.");
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Lowpass", nullptr, nullptr, am_settings.sac_lp_options))
    {
      // To be implemented later
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Butterworth Lowpass filter all data. Not implemented");
    }
    if (ImGui::MenuItem("Highpass", nullptr, nullptr, am_settings.sac_hp_options))
    {
      // To be implemented later
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Butterworth Highpass filter all data. Not implemented");
    }
    if (ImGui::MenuItem("Bandpass", nullptr, nullptr, am_settings.sac_bp_options))
    {
      // To be implemented later
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Butterworth Bandpass filter all data. Not implemented");
    }
    if (ImGui::MenuItem("Bandreject", nullptr, nullptr, am_settings.sac_br_options))
    {
      // To be implemented later
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
    {
      ImGui::SetTooltip("Butterworth Bandreject filter all data. Not implemented");
    }
    ImGui::EndMenu();
  }
  ImGui::EndMainMenuBar();
}
//------------------------------------------------------------------------
// End Main menu bar
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// 1-component SAC plot window
//------------------------------------------------------------------------
void window_plot_sac(WindowSettings& window_settings, std::vector<sac_1c>& sac_vector, int& selected)
{
  if (window_settings.show)
  {
    if (!window_settings.is_set)
    {
      ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
      ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
      window_settings.is_set = true;
    }
    ImGui::Begin("Sac Plot", &window_settings.show);
    if (ImPlot::BeginPlot("Seismogram"))
    {
      ImPlot::SetupAxis(ImAxis_X1, "Time (s)"); // Move this line here
      sac_vector[selected].sac_mutex.lock_shared();
      ImPlot::PlotLine("", &sac_vector[selected].sac.data1[0], sac_vector[selected].sac.data1.size(), sac_vector[selected].sac.delta);
      sac_vector[selected].sac_mutex.unlock_shared();
      // This allows us to add a separate context menu inside the plot area that appears upon double left-clicking
      // Right-clicking is reserved for the built in context menu (have not figured out how to add to it without
      // directly modifying ImPlot, which I don't want to do)
      ImPlotContext* plot_ctx = ImPlot::GetCurrentContext();
      if (plot_ctx && ImPlot::IsPlotHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
      {
        // Oddly, BeginPopupContextItem doesn't seem to do the job here, so we must use the based functions
        ImGui::OpenPopup("##CustomPlotOptions");
      }
      if (ImGui::BeginPopup("##CustomPlotOptions"))
      {
        if (ImGui::BeginMenu("Test"))
        {
          if (ImGui::MenuItem("Custom 1"))
          {
          }
          ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Test 2"))
        {
          if (ImGui::MenuItem("Custom 2"))
          {
          }
          if (ImGui::MenuItem("Custom 3"))
          {
          }
          ImGui::EndMenu();
        }
        ImGui::EndPopup();
      }
      ImPlot::EndPlot();
    }
    ImGui::End();
  }
}

//------------------------------------------------------------------------
// End 1-component SAC plot window
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// 1-component SAC spectrum window
//------------------------------------------------------------------------
void window_plot_spectrum(WindowSettings& window_settings, sac_1c& spectrum)
{
  if (window_settings.show)
  {
    if (!window_settings.is_set)
    {
      ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
      ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
      window_settings.is_set = true;
    }
    ImGui::Begin("Spectrum", &window_settings.show);
    ImGui::Columns(2);
    if (ImPlot::BeginPlot("Real"))
    {
      spectrum.sac_mutex.lock_shared();
      ImPlot::SetupAxis(ImAxis_X1, "Freq (Hz)");
      const double sampling_freq{1.0 / spectrum.sac.delta};
      const double freq_step{sampling_freq / spectrum.sac.npts};
      ImPlot::PlotLine("", &spectrum.sac.data1[0], spectrum.sac.data1.size() / 2, freq_step);
      spectrum.sac_mutex.unlock_shared();
      ImPlot::EndPlot();
    }
    ImGui::NextColumn();
    if (ImPlot::BeginPlot("Imaginary"))
    {
      spectrum.sac_mutex.lock_shared();
      ImPlot::SetupAxis(ImAxis_X1, "Freq (Hz)");
      const double sampling_freq{1.0 / spectrum.sac.delta};
      const double freq_step{sampling_freq / spectrum.sac.npts};
      ImPlot::PlotLine("", &spectrum.sac.data2[0], spectrum.sac.data2.size() / 2, freq_step);
      spectrum.sac_mutex.unlock_shared();
      ImPlot::EndPlot();
    }
    ImGui::Columns(1);
    ImGui::End();
  }
}
//------------------------------------------------------------------------
// End 1-component SAC spectrum window
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// 1-component SAC header window
//------------------------------------------------------------------------
void window_sac_header(WindowSettings& window_settings, sac_1c& sac)
{
  if (window_settings.show)
  {
    if (!window_settings.is_set)
    {
      ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
      ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
      window_settings.is_set = true;
    }

    ImGui::Begin("Sac Header", &window_settings.show, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav);

    sac.sac_mutex.lock_shared();
    if (ImGui::CollapsingHeader("Station Information", ImGuiTreeNodeFlags_DefaultOpen))
    {
      ImGui::Text("Network:    %s", sac.sac.knetwk.c_str());
      ImGui::Text("Station:    %s", sac.sac.kstnm.c_str());
      ImGui::Text("Instrument: %s", sac.sac.kinst.c_str());
      ImGui::Text("Latitude:   %.2f\u00B0N", sac.sac.stla);
      ImGui::Text("Longitude:  %.2f\u00B0E", sac.sac.stlo);
      ImGui::Text("Elevation:  %.2f m", sac.sac.stel);
      ImGui::Text("Depth:      %.2f m", sac.sac.stdp);
      ImGui::Text("Back Azi:   %.2f\u00B0", sac.sac.baz);
    }
    if (ImGui::CollapsingHeader("Component Information", ImGuiTreeNodeFlags_DefaultOpen))
    {
      ImGui::Text("Component:  %s", sac.sac.kcmpnm.c_str());
      ImGui::Text("Azimuth:    %.2f\u00B0", sac.sac.cmpaz);
      ImGui::Text("Incidence:  %.2f\u00B0", sac.sac.cmpinc);
    }
    if (ImGui::CollapsingHeader("Event Information", ImGuiTreeNodeFlags_DefaultOpen))
    {
      ImGui::Text("Name:       %s", sac.sac.kevnm.c_str());
      ImGui::Text("Latitude:   %.2f\u00B0N", sac.sac.evla);
      ImGui::Text("Longitude:  %.2f\u00B0E", sac.sac.evlo);
      ImGui::Text("Depth:      %.2f km", sac.sac.evdp);
      ImGui::Text("Magnitude:  %.2f", sac.sac.mag);
      ImGui::Text("Azimuth:    %.2f\u00B0", sac.sac.az);
    }
    if (ImGui::CollapsingHeader("DateTime Information", ImGuiTreeNodeFlags_DefaultOpen))
    {
      ImGui::Text("Year:       %i", sac.sac.nzyear);
      ImGui::Text("Julian Day: %i", sac.sac.nzjday);
      ImGui::Text("Hour:       %i", sac.sac.nzhour);
      ImGui::Text("Minute:     %i", sac.sac.nzmin);
      ImGui::Text("Second:     %i", sac.sac.nzsec);
      ImGui::Text("MSecond:    %i", sac.sac.nzmsec);
    }
    if (ImGui::CollapsingHeader("Data Information", ImGuiTreeNodeFlags_DefaultOpen))
    {
      ImGui::Text("Npts:       %i", sac.sac.npts);
      ImGui::Text("IfType:     %i", sac.sac.iftype);
    }
    sac.sac_mutex.unlock_shared();
    ImGui::End();
  }
}
//------------------------------------------------------------------------
// End 1-component SAC header window
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Welcome window
//------------------------------------------------------------------------
void window_welcome(WindowSettings& window_settings, std::string_view& welcome_message)
{
  if (window_settings.show)
  {
    if (!window_settings.is_set)
    {
      ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
      ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
      window_settings.is_set = true;
    }
    ImGui::Begin(" ", &window_settings.show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav);
    ImGui::TextUnformatted(welcome_message.data());
    ImGui::End();
  }
}
//------------------------------------------------------------------------
// End Welcome window
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// FPS window
//------------------------------------------------------------------------
// Creates a small window to show the FPS of the program, pretty much setup
void window_fps(fps_info& fps_tracker, WindowSettings& window_settings)
{
  if (window_settings.show)
  {
    std::lock_guard<std::mutex> guard(fps_tracker.fps_mutex);
    if (!window_settings.is_set)
    {
      // Setup the window
      ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
      ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
      window_settings.is_set = true;
      // Reset the fps_tracker
      fps_tracker.prev_time = 0.0f;
      fps_tracker.frame_count = 0;
      fps_tracker.current_time = 0;
    }
    if (fps_tracker.current_interval >= fps_tracker.reporting_interval)
    {
      fps_tracker.fps = static_cast<float>(fps_tracker.frame_count) / fps_tracker.current_interval;
      fps_tracker.frame_count = 0;
      fps_tracker.current_interval = 0;
      fps_tracker.prev_time = fps_tracker.current_time;
    }
    ImGui::Begin("FPS", &window_settings.show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav);
    ImGui::Text("%i", static_cast<int>(fps_tracker.fps));
    ImGui::End();
  }
}
//------------------------------------------------------------------------
// End FPS window
//------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SAC-loaded window
//-----------------------------------------------------------------------------
void window_sac_vector(AllWindowSettings& aw_settings, AllMenuSettings& am_settings, std::vector<sac_1c>& sac_vector, sac_1c& spectrum, int& selected, bool& cleared)
{
  WindowSettings& window_settings = aw_settings.sac_vector_settings;
  std::string option{};
  if (window_settings.show)
  {
    if (!window_settings.is_set)
    {
      // Setup the window
      ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
      ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
      window_settings.is_set = true;
    }
    ImGui::Begin("Sac List", &window_settings.show);
    for (int i = 0; const auto& sac : sac_vector)
    {
      const bool is_selected{selected == i};
      option = sac.file_name.substr(sac.file_name.find_last_of("\\/") + 1);
      if (ImGui::Selectable(option.c_str(), is_selected))
      {
        selected = i;
      }
      // Right-click menu
      if (ImGui::BeginPopupContextItem((std::string("Context Menu##") + std::to_string(i)).c_str()))
      {
        if (ImGui::MenuItem("Save", nullptr, nullptr, am_settings.save_sac_1c))
        {
          selected = i;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        {
          ImGui::SetTooltip("Save SAC file. Not implemented in this context. Use File->Save 1C");
        }
        if (ImGui::MenuItem("Remove"))
        {
          selected = i;
          cleared = true;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        {
          ImGui::SetTooltip("Unload SAC data from memory");
        }
        if (ImGui::MenuItem("Reload"))
        {
          selected = i;
          sac_vector[selected].sac_mutex.lock();
          sac_vector[selected].sac = SAC::SacStream(sac_vector[selected].file_name);
          sac_vector[selected].sac_mutex.unlock();
          calc_spectrum(sac_vector[selected], spectrum);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        {
          ImGui::SetTooltip("Reload the original SAC file");
        }
        if (ImGui::MenuItem("LowPass", nullptr, nullptr, am_settings.sac_lp_options))
        {
          selected = i;
          aw_settings.sac_lp_options_settings.show = true;
          aw_settings.sac_hp_options_settings.show = false;
          aw_settings.sac_bp_options_settings.show = false;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        {
          ImGui::SetTooltip("Butterworth lowpass filter");
        }
        if (ImGui::MenuItem("HighPass", nullptr, nullptr, am_settings.sac_hp_options))
        {
          selected = i;
          aw_settings.sac_lp_options_settings.show = false;
          aw_settings.sac_hp_options_settings.show = true;
          aw_settings.sac_bp_options_settings.show = false;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        {
          ImGui::SetTooltip("Butterworth highpass filter");
        }
        if (ImGui::MenuItem("BandPass", nullptr, nullptr, am_settings.sac_bp_options))
        {
          selected = i;
          aw_settings.sac_lp_options_settings.show = false;
          aw_settings.sac_hp_options_settings.show = false;
          aw_settings.sac_bp_options_settings.show = true;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        {
         ImGui::SetTooltip("Butterworth bandpass filter");
        }
        ImGui::EndPopup();
      }
      ++i;
    }

    ImGui::End();
  }
}

//-----------------------------------------------------------------------------
// End SAC-loaded window
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// End UI Windows
//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// End pssp namespace
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
int main()
{
  //---------------------------------------------------------------------------
  // Initialization
  //---------------------------------------------------------------------------
  // Check to make sure GLFW can startup
  glfwSetErrorCallback(pssp::glfw_error_callback);
  if (!glfwInit())
  {
    std::abort();
  }
  // Setup the graphics library and get the version
  const char* glsl_version = pssp::setup_gl();
  // Setup the GLFW window
  GLFWwindow* window = glfwCreateWindow(1024, 720, "Passive-source Seismic-processing", nullptr, nullptr);
  // Start the graphics backends and create the ImGui and ImPlot contexts
  ImGuiIO& io = pssp::start_graphics(window, glsl_version);
  //---------------------------------------------------------------------------
  // End Initialization
  //---------------------------------------------------------------------------

  //---------------------------------------------------------------------------
  // Misc Draw loop variables
  //---------------------------------------------------------------------------
  // Default color for clearing the screen
  ImVec4 clear_color = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
  pssp::fps_info fps_tracker{};
  std::string_view welcome_message{"Welcome to Passive-source Seismic-processing (PsSP)!"};
  pssp::AllWindowSettings aw_settings{};
  pssp::AllMenuSettings am_settings{};
  pssp::filter_options lowpass_settings{};
  pssp::filter_options highpass_settings{};
  pssp::filter_options bandpass_settings{};
  pssp::ProgramStatus program_status{};
  // Time-series
  std::vector<pssp::sac_1c> sac_vector;
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
    // Do we need to remove a sac_1c from the sac_vector?
    cleanup_sac(sac_vector, active_sac, clear_sac);
    // Start the frame
    pssp::prep_newframe();
    pssp::status_bar(program_status.message.c_str(), program_status.progress);
    // Testing the progress bar in the main loop since it doesn't fucking work anywhere else
    // It works here, we'll need to move the file reading either to a separate thread
    // or limit outselves to one file were frame at most...
    if (program_status.fileio.to_read)
    {
      program_status.program_mutex.lock();
      if (!program_status.fileio.started)
      {
        program_status.message = "Reading SAC files...";
        program_status.progress = 0.0f;
        program_status.fileio.total = static_cast<int>(program_status.fileio.file_vector.size());
        program_status.fileio.started = true;
      }
      if (program_status.fileio.count < program_status.fileio.total)
      {
        pssp::sac_1c sac{};
        sac.sac_mutex.lock();
        sac.file_name = program_status.fileio.file_vector[program_status.fileio.count];
        sac.sac = SAC::SacStream(sac.file_name);
        sac_vector.push_back(sac);
        sac.sac_mutex.unlock();
        ++program_status.fileio.count;
        program_status.progress = static_cast<float>(program_status.fileio.count) / static_cast<float>(program_status.fileio.total);
      }
      else
      {
        program_status.progress = 1.1f;
        program_status.message = "Idle";
        program_status.fileio.to_read = false;
        program_status.fileio.started = false;
        aw_settings.sac_header_settings.show = true;
        aw_settings.sac_1c_plot_settings.show = true;
        aw_settings.sac_vector_settings.show = true;
        aw_settings.sac_1c_spectrum_plot_settings.show = true;
      }
      program_status.program_mutex.unlock();
    }
    pssp::main_menu_bar(window, aw_settings, am_settings, program_status, sac_vector, active_sac);
    // Show the Welcome window if appropriate
    pssp::window_welcome(aw_settings.welcome_settings, welcome_message);
    pssp::update_fps(fps_tracker, io);
    // Show the FPS window if appropriate
    pssp::window_fps(fps_tracker, aw_settings.fps_settings);
    // We don't want to show any of these windows if there are now sac files loaded in
    // (That would involve accessing memory that doesn't exist and crash)
    if (sac_vector.size() > 0)
    {
      // Allow menu options that require sac files
      am_settings.save_sac_1c = true;
      am_settings.sac_vector = true;
      am_settings.sac_header = true;
      am_settings.sac_1c_plot = true;
      am_settings.sac_1c_spectrum_plot = true;
      am_settings.sac_lp_options = true;
      am_settings.sac_hp_options = true;
      am_settings.sac_bp_options = true;
      am_settings.rmean = true;
      am_settings.rtrend = true;
      // This fixes the issue of deleting all sac_1cs in the vector
      // loading new ones, and then trying to access the -1 element
      if (active_sac < 0)
      {
        active_sac = 0;
      }
      pssp::window_sac_header(aw_settings.sac_header_settings, sac_vector[active_sac]);
      // Show the Sac Plot window if appropriate
      pssp::window_plot_sac(aw_settings.sac_1c_plot_settings, sac_vector, active_sac);
      // Show the Sac Spectrum window if appropriate
      // We need to see if the FFT needs to be calculated (don't want to do it
      // every frame)
      if (aw_settings.sac_1c_spectrum_plot_settings.show)
      {
        // If they're not the same, then calculate the FFT
        if (spectrum.file_name != sac_vector[active_sac].file_name)
        {
          pssp::calc_spectrum(sac_vector[active_sac], spectrum);
        }
      }
      // Finally plot the spectrum
      pssp::window_plot_spectrum(aw_settings.sac_1c_spectrum_plot_settings, spectrum);
      // Show the Sac List window if appropriate
      pssp::window_sac_vector(aw_settings, am_settings, sac_vector, spectrum, active_sac, clear_sac);
      pssp::window_lowpass_options(aw_settings.sac_lp_options_settings, lowpass_settings, sac_vector[active_sac], spectrum);
      pssp::window_highpass_options(aw_settings.sac_hp_options_settings, highpass_settings, sac_vector[active_sac], spectrum);
      pssp::window_bandpass_options(aw_settings.sac_bp_options_settings, bandpass_settings, sac_vector[active_sac], spectrum);
    }
    else
    {
      // Disallow menu options that require sac files
      am_settings.save_sac_1c = false;
      am_settings.sac_vector = false;
      am_settings.sac_header = false;
      am_settings.sac_1c_plot = false;
      am_settings.sac_1c_spectrum_plot = false;
      am_settings.sac_lp_options = false;
      am_settings.sac_hp_options = false;
      am_settings.sac_bp_options = false;
      am_settings.rmean = false;
      am_settings.rtrend = false;
      spectrum.file_name = "";
    }
    // Finish the frame
    pssp::finish_newframe(window, clear_color);
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
