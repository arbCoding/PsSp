#include "pssp_windows.hpp"

//-----------------------------------------------------------------------------
// Status Bar
//-----------------------------------------------------------------------------
void pssp::status_bar(ProgramStatus& program_status)
{
    // Size and position
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetTextLineHeightWithSpacing() + (ImGui::GetStyle().FramePadding.y * 2.0f) + 10));
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - ImGui::GetTextLineHeightWithSpacing() - (ImGui::GetStyle().FramePadding.y * 2.0f) - 10));
    //----------------------------------------------------------------------
    // Status of program (message and progress)
    //----------------------------------------------------------------------
    // If we're not reading or shutting down, we're idle
    std::string status_message{""};
    {
        std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
        program_status.is_idle = (!program_status.fileio.is_reading && !program_status.fileio.is_processing);
        if (program_status.is_idle)
        {
            status_message = "Idle";
            program_status.progress = 1.1f;
        }
        else
        {
            program_status.progress = static_cast<float>(program_status.fileio.count) / static_cast<float>(program_status.fileio.total);
            if (program_status.progress >= 1.0f)
            {
                std::lock_guard<std::shared_mutex> lock_io(program_status.fileio.io_mutex);
                program_status.is_idle = true;
                program_status.fileio.is_reading = false;
                program_status.fileio.is_processing = false;
                program_status.fileio.total = 0;
            }
            else if (program_status.fileio.is_reading) { status_message = "Reading SAC files..."; }
            else if (program_status.fileio.is_processing) { status_message = "Processing..."; }
        }
    }
    //----------------------------------------------------------------------
    // End Status of program (message and progress)
    //----------------------------------------------------------------------
    std::ostringstream oss{};
    oss << "Threads (Busy/Total): " << program_status.thread_pool.n_busy_threads()
    << '/' << program_status.thread_pool.n_threads_total();

    // Start the status bar
    ImGui::Begin("Status##", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav);
    // Add status message, this on the left of the bar
    ImGui::Text("%s", status_message.c_str());
    // Add information about running threads, this is in the middle of the bar
    ImGui::SameLine((ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(oss.str().c_str()).x) / 2.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x);
    ImGui::Text("%s", oss.str().c_str());
    // Draw progress bar
    // If below 0 or above 1 it will not draw a progress bar
    // which is super useful for hiding it
    // This is on the right of the bar
    if (program_status.progress >= 0.0f && program_status.progress <= 1.0f)
    {
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100.0f);
    ImGui::ProgressBar(program_status.progress, ImVec2(100.0f, ImGui::GetTextLineHeight()));
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
//-----------------------------------------------------------------------------
// End Status Bar
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Lowpass Filter Options Window
//-----------------------------------------------------------------------------
void pssp::window_lowpass_options(ProgramStatus& program_status, WindowSettings& window_settings, FilterOptions& lowpass_settings)
{
    if (window_settings.show)
    {
        if (!window_settings.is_set)
        {
            ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
            ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
            window_settings.is_set = true;
        }

        ImGui::Begin(window_settings.title.c_str(), &window_settings.show, window_settings.img_flags);
        if (!program_status.is_idle) { ImGui::BeginDisabled(); }
        
        ImGui::SetNextItemWidth(130);
        
        if (ImGui::InputFloat("Freq (Hz)", &lowpass_settings.freq_low, lowpass_settings.freq_step))
        { lowpass_settings.freq_low = std::max(0.0f, lowpass_settings.freq_low); }
        
        ImGui::SetNextItemWidth(130);
        if (ImGui::InputInt("Order##", &lowpass_settings.order))
        { lowpass_settings.order = std::clamp(lowpass_settings.order, lowpass_settings.min_order, lowpass_settings.max_order); }
        
        if (ImGui::Button("Ok##"))
        {
            lowpass_settings.apply_filter = true;
            window_settings.show = false;
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel##"))
        {
            lowpass_settings.apply_filter = false;
            lowpass_settings.apply_batch = false;
            window_settings.show = false;
        }
        if (!program_status.is_idle) { ImGui::EndDisabled(); }
        ImGui::End();
    }
}
//-----------------------------------------------------------------------------
// End lowpass Filter Options Window
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Highpass Filter Options Window
//-----------------------------------------------------------------------------
void pssp::window_highpass_options(ProgramStatus& program_status, WindowSettings& window_settings, FilterOptions& highpass_settings)
{
    if (window_settings.show)
    {
        if (!window_settings.is_set)
        {
            ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
            ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
            window_settings.is_set = true;
        }

        ImGui::Begin(window_settings.title.c_str(), &window_settings.show, window_settings.img_flags);
        if (!program_status.is_idle) { ImGui::BeginDisabled(); }
        
        ImGui::SetNextItemWidth(130);
        
        if (ImGui::InputFloat("Freq (Hz)", &highpass_settings.freq_low, highpass_settings.freq_step))
        { highpass_settings.freq_low = std::max(0.0f, highpass_settings.freq_low); }
        
        ImGui::SetNextItemWidth(130);
        if (ImGui::InputInt("Order##", &highpass_settings.order))
        { highpass_settings.order = std::clamp(highpass_settings.order, highpass_settings.min_order, highpass_settings.max_order); }
        
        if (ImGui::Button("Ok##"))
        {
            highpass_settings.apply_filter = true;
            window_settings.show = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel##"))
        {
            highpass_settings.apply_filter = false;
            highpass_settings.apply_batch = false;
            window_settings.show = false;
        }
        if (!program_status.is_idle) { ImGui::EndDisabled(); }
        ImGui::End();
    }
}
//-----------------------------------------------------------------------------
// End highpass Filter Options Window
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Bandpass Filter Options Window
//-----------------------------------------------------------------------------
void pssp::window_bandpass_options(ProgramStatus& program_status, WindowSettings& window_settings, FilterOptions& bandpass_settings)
{
    if (window_settings.show)
    {
        if (!window_settings.is_set)
        {
            ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
            ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
            window_settings.is_set = true;
        }

        ImGui::Begin(window_settings.title.c_str(), &window_settings.show, window_settings.img_flags);
        if (!program_status.is_idle) { ImGui::BeginDisabled(); }
        
        ImGui::SetNextItemWidth(130);
        
        if (ImGui::InputFloat("Min Freq (Hz)", &bandpass_settings.freq_low, bandpass_settings.freq_step))
        { bandpass_settings.freq_low = std::max(0.0f, bandpass_settings.freq_low); }
        
        ImGui::SetNextItemWidth(130);
        
        if (ImGui::InputFloat("Max Freq (Hz)", &bandpass_settings.freq_high, bandpass_settings.freq_step))
        { bandpass_settings.freq_high = std::max(bandpass_settings.freq_low, bandpass_settings.freq_high); }
        
        ImGui::SetNextItemWidth(130);
        
        if (ImGui::InputInt("Order##", &bandpass_settings.order))
        { bandpass_settings.order = std::clamp(bandpass_settings.order, bandpass_settings.min_order, bandpass_settings.max_order); }
        
        if (ImGui::Button("Ok##") && bandpass_settings.freq_low < bandpass_settings.freq_high)
        {
            bandpass_settings.apply_filter = true;
            window_settings.show = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel##"))
        {
            bandpass_settings.apply_filter = false;
            bandpass_settings.apply_batch = false;
            window_settings.show = false;
        }
        if (!program_status.is_idle) { ImGui::EndDisabled(); }
        ImGui::End();
    }
}
//-----------------------------------------------------------------------------
// End bandpass Filter Options Window
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Main menu bar
//-----------------------------------------------------------------------------
void pssp::main_menu_bar(GLFWwindow* window, AllWindowSettings& allwindow_settings, MenuAllowed& menu_allowed,
AllFilterOptions& af_settings, ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, int& active_sac)
{
    sac_1c sac{};
    std::string home_path{};
    // We're going to need to use a preprocessor macro to get the default home directory for the OS being used
#ifdef _WIN32
    const char* user_profile{std::getenv("USERPROFILE")};
    if (user_profile) { home_path = user_profile; home_path += '\\'; }
#else
    const char* home_dir{std::getenv("HOME")};
    if (home_dir) { home_path = home_dir; home_path += '/'; }
#endif

    ImGui::BeginMainMenuBar();
    // File menu
    if (ImGui::BeginMenu("File##", menu_allowed.file_menu))
    {
        if (ImGui::MenuItem("Open 1C##", nullptr, nullptr, menu_allowed.open_1c))
        { ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".SAC,.sac", home_path, ImGuiFileDialogFlags_Modal); }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Read a single SAC-file"); }
        
        if (ImGui::MenuItem("Open Dir##", nullptr, nullptr, menu_allowed.open_dir))
        { ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey", "Choose Directory", nullptr, home_path, ImGuiFileDialogFlags_Modal); }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Read a directory full of SAC-files"); }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Save 1C##", nullptr, nullptr, menu_allowed.save_1c))
        { ImGuiFileDialog::Instance()->OpenDialog("SaveFileDlgKey", "Save File", ".SAC,.sac", home_path, ImGuiFileDialogFlags_Modal); }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Save a single SAC-file"); }
        
        if (ImGui::MenuItem("Exit##", nullptr, nullptr, menu_allowed.exit))
        { glfwSetWindowShouldClose(window, true); }
        
        ImGui::EndMenu();
    }
    // Edit menu
    if (ImGui::BeginMenu("Edit##", menu_allowed.edit_menu))
    {
        if (ImGui::MenuItem("Undo##", nullptr, nullptr, menu_allowed.undo))
        {
            // To be implemented at some point
        }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Not implemented"); }
        
        if (ImGui::MenuItem("Redo##", nullptr, nullptr, menu_allowed.redo))
        {
            // TO be implemented at some point
        }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Not implemented"); }
        
        ImGui::EndMenu();
    }
    // Project Menu
    // Eventually projects will be supported
    // That will involve keeping track of files, actions, etc.
    // Place-holder for now as a promise and reminder of my intentions
    if (ImGui::BeginMenu("Project##", menu_allowed.project_menu)) { ImGui::EndMenu(); }
    // Options Menu
    // Changing fonts, their sizes, etc.
    if (ImGui::BeginMenu("Options##", menu_allowed.options_menu)) { ImGui::EndMenu(); }
    // Window menu
    if (ImGui::BeginMenu("Window##", menu_allowed.window_menu))
    {
        // Bring all windows to the center incase the layout got borked
        if (ImGui::MenuItem("Center Windows##", nullptr, nullptr, menu_allowed.center_windows))
        {
            // To be implemented at some point
        }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Center ALL windows. Not yet implemented"); }
        
        // Change the default layout, if the user wants that
        if (ImGui::MenuItem("Save Layout##", nullptr, nullptr, menu_allowed.save_layout))
        {
            // To be implemented at some point
        }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Save current window layout as new default. Not yet implemented"); }
        // Reset window positions incase something got lost
        
        if (ImGui::MenuItem("Reset Windows##", nullptr, nullptr, menu_allowed.reset_windows))
        {
            allwindow_settings.welcome.is_set = false;
            allwindow_settings.fps.is_set = false;
            allwindow_settings.header.is_set = false;
            allwindow_settings.plot_1c.is_set = false;
            allwindow_settings.spectrum_1c.is_set = false;
            allwindow_settings.sac_files.is_set = false;
            allwindow_settings.lowpass.is_set = false;
            allwindow_settings.highpass.is_set = false;
            allwindow_settings.bandpass.is_set = false;
        }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Reset all windows to default position and size"); }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Welcome##", nullptr, nullptr, menu_allowed.welcome))
        { allwindow_settings.welcome.show = true; }
        
        if (ImGui::MenuItem("FPS Tracker##", nullptr, nullptr, menu_allowed.fps))
        { allwindow_settings.fps.show = true; }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Frames Per Second display"); }
        
        if (ImGui::MenuItem("Sac Header##", nullptr, nullptr, menu_allowed.sac_header))
        { allwindow_settings.header.show = true; }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Displays SAC header values"); }
        
        if (ImGui::MenuItem("Sac Plot 1C##", nullptr, nullptr, menu_allowed.plot_1c))
        { allwindow_settings.plot_1c.show = true; }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("1-component SAC plot"); }
        
        if (ImGui::MenuItem("Spectrum Plot 1C##", nullptr, nullptr, menu_allowed.plot_spectrum_1c))
        { allwindow_settings.spectrum_1c.show = true; }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("1-component SAC spectrogram (real/imaginary) plot"); }
        
        if (ImGui::MenuItem("Sac List##", nullptr, nullptr, (menu_allowed.sac_deque && program_status.is_idle)))
        { allwindow_settings.sac_files.show = true; }
        
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("List of SAC files currently loaded in memory"); }
        
        ImGui::EndMenu();
    }
    // Open File Dialog (single SAC)
    ImVec2 maxSize = ImVec2(allwindow_settings.file_dialog.width * 1.5, allwindow_settings.file_dialog.height * 1.5);
    ImVec2 minSize = ImVec2(maxSize.x * 0.75f, maxSize.y * 0.75f);
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
    {
        // Read the SAC-File safely
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
            program_status.fileio.count = 0;
            // Can only select 1 file anyway!
            program_status.fileio.total = 1;
            program_status.thread_pool.enqueue(read_sac_1c, std::ref(sac_deque), std::ref(program_status.fileio), ImGuiFileDialog::Instance()->GetFilePathName());
        }
        ImGuiFileDialog::Instance()->Close();
    }
    if (ImGuiFileDialog::Instance()->Display("ChooseDirDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::filesystem::path directory = ImGuiFileDialog::Instance()->GetFilePathName();
            std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
            program_status.thread_pool.enqueue(scan_and_read_dir, std::ref(program_status), std::ref(sac_deque), directory);
        }
        ImGuiFileDialog::Instance()->Close();
    }
    // Save file dialog (single sac)
    if (ImGuiFileDialog::Instance()->Display("SaveFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
    {
        // Save the SAC-File safely
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::lock_guard<std::shared_mutex> lock_sac(sac_deque[active_sac].sac_mutex);
            sac_deque[active_sac].sac.write(ImGuiFileDialog::Instance()->GetFilePathName());
        }
        ImGuiFileDialog::Instance()->Close();
    }
    if (ImGui::BeginMenu("Processing##", menu_allowed.processing_menu))
    {
        if (ImGui::MenuItem("Remove Mean##", nullptr, nullptr, menu_allowed.rmean))
        {
            std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
            program_status.fileio.is_processing = true;
            program_status.fileio.count = 0;
            program_status.fileio.total = 1;
            program_status.thread_pool.enqueue(remove_mean, std::ref(program_status.fileio), std::ref(sac_deque[active_sac]));
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Remove mean value from active data."); }
        
        if (ImGui::MenuItem("Remove Trend##", nullptr, nullptr, menu_allowed.rtrend))
        {
            std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
            program_status.fileio.is_processing = true;
            program_status.fileio.count = 0;
            program_status.fileio.total = 1;
            program_status.thread_pool.enqueue(remove_trend, std::ref(program_status.fileio), std::ref(sac_deque[active_sac]));
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Remove linear trend from active data."); }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Lowpass##", nullptr, nullptr, menu_allowed.lowpass))
        {
            allwindow_settings.lowpass.show = true;
            allwindow_settings.highpass.show = false;
            allwindow_settings.bandpass.show = false;
            af_settings.lowpass.apply_batch = false;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Butterworth Lowpass filter active data."); }
        
        if (ImGui::MenuItem("Highpass##", nullptr, nullptr, menu_allowed.highpass))
        {
            allwindow_settings.lowpass.show = false;
            allwindow_settings.highpass.show = true;
            allwindow_settings.bandpass.show = false;
            af_settings.highpass.apply_batch = false;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Butterworth Highpass filter active data."); }
        
        if (ImGui::MenuItem("Bandpass##", nullptr, nullptr, menu_allowed.bandpass))
        {
            allwindow_settings.lowpass.show = false;
            allwindow_settings.highpass.show = false;
            allwindow_settings.bandpass.show = true;
            af_settings.bandpass.apply_batch = false;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Butterworth Bandpass filter active data."); }
        
        if (ImGui::MenuItem("Bandreject##", nullptr, nullptr, menu_allowed.bandreject))
        {
            // To be implemented later
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Butterworth Bandreject filter active data. Not implemented"); }
        
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Picking##", menu_allowed.picking_menu)) { ImGui::EndMenu(); }
    
    if (ImGui::BeginMenu("Batch##", menu_allowed.batch_menu)) // testing disabling a menu
    {
        if (ImGui::MenuItem("Remove Mean##", nullptr, nullptr, menu_allowed.rmean))
        {
            std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
            program_status.thread_pool.enqueue(batch_remove_mean, std::ref(program_status), std::ref(sac_deque));
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Remove mean value from all data."); }
        
        if (ImGui::MenuItem("Remove Trend##", nullptr, nullptr, menu_allowed.rtrend))
        {
            std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
            program_status.thread_pool.enqueue(batch_remove_trend, std::ref(program_status), std::ref(sac_deque));
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Remove trend value from all data."); }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("Lowpass##", nullptr, nullptr, menu_allowed.lowpass))
        {
            allwindow_settings.lowpass.show = true;
            allwindow_settings.highpass.show = false;
            allwindow_settings.bandpass.show = false;
            af_settings.lowpass.apply_batch = true;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Butterworth Lowpass filter all data. Not implemented"); }
        
        if (ImGui::MenuItem("Highpass##", nullptr, nullptr, menu_allowed.highpass))
        {
            allwindow_settings.lowpass.show = false;
            allwindow_settings.highpass.show = true;
            allwindow_settings.bandpass.show = false;
            af_settings.highpass.apply_batch = true;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Butterworth Highpass filter all data. Not implemented"); }
       
        if (ImGui::MenuItem("Bandpass##", nullptr, nullptr, menu_allowed.bandpass))
        {
            allwindow_settings.lowpass.show = false;
            allwindow_settings.highpass.show = false;
            allwindow_settings.bandpass.show = true;
            af_settings.bandpass.apply_batch = true;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Butterworth Bandpass filter all data. Not implemented"); }
        
        if (ImGui::MenuItem("Bandreject##", nullptr, nullptr, menu_allowed.bandreject))
        {
            // To be implemented later
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
        { ImGui::SetTooltip("Butterworth Bandreject filter all data. Not implemented"); }
        
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}
//-----------------------------------------------------------------------------
// End Main menu bar
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 1-component SAC plot window
//-----------------------------------------------------------------------------
void pssp::window_plot_sac(WindowSettings& window_settings, std::deque<sac_1c>& sac_deque, int& selected)
{
    if (window_settings.show)
    {
        if (!window_settings.is_set)
        {
            ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
            ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
            window_settings.is_set = true;
        }
        
        ImGui::Begin(window_settings.title.c_str(), &window_settings.show, window_settings.img_flags);
        
        if (ImPlot::BeginPlot("Seismogram##"))
        {
            ImPlot::SetupAxis(ImAxis_X1, "Time (s)"); // Move this line here
            {
                std::shared_lock<std::shared_mutex> lock_sac(sac_deque[selected].sac_mutex);
                ImPlot::PlotLine("", &sac_deque[selected].sac.data1[0], sac_deque[selected].sac.data1.size(), sac_deque[selected].sac.delta);
            }
            // This allows us to add a separate context menu inside the plot area that appears upon double left-clicking
            // Right-clicking is reserved for the built in context menu (have not figured out how to add to it without
            // directly modifying ImPlot, which I don't want to do)
            ImPlotContext* plot_ctx = ImPlot::GetCurrentContext();
            if (plot_ctx && ImPlot::IsPlotHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            { ImGui::OpenPopup("CustomPlotOptions##"); }
            
            if (ImGui::BeginPopup("CustomPlotOptions##"))
            {
                if (ImGui::BeginMenu("Test##"))
                {
                    if (ImGui::MenuItem("Custom 1##")) {}

                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Test 2##"))
                {
                    if (ImGui::MenuItem("Custom 2##")) {}
                    if (ImGui::MenuItem("Custom 3##")) {}
                    
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
            ImPlot::EndPlot();
        }
        ImGui::End();
    }
}
//-----------------------------------------------------------------------------
// End 1-component SAC plot window
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 1-component SAC spectrum window
//-----------------------------------------------------------------------------
void pssp::window_plot_spectrum(WindowSettings& window_settings, sac_1c& spectrum)
{
    if (window_settings.show)
    {
        if (!window_settings.is_set)
        {
            ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
            ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
            window_settings.is_set = true;
        }
        ImGui::Begin(window_settings.title.c_str(), &window_settings.show, window_settings.img_flags);
        ImGui::Columns(2);
        if (ImPlot::BeginPlot("Real##"))
        {
            {
                std::shared_lock<std::shared_mutex> lock_spectrum(spectrum.sac_mutex);
                ImPlot::SetupAxis(ImAxis_X1, "Freq (Hz)");
                const double sampling_freq{1.0 / spectrum.sac.delta};
                const double freq_step{sampling_freq / spectrum.sac.npts};
                ImPlot::PlotLine("", &spectrum.sac.data1[0], spectrum.sac.data1.size() / 2, freq_step);
            }
            ImPlot::EndPlot();
        }
        ImGui::NextColumn();
        if (ImPlot::BeginPlot("Imaginary##"))
        {
            {
                std::shared_lock<std::shared_mutex> lock_spectrum(spectrum.sac_mutex);
                ImPlot::SetupAxis(ImAxis_X1, "Freq (Hz)");
                const double sampling_freq{1.0 / spectrum.sac.delta};
                const double freq_step{sampling_freq / spectrum.sac.npts};
                ImPlot::PlotLine("", &spectrum.sac.data2[0], spectrum.sac.data2.size() / 2, freq_step);
            }
            ImPlot::EndPlot();
        }
        ImGui::Columns(1);
        ImGui::End();
    }
}
//-----------------------------------------------------------------------------
// End 1-component SAC spectrum window
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 1-component SAC header window
//-----------------------------------------------------------------------------
void pssp::window_sac_header(ProgramStatus& program_status, WindowSettings& window_settings, sac_1c& sac)
{
    if (window_settings.show)
    {
        if (!window_settings.is_set)
        {
            ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
            ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
            window_settings.is_set = true;
        }
        ImGui::Begin(window_settings.title.c_str(), &window_settings.show, window_settings.img_flags);
        {
            if (!program_status.is_idle) { ImGui::BeginDisabled(); }
            
            std::shared_lock<std::shared_mutex> lock_sac(sac.sac_mutex);
            if (ImGui::CollapsingHeader("Station Information##", ImGuiTreeNodeFlags_DefaultOpen))
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
            if (ImGui::CollapsingHeader("Component Information##", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("Component:  %s", sac.sac.kcmpnm.c_str());
                ImGui::Text("Azimuth:    %.2f\u00B0", sac.sac.cmpaz);
                ImGui::Text("Incidence:  %.2f\u00B0", sac.sac.cmpinc);
            }
            if (ImGui::CollapsingHeader("Event Information##", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("Name:       %s", sac.sac.kevnm.c_str());
                ImGui::Text("Latitude:   %.2f\u00B0N", sac.sac.evla);
                ImGui::Text("Longitude:  %.2f\u00B0E", sac.sac.evlo);
                ImGui::Text("Depth:      %.2f km", sac.sac.evdp);
                ImGui::Text("Magnitude:  %.2f", sac.sac.mag);
                ImGui::Text("Azimuth:    %.2f\u00B0", sac.sac.az);
            }
            if (ImGui::CollapsingHeader("DateTime Information##", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("Year:       %i", sac.sac.nzyear);
                ImGui::Text("Julian Day: %i", sac.sac.nzjday);
                ImGui::Text("Hour:       %i", sac.sac.nzhour);
                ImGui::Text("Minute:     %i", sac.sac.nzmin);
                ImGui::Text("Second:     %i", sac.sac.nzsec);
                ImGui::Text("MSecond:    %i", sac.sac.nzmsec);
            }
            if (ImGui::CollapsingHeader("Data Information##", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("Npts:       %i", sac.sac.npts);
                ImGui::Text("IfType:     %i", sac.sac.iftype);
            }
            if (!program_status.is_idle) { ImGui::EndDisabled(); }
        }
        ImGui::End();
    }
}
//-----------------------------------------------------------------------------
// End 1-component SAC header window
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Welcome window
//-----------------------------------------------------------------------------
void pssp::window_welcome(WindowSettings& window_settings, std::string_view& welcome_message)
{
    if (window_settings.show)
    {
        if (!window_settings.is_set)
        {
            ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
            ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
            window_settings.is_set = true;
        }
        ImGui::Begin(window_settings.title.c_str(), &window_settings.show, window_settings.img_flags);
        ImGui::TextUnformatted(welcome_message.data());
        ImGui::End();
    }
}
//-----------------------------------------------------------------------------
// End Welcome window
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FPS window
//-----------------------------------------------------------------------------
// Creates a small window to show the FPS of the program, pretty much setup
void pssp::window_fps(fps_info& fps_tracker, WindowSettings& window_settings)
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
        ImGui::Begin(window_settings.title.c_str(), &window_settings.show, window_settings.img_flags);
        ImGui::Text("%i", static_cast<int>(fps_tracker.fps));
        ImGui::End();
    }
}
//-----------------------------------------------------------------------------
// End FPS window
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SAC-loaded window
//-----------------------------------------------------------------------------
void pssp::window_sac_deque(AllWindowSettings& aw_settings, MenuAllowed& menu_allowed,
ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, sac_1c& spectrum, int& selected, bool& cleared)
{
    WindowSettings& window_settings = aw_settings.sac_files;
    std::string option{};
    //if (window_settings.show && program_status.is_idle)
    if (window_settings.show && program_status.is_idle)
    {
        if (!window_settings.is_set)
        {
            // Setup the window
            ImGui::SetNextWindowSize(ImVec2(window_settings.width, window_settings.height));
            ImGui::SetNextWindowPos(ImVec2(window_settings.pos_x, window_settings.pos_y));
            window_settings.is_set = true;
        }
        ImGui::Begin(window_settings.title.c_str(), &window_settings.show, window_settings.img_flags);
        if (!program_status.is_idle) { ImGui::BeginDisabled(); }
        for (int i = 0; const auto& sac : sac_deque)
        {
            const bool is_selected{selected == i};
            option = sac.file_name.substr(sac.file_name.find_last_of("\\/") + 1) + "##";
            if (ImGui::Selectable(option.c_str(), is_selected)) { selected = i; }
            // Right-click menu
            if (ImGui::BeginPopupContextItem((std::string("Context Menu##") + std::to_string(i)).c_str()))
            {
                if (ImGui::MenuItem("Save##", nullptr, nullptr, menu_allowed.save_1c)) { selected = i; }
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
                { ImGui::SetTooltip("Save SAC file. Not implemented in this context. Use File->Save 1C"); }
                
                if (ImGui::MenuItem("Remove##"))
                {
                    selected = i;
                    cleared = true;
                }
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) { ImGui::SetTooltip("Unload SAC data from memory"); }
                
                if (ImGui::MenuItem("Reload##"))
                {
                    selected = i;
                    {
                    std::lock_guard<std::shared_mutex> lock_sac(sac_deque[selected].sac_mutex);
                    sac_deque[selected].sac = SAC::SacStream(sac_deque[selected].file_name);
                    }
                    calc_spectrum(sac_deque[selected], spectrum);
                }
                
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) { ImGui::SetTooltip("Reload the original SAC file"); }
                
                if (ImGui::MenuItem("LowPass##", nullptr, nullptr, menu_allowed.lowpass))
                {
                    selected = i;
                    aw_settings.lowpass.show = true;
                    aw_settings.highpass.show = false;
                    aw_settings.bandpass.show = false;
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) { ImGui::SetTooltip("Butterworth lowpass filter"); }
                
                if (ImGui::MenuItem("HighPass##", nullptr, nullptr, menu_allowed.highpass))
                {
                    selected = i;
                    aw_settings.lowpass.show = false;
                    aw_settings.highpass.show = true;
                    aw_settings.bandpass.show = false;
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) { ImGui::SetTooltip("Butterworth highpass filter"); }
                
                if (ImGui::MenuItem("BandPass##", nullptr, nullptr, menu_allowed.bandpass))
                {
                    selected = i;
                    aw_settings.lowpass.show = false;
                    aw_settings.highpass.show = false;
                    aw_settings.bandpass.show = true;
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) { ImGui::SetTooltip("Butterworth bandpass filter"); }
                ImGui::EndPopup();
            }
            ++i;
        }
        if (!program_status.is_idle) { ImGui::EndDisabled(); }
        ImGui::End();
    }
}
//-----------------------------------------------------------------------------
// End SAC-loaded window
//-----------------------------------------------------------------------------
