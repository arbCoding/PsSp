#ifndef PSSP_WINDOWS_HPP
#define PSSP_WINDOWS_HPP

//-----------------------------------------------------------------------------
// Include statments
//-----------------------------------------------------------------------------
// Necessary structs from pssp_projects
#include "pssp_program_settings.hpp"
#include "pssp_misc.hpp"
#include "pssp_projects.hpp"
// Dear ImGui
#include <imgui.h>
// FileDialog add-on for Dear ImGui
#include <ImGuiFileDialog.h>
// ImPlot add-on for Dear ImGui
#include <implot.h>
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
void window_lowpass_options(ProgramStatus& program_status, WindowSettings& window_settings, FilterOptions& lowpass_settings);
// Window to allow user to select options for their highpass filter
void window_highpass_options(ProgramStatus& program_status, WindowSettings& window_settings, FilterOptions& highpass_settings);
// Window to allow user to select options for their bandpass filter
void window_bandpass_options(ProgramStatus& program_status, WindowSettings& window_settings, FilterOptions& bandpass_settings);
// Main menu bar along top of screen
void main_menu_bar(GLFWwindow* window, AllWindowSettings& allwindow_settings, MenuAllowed& menu_allowed,
AllFilterOptions& af_settings, ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, int& active_sac, Project& project);
// Plot a single sac file
void window_plot_sac(WindowSettings& window_settings, std::deque<sac_1c>& sac_deque, int& selected);
// Plot a single real/imaginary spectrum
void window_plot_spectrum(WindowSettings& window_settings, sac_1c& spectrum);
// Window that displays sac header information
void window_sac_header(ProgramStatus& program_status, WindowSettings& window_settings, sac_1c& sac);
// Window that displays welcome message on startup
void window_welcome(WindowSettings& window_settings, std::string_view& welcome_message);
// Window that displays the frame-rate in Frames Per Second
void window_fps(fps_info& fps_tracker, WindowSettings& window_settings);
// Window lists data currently in memory
void window_sac_deque(AllWindowSettings& aw_settings, MenuAllowed& menu_allowed,
ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, sac_1c& spectrum, int& selected, bool& cleared);
//-----------------------------------------------------------------------------
// End UI Windows
//-----------------------------------------------------------------------------
};
//-----------------------------------------------------------------------------
// End pssp namespace
//-----------------------------------------------------------------------------
#endif