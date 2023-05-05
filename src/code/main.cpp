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

// Silence OpenGL deprecation warnings on compile
#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>

// std::cout, std::cerr
#include <iostream>
// mutex locks
#include <mutex>
// std::string_view
#include <string>

// Helper program for errors with glfw
static void glfw_error_callback(int error, const char *description)
{
  std::cerr << "GLFW Error " << error << ": " << description << '\n'; 
}

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

// For each window we want to know
// Where it should be placed (pos_x/y)
// What size it should be (width/height)
// If it should be shown or not
// And if the window is set (size/position/etc)
struct WindowSettings
{
  int pos_x{};
  int pos_y{};
  int width{};
  int height{};
  bool show{true};
  bool is_set{false};
};

struct AllWindowSettings
{
  WindowSettings welcome_settings{400, 200, 500, 250};
  WindowSettings fps_settings{10, 30, 60, 55};
  WindowSettings sac_header_settings{1000, 200, 250, 500, false};
};

AllWindowSettings aw_settings{};

struct fps_info
{
  // FPS tracking
  float prev_time{0.0f};
  int frame_count{0};
  float fps{0.0f};
  float current_time{0.f};
  float reporting_interval{0.1f};
  float current_interval{0.f};
};


// Function that handles the main menu bar
static void main_menu_bar(GLFWwindow* window, AllWindowSettings& allwindow_settings, SAC::SacStream& sac_file, std::mutex& sac_mutex)
{
  ImGui::BeginMainMenuBar();
  // File menu
  if (ImGui::BeginMenu("File"))
  {
    if (ImGui::MenuItem("Open"))
    {
      // Nothing happens with this yet...
      ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".SAC,.sac", ".");
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
      sac_mutex.lock();
      sac_file = SAC::SacStream(ImGuiFileDialog::Instance()->GetFilePathName());
      sac_mutex.unlock();
      // We should show the sac header window after loading a sac file (not before)
      allwindow_settings.sac_header_settings.show = true;
    }
    ImGuiFileDialog::Instance()->Close();
  }
  ImGui::EndMainMenuBar();
}

void window_sac_header(WindowSettings& window_settings, SAC::SacStream& sac, std::mutex& sac_mutex)
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
    sac_mutex.lock();
    ImGui::Text("Station Information");
    ImGui::Text("StNm: %s", sac.kstnm.c_str());
    ImGui::Text("CmpNm: %s", sac.kcmpnm.c_str());
    ImGui::Text("Stla: %.2f\u00B0N", sac.stla);
    ImGui::Text("Stlo: %.2f\u00B0E", sac.stlo);
    ImGui::Text("Stel: %.2f m", sac.stel);
    ImGui::Text("Event Information");
    ImGui::Text("EvNm: %s", sac.kevnm.c_str());
    ImGui::Text("Evla: %.2f\u00B0N", sac.evla);
    ImGui::Text("Evlo: %.2f\u00B0E", sac.evlo);
    ImGui::Text("Evdp: %.2f km", sac.evdp);
    sac_mutex.unlock();
    ImGui::End();
  }
}

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
    ImGui::Begin("Welcome", &window_settings.show);
    ImGui::TextUnformatted(welcome_message.data());
    ImGui::End();
  }
}

// Creates a small window to show the FPS of the program, pretty much setup
void window_fps(fps_info& fps_tracker, WindowSettings& window_settings, std::mutex& fps_mutex)
{
  if (window_settings.show)
  {
    if (!window_settings.is_set)
    {
      // Setup the window
      ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
      ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
      window_settings.is_set = true;
      // Reset the fps_tracker
      fps_mutex.lock();
      fps_tracker.prev_time = 0.0f;
      fps_tracker.frame_count = 0;
      fps_tracker.current_time = 0;
      fps_mutex.unlock();
    }
    if (fps_tracker.current_interval >= fps_tracker.reporting_interval)
    {
      fps_mutex.lock();
      fps_tracker.fps = static_cast<float>(fps_tracker.frame_count) / fps_tracker.current_interval;
      fps_tracker.frame_count = 0;
      fps_tracker.current_interval = 0;
      fps_tracker.prev_time = fps_tracker.current_time;
      fps_mutex.unlock();
    }
    ImGui::Begin("FPS", &window_settings.show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("%i", static_cast<int>(fps_tracker.fps));
    ImGui::End();
  }
}

int main()
{
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
  {
    std::abort();
  }

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

  // Setup the GLFW window
  GLFWwindow* window = glfwCreateWindow(1024, 720, "Passive-source Seismic-processing", nullptr, nullptr);
  if (window == nullptr)
  {
    return 1;
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

  // Default color for clearing the screen
  ImVec4 clear_color = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
  fps_info fps_tracker{};
  std::mutex fps_mutex{};

  std::string_view welcome_message{"Welcome to Passive-source Seismic-processing (PsSP)!"};

  SAC::SacStream sac{};
  std::mutex sac_mutex{};

  // The draw loop, this is ran EVERY FRAME
  // so be careful to make the stuff in here safe
  // (minimize spurious work, be thread-safe, etc.)
  while (!glfwWindowShouldClose(window))
  {
    // Start the frame
    prep_newframe();
    main_menu_bar(window, aw_settings, sac, sac_mutex);
    // Lock the fps_tracker
    fps_mutex.lock();
    // Increase time
    fps_tracker.current_time += io.DeltaTime;
    // Update the interval
    fps_tracker.current_interval = fps_tracker.current_time - fps_tracker.prev_time;
    // Increase frame_count
    ++fps_tracker.frame_count;
    fps_mutex.unlock();
    // Show the Welcome window if appropriate
    window_welcome(aw_settings.welcome_settings, welcome_message);
    // Show the FPS window if appropriate
    window_fps(fps_tracker, aw_settings.fps_settings, fps_mutex);
    // Show the Sac Header window if appropriate
    window_sac_header(aw_settings.sac_header_settings, sac, sac_mutex);
    // Finish the frame
    finish_newframe(window, clear_color);
  }

  // Kill the GLFW and OpenGL backends
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  // Destroy the Contexts (free memory)
  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  // Destroy the window
  glfwDestroyWindow(window);
  glfwTerminate();

  // End program
  return 0;
}
