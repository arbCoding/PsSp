#include "pssp_misc.hpp"
#include <filesystem>
#include <mutex>
#include <shared_mutex>
#include <sstream>

//-----------------------------------------------------------------------------
// Misc functions
//-----------------------------------------------------------------------------
void pssp::update_fps(fps_info& fps, ImGuiIO& io)
{
    // Lock the fps_tracker
    std::lock_guard<std::mutex> guard(fps.mutex_);
    // Not using the lock_guard here
    // Increase time
    fps.current_time += io.DeltaTime;
    // Update the interval
    fps.current_interval = fps.current_time - fps.prev_time;
    // Increase frame_count
    ++fps.frame_count;
}

void pssp::cleanup_sac(Project& project, std::deque<sac_1c>& sac_deque, int& selected, bool& clear)
{
    if (clear)
    {
        (void) project;
        --selected;
        {
            // I still need to add in timestamping the removal in the database
            std::lock_guard<std::shared_mutex> lock_sac(sac_deque[selected].mutex_);
            project.add_data_processing(sac_deque[selected].data_id, "REMOVED");
        }
        sac_deque.erase(sac_deque.begin() + selected + 1);
        if (selected < 0 && sac_deque.size() > 0)
        {
            selected = 0;
        }
        clear = false;
    }
}

void pssp::calc_spectrum(sac_1c& sac, sac_1c& spectrum)
{
    std::lock_guard<std::shared_mutex> lock_spectrum(spectrum.mutex_);
    {
        std::shared_lock<std::shared_mutex> lock_sac(sac.mutex_);
        spectrum.sac = sac.sac;
        spectrum.file_name = sac.file_name;
    }
    // Calculate the FFT
    SAC::fft_real_imaginary(spectrum.sac);
}

void pssp::remove_mean(Project& project, FileIO& fileio, sac_1c& sac)
{
    {
        std::lock_guard<std::shared_mutex> lock_sac(sac.mutex_);
        project.add_data_processing(sac.data_id, "REMOVE MEAN");
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
                
            }
            // The mean is zero
            sac.sac.depmen = 0.0f;
        }
    }
    std::lock_guard<std::shared_mutex> lock_io(fileio.mutex_);
    ++fileio.count;
}

void pssp::batch_remove_mean(Project& project, ProgramStatus& program_status, std::deque<sac_1c>& sac_deque)
{
    std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
    program_status.fileio.is_processing = true;
    program_status.fileio.count = 0;
    program_status.fileio.total = static_cast<int>(sac_deque.size());
    for (std::size_t i{0}; i < sac_deque.size(); ++i)
    {
        program_status.thread_pool.enqueue(remove_mean, std::ref(project), std::ref(program_status.fileio), std::ref(sac_deque[i]));
    }
}

void pssp::remove_trend(Project& project, FileIO& fileio, sac_1c& sac)
{
    std::lock_guard<std::shared_mutex> lock_sac(sac.mutex_);
    project.add_data_processing(sac.data_id, "REMOVE TREND");
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
    std::lock_guard<std::shared_mutex> lock_io(fileio.mutex_);
    ++fileio.count;
}

void pssp::batch_remove_trend(Project& project, ProgramStatus& program_status, std::deque<sac_1c>& sac_deque)
{
    std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
    program_status.fileio.is_processing = true;
    program_status.fileio.count = 0;
    program_status.fileio.total = static_cast<int>(sac_deque.size());
    for (std::size_t i{0}; i < sac_deque.size(); ++i)
    {
        program_status.thread_pool.enqueue(remove_trend, std::ref(project), std::ref(program_status.fileio), std::ref(sac_deque[i]));
    }
}

void pssp::apply_lowpass(Project& project, FileIO& fileio, sac_1c& sac, FilterOptions& lowpass_options)
{
    {
        std::lock_guard<std::shared_mutex> lock_sac(sac.mutex_);
        std::ostringstream oss{};
        oss << "LOWPASS; ORDER ";
        oss << lowpass_options.order;
        oss << "; FREQ LOW ";
        oss << lowpass_options.freq_low;
        oss << ";";
        project.add_data_processing(sac.data_id, oss.str());
        SAC::lowpass(sac.sac, lowpass_options.order, lowpass_options.freq_low);
    }
    std::lock_guard<std::shared_mutex> lock_io(fileio.mutex_);
    ++fileio.count;
}

