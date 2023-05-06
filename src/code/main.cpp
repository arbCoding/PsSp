// Silence OpenGL deprecation warnings on compile
#define GL_SILENCE_DEPRECATION
//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
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
// std::string_view
#include <string>
// Path stuff
#include <filesystem>
// std::list (thread-safe version of vector)
#include <list>
//-----------------------------------------------------------------------------
// End include statements
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Custom structs
//-----------------------------------------------------------------------------
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

struct AllWindowSettings
{
  // Window with welcome message
  WindowSettings welcome_settings{400, 200, 475, 60};
  // FPS tracker window
  WindowSettings fps_settings{5, 25, 60, 55, false};
  // Header info for a single SAC file
  WindowSettings sac_header_settings{1160, 25, 275, 500, false};
  // Plot of a single sac file (time-series only)
  WindowSettings sac_1c_plot_settings{5, 25, 1150, 350, false};
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
  std::string file_dir{};
  std::string file_name{};
  SAC::SacStream sac{};
  std::mutex sac_mutex{};
};
//-----------------------------------------------------------------------------
// End custom structs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Graphical Backend functions
//-----------------------------------------------------------------------------
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

  ImGuiIO& io = ImGui::GetIO();
  (void) io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // Make all 25% bigger (MBP screen)
  io.FontGlobalScale = 1.25f;
  // Dark mode FTW
  ImGui::StyleColorsDark();
  // Setup ImGui to use the GLFW and OpenGL backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
  return io;
}

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
static void glfw_error_callback(int error, const char *description)
{
  std::cerr << "GLFW Error " << error << ": " << description << '\n'; 
}

