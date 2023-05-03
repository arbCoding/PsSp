
#include "sac_stream.hpp"

// Dear ImGui headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// OS-Independent file dialogs
#include "tinyfiledialogs.h"

#include <iostream>
#include <string>

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
};

std::string open_file_dialog()
{
  const char* filters[] = {"*.SAC"};
  const char* selected_file = tinyfd_openFileDialog("Open File", "", 5, filters, NULL, 0);
  std::string file_string{};
  if (selected_file)
  {
    file_string = selected_file;
  }
  return file_string;
}

// Standard file menu
static void file_menu(GLFWwindow* window, SAC::SacStream& sac)
{
  if (ImGui::BeginMenu("File"))
  {
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open"))
    {
      std::string file_name = open_file_dialog();
      if (file_name != "")
      {
        // Read the file
        sac = SAC::SacStream(file_name);
      }
    }
    if (ImGui::MenuItem("Exit"))
    {
      glfwSetWindowShouldClose(window, true);
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
static void main_menu_bar(GLFWwindow* window, SAC::SacStream& sac)
{
  ImGui::BeginMainMenuBar();
  file_menu(window, sac);
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

// Info window
static void info_window(WindowSettings* w_settings, SAC::SacStream& sac)
{
  if (!w_settings->set)
  {
    ImGui::SetNextWindowSize(ImVec2(w_settings->width, w_settings->height));
    ImGui::SetNextWindowPos(ImVec2(w_settings->x, w_settings->y));
    w_settings->set = true;
  }
  ImGui::Begin("Sac Info", nullptr);
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

  main_menu_bar(window, sac);
  main_window();
  info_window(w_settings, sac);

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
  ImGuiIO& io = ImGui::GetIO();
  (void) io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // Make all 25% bigger (MBP screen)
  io.FontGlobalScale = 1.25f;

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
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