void pssp::batch_apply_lowpass(Project& project, ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, FilterOptions& lowpass_options)
{
    {
        std::lock_guard<std::shared_mutex> lock_io(program_status.fileio.mutex_);
        program_status.fileio.count = 0;
        program_status.fileio.is_processing = true;
        program_status.fileio.total = static_cast<int>(sac_deque.size());
    }
    for (std::size_t i{0}; i < sac_deque.size(); ++i)
    {
        apply_lowpass(project, program_status.fileio, sac_deque[i], lowpass_options);
    }
}

void pssp::apply_highpass(Project& project, FileIO& fileio, sac_1c& sac, FilterOptions& highpass_options)
{
    {
        std::lock_guard<std::shared_mutex> lock_sac(sac.mutex_);
        std::ostringstream oss{};
        oss << "HIGHPASS; ORDER ";
        oss << highpass_options.order;
        oss << "; FREQ LOW ";
        oss << highpass_options.freq_low;
        oss << ";";
        project.add_data_processing(sac.data_id, oss.str());
        SAC::highpass(sac.sac, highpass_options.order, highpass_options.freq_low);
    }
    std::lock_guard<std::shared_mutex> lock_io(fileio.mutex_);
    ++fileio.count;
}

void pssp::batch_apply_highpass(Project& project, ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, FilterOptions& highpass_options)
{
    {
        std::lock_guard<std::shared_mutex> lock_io(program_status.fileio.mutex_);
        program_status.fileio.count = 0;
        program_status.fileio.is_processing = true;
        program_status.fileio.total = static_cast<int>(sac_deque.size());
    }
    for (std::size_t i{0}; i < sac_deque.size(); ++i)
    {
        apply_highpass(project, program_status.fileio, sac_deque[i], highpass_options);
    }
}

void pssp::apply_bandpass(Project& project, FileIO& fileio, sac_1c& sac, FilterOptions& bandpass_options)
{
    {
        std::lock_guard<std::shared_mutex> lock_sac(sac.mutex_);
        std::ostringstream oss{};
        oss << "BANDPASS; ORDER ";
        oss << bandpass_options.order;
        oss << "; FREQ LOW ";
        oss << bandpass_options.freq_low;
        oss << "; FREQ HIGH ";
        oss << bandpass_options.freq_high;
        oss << ";";
        project.add_data_processing(sac.data_id, oss.str());
        SAC::bandpass(sac.sac, bandpass_options.order, bandpass_options.freq_low, bandpass_options.freq_high);
    }
    std::lock_guard<std::shared_mutex> lock_io(fileio.mutex_);
    ++fileio.count;
}

void pssp::batch_apply_bandpass(Project& project, ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, FilterOptions& bandpass_options)
{
    {
        std::lock_guard<std::shared_mutex> lock_io(program_status.fileio.mutex_);
        program_status.fileio.count = 0;
        program_status.fileio.is_processing = true;
        program_status.fileio.total = static_cast<int>(sac_deque.size());
    }
    for (std::size_t i{0}; i < sac_deque.size(); ++i)
    {
        apply_bandpass(project, program_status.fileio, sac_deque[i], bandpass_options);
    }
}

void pssp::read_sac_1c(std::deque<sac_1c>& sac_deque, FileIO& fileio, const std::filesystem::path file_name, Project& project)
{
    sac_1c sac{};
    {
        std::lock_guard<std::shared_mutex> lock_sac(sac.mutex_);
        sac.file_name = file_name;
        sac.sac = SAC::SacStream(sac.file_name);
        sac.data_id = project.add_sac(sac.sac, file_name.string());
    }
    std::shared_lock<std::shared_mutex> lock_sac(sac.mutex_);
    std::lock_guard<std::shared_mutex> lock_io(fileio.mutex_);
    fileio.is_reading = true;
    ++fileio.count;
    sac_deque.push_back(sac);
}

void pssp::scan_and_read_dir(ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, std::filesystem::path directory, Project& project)
{
    // Iterate over files in directory
    std::vector<std::filesystem::path> file_names{};
    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        // Check extension
        if (entry.path().extension() == ".sac" || entry.path().extension() == ".SAC")
        {
            // Using canconical so that we have the entire path for the Project database
            file_names.push_back(std::filesystem::canonical(entry.path()));
        }
    }
    std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
    program_status.is_idle = false;
    program_status.fileio.total = static_cast<int>(file_names.size());
    program_status.fileio.count = 0;
    // Queue them up!
    for (std::string file_name : file_names)
    {
        program_status.thread_pool.enqueue(read_sac_1c, std::ref(sac_deque), std::ref(program_status.fileio), file_name, std::ref(project));
    }
}
//-----------------------------------------------------------------------------
// Graphical Backend functions
//-----------------------------------------------------------------------------
// Setup the graphicaly libraries, figure out version info depending on OS
const char* pssp::setup_gl()
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