// Ran at beginning of new frame draw cycle
static void prep_newframe()
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
static void finish_newframe(GLFWwindow* window, ImVec4 clear_color)
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
// Main menu bar
//------------------------------------------------------------------------
// Function that handles the main menu bar. Preparing to handle 3C sac data soon
// I'll do a list of sac_1c structs, file_dir will be redundant, but I don't care at the moment.
static void main_menu_bar(GLFWwindow* window, AllWindowSettings& allwindow_settings, sac_1c& sac)
{
  ImGui::BeginMainMenuBar();
  // File menu
  if (ImGui::BeginMenu("File"))
  {
    if (ImGui::MenuItem("Open 1C"))
    {
      if (sac.file_dir != "")
      {
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".SAC,.sac", sac.file_dir.c_str());
      }
      else
      {
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".SAC,.sac", ".");
      }
    }
    if (ImGui::MenuItem("Open Dir"))
    {

    }
    if (ImGui::MenuItem("Exit"))
    {
      glfwSetWindowShouldClose(window, true);
    }
    ImGui::EndMenu();
  }
  // Window menu
  if (ImGui::BeginMenu("Window"))
  {
    if (ImGui::MenuItem("Welcome"))
    {
      allwindow_settings.welcome_settings.show = !allwindow_settings.welcome_settings.show;
    }
    if (ImGui::MenuItem("FPS Tracker"))
    {
      allwindow_settings.fps_settings.show = !allwindow_settings.fps_settings.show;
    }
    if (ImGui::MenuItem("Sac Header"))
    {
      allwindow_settings.sac_header_settings.show = !allwindow_settings.sac_header_settings.show;
    }
    if (ImGui::MenuItem("Sac Plot 1C"))
    {
      allwindow_settings.sac_1c_plot_settings.show = !allwindow_settings.sac_1c_plot_settings.show;
    }
    ImGui::EndMenu();
  }
  // File Dialog
  ImVec2 maxSize = ImVec2(1000, 600);
  ImVec2 minSize = ImVec2(maxSize.x * 0.75f, maxSize.y * 0.75f);
  if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
  {
    // Read the SAC-File safely
    if (ImGuiFileDialog::Instance()->IsOk())
    {
      // lock_guard unlocks at end of scope (end of function)
      std::lock_guard<std::mutex> guard(sac.sac_mutex);
      sac.file_name = ImGuiFileDialog::Instance()->GetFilePathName();
      sac.file_dir = sac.file_name.substr(0, sac.file_name.find_last_of("\\/")) + '/';
      sac.sac = SAC::SacStream(sac.file_name);
      // We should show the sac header window after loading a sac file (not before)
      allwindow_settings.sac_header_settings.show = true;
      allwindow_settings.sac_1c_plot_settings.show = true;
    }
    ImGuiFileDialog::Instance()->Close();
  }
  ImGui::EndMainMenuBar();
}
//------------------------------------------------------------------------
// End Main menu bar
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// 1-component SAC plot window
//------------------------------------------------------------------------
void window_plot_sac(WindowSettings& window_settings, sac_1c& sac)
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
    if (ImGui::BeginChild("Sac Plot"))
    {
      if(ImPlot::BeginPlot(sac.sac.kstnm.c_str()))
      {
        std::lock_guard<std::mutex> guard(sac.sac_mutex);
        ImPlot::PlotLine(sac.sac.kcmpnm.c_str(), &sac.sac.data1[0], sac.sac.data1.size());
        ImPlot::EndPlot();
      }
      ImGui::EndChild();
    }
    ImGui::End();
  }
}
//------------------------------------------------------------------------
// End 1-component SAC plot window
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

    ImGui::Begin("Sac Header", &window_settings.show);
    std::lock_guard<std::mutex> guard(sac.sac_mutex);
    if (ImGui::CollapsingHeader("Station Information"))
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
    if (ImGui::CollapsingHeader("Component Information"))
    {
      ImGui::Text("Component:  %s", sac.sac.kcmpnm.c_str());
      ImGui::Text("Azimuth:    %.2f\u00B0", sac.sac.cmpaz);
      ImGui::Text("Incident:   %.2f\u00B0", sac.sac.cmpinc);
    }
    if (ImGui::CollapsingHeader("Event Information"))
    {
      ImGui::Text("Name:       %s", sac.sac.kevnm.c_str());
      ImGui::Text("Latitude:   %.2f\u00B0N", sac.sac.evla);
      ImGui::Text("Longitude:  %.2f\u00B0E", sac.sac.evlo);
      ImGui::Text("Depth:      %.2f km", sac.sac.evdp);
      ImGui::Text("Magnitude:  %.2f", sac.sac.mag);
      ImGui::Text("Azimuth:    %.2f\u00B0", sac.sac.az);
    }
    if (ImGui::CollapsingHeader("DateTime Information"))
    {
      ImGui::Text("Year:       %i", sac.sac.nzyear);
      ImGui::Text("Julian Day: %i", sac.sac.nzjday);
      ImGui::Text("Hour:       %i", sac.sac.nzhour);
      ImGui::Text("Minute:     %i", sac.sac.nzmin);
      ImGui::Text("Second:     %i", sac.sac.nzsec);
      ImGui::Text("MSecond:    %i", sac.sac.nzmsec);
    }
    if (ImGui::CollapsingHeader("Data Information"))
    {
      ImGui::Text("Npts:       %i", sac.sac.npts);
      ImGui::Text("IfType:     %i", sac.sac.iftype);
    }
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
    ImGui::Begin(" ", &window_settings.show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoResize);
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
    ImGui::Begin("FPS", &window_settings.show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("%i", static_cast<int>(fps_tracker.fps));
    ImGui::End();
  }
}
//------------------------------------------------------------------------
// End FPS window
//------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// End UI Windows
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
//-----------------------------------------------------------------------------
// End Misc functions
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
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
  {
    std::abort();
  }
  // Setup the graphics library and get the version
  const char* glsl_version = setup_gl();
  // Setup the GLFW window
  GLFWwindow* window = glfwCreateWindow(1024, 720, "Passive-source Seismic-processing", nullptr, nullptr);
  // Start the graphics backends and create the ImGui and ImPlot contexts
  ImGuiIO& io = start_graphics(window, glsl_version);
  //---------------------------------------------------------------------------
  // End Initialization
  //---------------------------------------------------------------------------

  //---------------------------------------------------------------------------
  // Misc Draw loop variables
  //---------------------------------------------------------------------------
  // Default color for clearing the screen
  ImVec4 clear_color = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
  fps_info fps_tracker{};
  std::string_view welcome_message{"Welcome to Passive-source Seismic-processing (PsSP)!"};
  sac_1c sac{};
  AllWindowSettings aw_settings{};
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
    // Start the frame
    prep_newframe();
    main_menu_bar(window, aw_settings, sac);
    // Show the Welcome window if appropriate
    window_welcome(aw_settings.welcome_settings, welcome_message);
    update_fps(fps_tracker, io);
    // Show the FPS window if appropriate
    window_fps(fps_tracker, aw_settings.fps_settings);
    // Show the Sac Header window if appropriate
    window_sac_header(aw_settings.sac_header_settings, sac);
    // Show the Sac Plot window if appropriate
    window_plot_sac(aw_settings.sac_1c_plot_settings, sac);
    // Finish the frame
    finish_newframe(window, clear_color);
  }
  //---------------------------------------------------------------------------
  // End draw loop
  //---------------------------------------------------------------------------
  end_graphics(window);
  // End program
  return 0;
}
//-----------------------------------------------------------------------------
// End Main
//-----------------------------------------------------------------------------
