#ifndef PSSP_WINDOWS_HPP_20230610
#define PSSP_WINDOWS_HPP_20230610

//-----------------------------------------------------------------------------
// Include statments
//-----------------------------------------------------------------------------
// Necessary structs from pssp_projects
#include "pssp_data_pool.hpp"
#include "pssp_program_settings.hpp"
#include "pssp_misc.hpp"
#include "pssp_projects.hpp"
// Dear ImGui
#include <imgui.h>
// Dear ImGui C++ textinput wrappers
#include <misc/cpp/imgui_stdlib.h>
// FileDialog add-on for Dear ImGui
#include <ImGuiFileDialog.h>
// ImPlot add-on for Dear ImGui
#include <implot.h>
#include <deque>
#include <filesystem>
//-----------------------------------------------------------------------------
// End Include statements
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Begin pssp namespace
//-----------------------------------------------------------------------------
namespace pssp
{
//-----------------------------------------------------------------------------
// UI Windows
//-----------------------------------------------------------------------------
// Status bar along bottom of screen
void status_bar(ProgramStatus& program_status);
// Window to allow user to select options for their lowpass filter
void window_lowpass_options(WindowSettings& window_settings, FilterOptions& lowpass_settings);
// Window to allow user to select options for their highpass filter
void window_highpass_options(WindowSettings& window_settings, FilterOptions& highpass_settings);
// Window to allow user to select options for their bandpass filter
void window_bandpass_options(WindowSettings& window_settings, FilterOptions& bandpass_settings);
// Main menu bar along top of screen
void main_menu_bar(GLFWwindow* window, AllWindowSettings& allwindow_settings, MenuAllowed& menu_allowed,
AllFilterOptions& af_settings, ProgramStatus& program_status, int& active_sac);
// Plot a single sac file
void window_plot_sac(WindowSettings& window_settings, ProgramStatus& program_status, int data_id);
// Plot a single real/imaginary spectrum
void window_plot_spectrum(WindowSettings& window_settings, bool is_project, sac_1c& spectrum);
// Window that displays sac header information
void window_sac_header(WindowSettings& window_settings, ProgramStatus& program_status, int data_id);
// Window that displays welcome message on startup
void window_welcome(WindowSettings& window_settings, std::string_view& welcome_message);
// Window that displays the frame-rate in Frames Per Second
void window_fps(fps_info& fps_tracker, WindowSettings& window_settings);
// Window lists data in project for current checkpoint
void window_data_list(ProgramStatus& program_status, AllWindowSettings& aw_settings, MenuAllowed& menu_allowed, int& selected, bool& clear_sac);
// Window to name a checkpoint
void window_name_checkpoint(WindowSettings& window_settings, ProgramStatus& program_status);
// Window to make notes for a checkpoint
void window_notes_checkpoint(WindowSettings& window_settings, Project& project);
// Window to show processing history for the active_sac
void window_processing_history(WindowSettings& window_settings, Project& project, int data_id);
//-----------------------------------------------------------------------------
// End UI Windows
//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// End pssp namespace
//-----------------------------------------------------------------------------
#endif
