#include <sac_stream.hpp>

// Dear ImGui headers
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// File dialog header
#include <ImGuiFileDialog.h>

// ImPlot headers
#include <implot.h>

// Standard Library Stuff
#include <algorithm> // std::min_element
#include <iostream> // std::cout
#include <string> // std::string
#include <vector> // std::vector
#include <mutex> // std::mutex

// Silence OpenGL deprecation warnings on compile
#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>

struct WindowSettings
{
  int x{};
  int y{};
  int width{};
  int height{};
  bool set{false};
  bool show{true};
};

struct PlotSettings
{
  int xmin{};
  int xmax{};
  int ymin{};
  int ymax{};
  bool reset_limits{true};
};


// Define global mutex
std::mutex data_mutex;

// Standard file menu
static void file_menu(GLFWwindow* window, SAC::SacStream& sac)
{
  if (ImGui::BeginMenu("File"))
  {
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open"))
    {
      ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".SAC,.sac", ".");
    }
    if (ImGui::MenuItem("Exit"))
    {
      glfwSetWindowShouldClose(window, true);
    }
    ImGui::EndMenu();
  }

  ImVec2 maxSize = ImVec2(1000, 600);
  ImVec2 minSize = ImVec2(maxSize.x * 0.75f, maxSize.y * 0.75f);
  if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
  {
    if (ImGuiFileDialog::Instance()->IsOk())
    {
      // Lock the data while it is being read in, unlock when finished
      data_mutex.lock();
      sac = SAC::SacStream(ImGuiFileDialog::Instance()->GetFilePathName());
      data_mutex.unlock();
    }
    ImGuiFileDialog::Instance()->Close();
  }
}

// Window menu
static void window_menu(WindowSettings* w_settings)
{
  if (ImGui::BeginMenu("Window"))
  {
    if (ImGui::MenuItem("Sac Info"))
    {
      w_settings->show = true;
    }
    ImGui::EndMenu();
  }
}

// Menu to help functions
static void help_menu()
{
  if (ImGui::BeginMenu("Help"))
  {
    ImGui::EndMenu();
  }
}

// Function that handles the main menu bar
static void main_menu_bar(GLFWwindow* window, WindowSettings* w_settings, SAC::SacStream& sac)
{
  ImGui::BeginMainMenuBar();
  file_menu(window, sac);
  window_menu(w_settings);
  help_menu();
  ImGui::EndMainMenuBar();
}


// This handles setting up the main internal window
static void main_window(float fps)
{
  // Set size of next window
  ImGui::SetNextWindowSize(ImVec2(500, 400));

  // Begin parent window
  ImGui::Begin("Main", nullptr);
  ImGui::Text("Welcome!");
  ImGui::Text("FPS: %i", static_cast<int>(fps));
  // End parent window
  ImGui::End();
}

static void plot_window(WindowSettings* w_settings, PlotSettings* p_settings, const SAC::SacStream& sac)
{
  if (!w_settings->set)
  {
    ImGui::SetNextWindowSize(ImVec2(w_settings->width, w_settings->height));
    ImGui::SetNextWindowPos(ImVec2(w_settings->x, w_settings->y));
    w_settings->set = true;
  }
  // Set minimum and maximum window size constraints
  ImGui::SetNextWindowSizeConstraints(ImVec2(500, 400), ImVec2(1000, 600));
  ImGui::Begin("Sac Plot", &(w_settings->show), ImGuiWindowFlags_NoCollapse);
  ImPlot::BeginPlot("Test Plot");
  // Lock the data before checking it
  data_mutex.lock();
  if (sac.npts != -12345)
  {
    // Reset limits upon loading in first bit of data
    if (p_settings->reset_limits)
    {
      auto minmax = std::minmax_element(sac.data1.begin(), sac.data1.end());
      p_settings->xmin = -10;
      p_settings->xmax = sac.data1.size();
      p_settings->ymin = *minmax.first;
      p_settings->ymax = *minmax.second;
      ImPlot::SetupAxisLimits(ImAxis_X1, p_settings->xmin, p_settings->xmax, ImPlotCond_Always);  // Set limits for the x-axis
      ImPlot::SetupAxisLimits(ImAxis_Y1, p_settings->ymin, p_settings->ymax, ImPlotCond_Always); // Set limits for the y-axis
      p_settings->reset_limits = false;
    }
    ImPlot::PlotLine("", &sac.data1[0], sac.data1.size());
  }
  // Release the lock on the data
  data_mutex.unlock();
  ImPlot::EndPlot();
  ImGui::End();
}

