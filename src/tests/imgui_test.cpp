#include "sac_stream.hpp"

// Dear ImGui headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// File dialog header
#include "ImGuiFileDialog.h"

// ImPlot headers
#include "implot.h"
#include "implot_internal.h"

#include <iostream>
#include <string>
#include <vector>

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
      sac = SAC::SacStream(ImGuiFileDialog::Instance()->GetFilePathName());
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
static void main_window()
{
  // Set size of next window
  ImGui::SetNextWindowSize(ImVec2(500, 400));

  // Begin parent window
  ImGui::Begin("Main", nullptr);
  ImGui::Text("Welcome!");
  // End parent window
  ImGui::End();
}

static void plot_window(WindowSettings* w_settings, SAC::SacStream& sac)
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
  if (sac.npts != -12345)
  {
    if (ImPlot::BeginPlot("Test Plot"))
    {
      // Trying to plot without giving the y-values
      ImPlot::PlotLine("", sac.data1.data(), sac.data1.size());
      ImPlot::EndPlot();
    }
    ImGui::Text("Plot here");
  }
  ImGui::End();
}

// Info window
static void info_window(WindowSettings* w_settings, SAC::SacStream& sac)
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

// The draw cycle for the external (containing) window
static void draw_cycle(GLFWwindow* window, ImVec4 clear_color, WindowSettings* w_settings, SAC::SacStream& sac)
{
  glfwPollEvents();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  main_menu_bar(window, w_settings, sac);
  main_window();
  if (w_settings->show)
  {
    info_window(w_settings, sac);
    plot_window(w_settings, sac); 
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

int main()
{
  if (!glfwInit())
  {
    return 1;
  }

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
  // Turn on VSync
  glfwSwapInterval(1);

  ImGui::CreateContext();
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
  while (!glfwWindowShouldClose(window))
  {
    draw_cycle(window, clear_color, &w_settings, sac);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
