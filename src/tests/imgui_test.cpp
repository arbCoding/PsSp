
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

std::string open_file_dialog() {
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
static void file_menu(GLFWwindow* window)
{
  if (ImGui::BeginMenu("File"))
  {
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open"))
    {
      std::string file_name = open_file_dialog();
      if (file_name != "")
      {
        SAC::SacStream sac(file_name);
        std::cout << sac.kstnm << '\n';
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
static void main_menu_bar(GLFWwindow* window)
{
  ImGui::BeginMainMenuBar();
  file_menu(window);
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

// The draw cycle for the external (containing) window
static void draw_cycle(GLFWwindow* window, ImVec4 clear_color)
{
  glfwPollEvents();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  main_menu_bar(window);
  main_window();

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

  while (!glfwWindowShouldClose(window))
  {
    draw_cycle(window, clear_color);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