// Info window
static void info_window(WindowSettings* w_settings, const SAC::SacStream& sac)
{
  if (!w_settings->set)
  {
    ImGui::SetNextWindowSize(ImVec2(w_settings->width, w_settings->height));
    ImGui::SetNextWindowPos(ImVec2(w_settings->x, w_settings->y));
    w_settings->set = true;
  }
  ImGui::Begin("Sac Info", &(w_settings->show), ImGuiWindowFlags_NoCollapse);
  ImGui::Text("Station Name: %s", sac.kstnm.c_str());
  ImGui::Text("Component: %s", sac.kcmpnm.c_str());
  ImGui::Text("Station Lat: %f", sac.stla);
  ImGui::Text("Station Lon: %f", sac.stlo);
  ImGui::Text("Station Elv: %f", sac.stel);
  ImGui::Text("Event Lat: %f", sac.evla);
  ImGui::Text("Event Lon: %f", sac.evlo);
  ImGui::Text("Event Depth: %f", sac.evdp);
  ImGui::Text("N Points: %i", sac.npts);
  if (sac.npts != -12345)
  {
    ImGui::Text("data[0]: %f", sac.data1[0]);
  }
  else
  {
    ImGui::Text("data[0]: -12345.0");
  }
  ImGui::End();
}

static void draw_cycle(GLFWwindow* window, ImVec4 clear_color, WindowSettings* w_settings, PlotSettings* p_settings, SAC::SacStream& sac, float fps)
{
  glfwPollEvents();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  main_menu_bar(window, w_settings, sac);
  main_window(fps);
  if (w_settings->show)
  {
    info_window(w_settings, sac);
    plot_window(w_settings, p_settings, sac); 
  }

  ImGui::Render();
  int display_w{};
  int display_h{};
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(window);
}

// Helper program for errors with glfw
static void glfw_error_callback(int error, const char *description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main()
{

  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
        abort();

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

  GLFWwindow* window = glfwCreateWindow(1024, 720, "Seismic CPP", nullptr, nullptr);
  if (window == nullptr)
  {
    return 1;
  }

  // Maximize the window
  glfwMaximizeWindow(window);
  glfwMakeContextCurrent(window);
  // Turn on VSync (or off)
  glfwSwapInterval(0);

  ImGui::CreateContext();
  // The issue appears to be related to creating and destroying the ImPlot context throughout the run
  // when there are no optimizations active on the compiler
  // From lldb exploration, the break occurs when this calls ImPlotContext* ctx = IM_NEW(ImPlotContext)();
  // It seems to to have issues with how it access memory...
  ImPlot::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  (void) io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // Make all 25% bigger (MBP screen)
  io.FontGlobalScale = 1.25f;
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".SAC", ImVec4(1.0f, 0.0f, 0.0f, 0.9f));
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".sac", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));

  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  ImVec4 clear_color = ImVec4(0.4f, 0.4f, 0.4f, 1.f);

  SAC::SacStream sac{};
  WindowSettings w_settings{1000, 100, 250, 500};
  PlotSettings p_settings{};
  // FPS tracking
  float prev_time{0.0f};
  int frame_count{0};
  float fps{0.0f};
  float current_time{0};
  
  while (!glfwWindowShouldClose(window))
  {
    draw_cycle(window, clear_color, &w_settings, &p_settings, sac, fps);
    current_time += io.DeltaTime;
    ++frame_count;
    if (current_time - prev_time >= 0.025f)
    {
      fps = static_cast<float>(frame_count) / (current_time - prev_time);
      frame_count = 0;
      prev_time = current_time;
    }
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