ImGuiIO& pssp::start_graphics(GLFWwindow* window, const char* glsl_version, std::filesystem::path program_path)
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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //----------------------------------------------------------------------
    // Resize fonts
    //----------------------------------------------------------------------
    // Will need to provide the font with the Application package
    // Will also need to keep it linked to the programs location, not the directory the program was called from...
    //ImFont* font = io.Fonts->AddFontFromFileTTF("./fonts/Hack/HackNerdFontMono-Regular.ttf", 18);
    std::string font_path{program_path.string()};
    font_path += "/HackNerdFontMono-Regular.ttf";
    constexpr int font_size{18};
    ImFont* font = io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size);
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

void pssp::end_graphics(GLFWwindow* window)
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
void pssp::glfw_error_callback(int error, const char *description)
{
    std::cerr << "GLFW Error " << error << ": " << description << '\n'; 
}

void pssp::prep_newframe()
{
    // Check for user input (mouse, keyboard, etc)
    glfwPollEvents();
    // Tell the backends to prepare for a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    // Tell Dear ImGui to prepare for a new frame
    ImGui::NewFrame();
}

void pssp::finish_newframe(GLFWwindow* window, ImVec4 clear_color)
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
// Checkpoint data (inside thread_pool)
//-----------------------------------------------------------------------------
void pssp::checkpoint_data(FileIO& fileio, Project& project, sac_1c& sac)
{
    {
        std::lock_guard<std::shared_mutex> lock_sac(sac.mutex_);
        project.add_data_checkpoint(sac.sac, sac.data_id);
        project.add_data_processing(sac.data_id, "CHECKPOINT");
    }
    std::lock_guard<std::shared_mutex> lock_io(fileio.mutex_);
    ++fileio.count;
}
//-----------------------------------------------------------------------------
// End checkpoint data (inside thread_pool)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Unload data from memory
//-----------------------------------------------------------------------------
void pssp::unload_data(Project& project, ProgramStatus& program_status, std::deque<sac_1c>& sac_deque)
{
    // Remove the SQLite3 connections and the file paths
    project.unload_project();
    // Clear data from the sac_deque
    {
        std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
        std::lock_guard<std::shared_mutex> lock_io(program_status.fileio.mutex_);
        program_status.fileio.count = 0;
        program_status.fileio.total = 0;
    }
    sac_deque.clear();
}
//-----------------------------------------------------------------------------
// End Unload data from memory
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Get SacStream from project, add to sac_deque
//-----------------------------------------------------------------------------
void pssp::fill_deque_project(Project& project, FileIO& fileio, std::deque<sac_1c>& sac_deque, int data_id)
{
    sac_1c sac{};
    {
        std::lock_guard<std::shared_mutex> lock_sac(sac.mutex_);
        // This is sometimes failing after unload and reload
        // It seems that the data_id is repeated...
        sac.sac = project.load_sacstream(data_id);
        sac.file_name = project.get_source(data_id);
        sac.data_id = data_id;
    }
    std::shared_lock<std::shared_mutex> lock_sac(sac.mutex_);
    std::lock_guard<std::shared_mutex> lock_io(fileio.mutex_);
    fileio.is_reading = true;
    ++fileio.count;
    sac_deque.push_back(sac);
}

//-----------------------------------------------------------------------------
// End Get SacStream from project, add to sac_deque
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Load a project from a project file
//-----------------------------------------------------------------------------
void pssp::load_data(Project& project, ProgramStatus& program_status, std::deque<sac_1c>& sac_deque, const std::filesystem::path project_file, int checkpoint_id)
{
    // First make sure we unload the present project
    //unload_data(project, program_status, sac_deque);
    // Connection to the project file
    project.connect_2_existing(project_file);
    if (checkpoint_id == -1){ checkpoint_id = project.get_latest_checkpoint_id(); }
    // Set the checkpoint id to the latest checkpoint
    project.set_checkpoint_id(checkpoint_id);
    // Get the data-ids to load
    std::vector<int> data_ids{project.get_data_ids_for_current_checkpoint()};
    {
        std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
        program_status.is_idle = false;
        program_status.fileio.total = static_cast<int>(data_ids.size());
        program_status.fileio.count = 0;
    }
    // Queue it up!
    for (std::size_t i{0}; i < data_ids.size(); ++i)
    {
        program_status.thread_pool.enqueue(fill_deque_project, std::ref(project), std::ref(program_status.fileio), std::ref(sac_deque), data_ids[i]);
    }
}
//-----------------------------------------------------------------------------
// End load a project from a project file
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// End Misc functions
//-----------------------------------------------------------------------------
