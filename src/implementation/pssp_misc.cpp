#include "pssp_misc.hpp"
#include "pssp_spectral.hpp"
#include <shared_mutex>

namespace pssp
{
//-----------------------------------------------------------------------------
// Misc functions
//-----------------------------------------------------------------------------
void update_fps(fps_info& fps, ImGuiIO& io)
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

// Sometimes calling the filters hangs the program at 100% (seems like a piece of data is being
// removed at the same time it is being locked? unsure).

/*
void cleanup_sac(Project& project, std::deque<sac_1c>& sac_deque, int& selected, bool& clear)
{
    if (clear)
    {
        (void) project;
        --selected;
        {
            // I still need to add in timestamping the removal in the database
            std::lock_guard<std::shared_mutex> lock_sac(sac_deque[selected].mutex_);
            project.add_data_processing(project.sq3_connection_file, sac_deque[selected].data_id, "REMOVED");
        }
        sac_deque.erase(sac_deque.begin() + selected + 1);
        if (selected < 0 && sac_deque.size() > 0)
        {
            selected = 0;
        }
        clear = false;
    }
}
*/

void calc_spectrum(ProgramStatus& program_status, int data_id, sac_1c& spectrum)
{
    if (!program_status.project.is_project) { return; }
    std::shared_ptr<sac_1c> sac_ptr{program_status.data_pool.get_ptr(program_status.project, data_id, program_status.project.checkpoint_id_)};
    if (!sac_ptr) { return; }
    
    std::lock_guard<std::shared_mutex> lock_sac(sac_ptr->mutex_);
    std::lock_guard<std::shared_mutex> lock_spectrum(spectrum.mutex_);
    std::vector<std::complex<double>> complex_spectrum{};
    {
        spectrum.file_name = sac_ptr->file_name;
        spectrum.sac = sac_ptr->sac;
        complex_spectrum = fft_time_series(program_status.fftw_planpool, sac_ptr->sac.data1, true);
    }
    spectrum.sac.data1.resize(complex_spectrum.size());
    spectrum.sac.data2.resize(complex_spectrum.size());
    for (std::size_t i{0}; i < complex_spectrum.size(); ++i)
    {
        spectrum.sac.data1[i] = complex_spectrum[i].real();
        spectrum.sac.data2[i] = complex_spectrum[i].imag();
    }
    program_status.data_pool.return_ptr(program_status.project, sac_ptr);
}

void remove_mean(ProgramStatus& program_status, int data_id)
{
    if (!program_status.project.is_project) { return; }
    std::shared_ptr<sac_1c> sac_ptr{program_status.data_pool.get_ptr(program_status.project, data_id, program_status.project.checkpoint_id_)};
    if (!sac_ptr) { return; }
    
    program_status.state.store(processing);
    {
        std::lock_guard<std::shared_mutex> lock_sac(sac_ptr->mutex_);
        program_status.project.add_data_processing(program_status.project.sq3_connection_memory, data_id, "REMOVE MEAN");
        double mean{0};
        // Check if the mean is already set
        if (sac_ptr->sac.depmen != SAC::unset_float)
        {
            mean = sac_ptr->sac.depmen;
        }
        else
        {
            // Need to calculate the mean...
            for (int i{0}; i < sac_ptr->sac.npts; ++i)
            {
                mean += sac_ptr->sac.data1[i];
            }
            mean /= static_cast<double>(sac_ptr->sac.npts);
        }
        // If out mean is zero, then we're done
        if (mean != 0.0 || sac_ptr->sac.depmen != 0.0f)
        {
            // Subtract the mean from ever data point
            for (int i{0}; i < sac_ptr->sac.npts; ++i)
            {
                sac_ptr->sac.data1[i] -= mean;
            }
            // The mean is zero
            sac_ptr->sac.depmen = 0.0f;
        }
    }
    program_status.data_pool.return_ptr(program_status.project, sac_ptr);
    ++program_status.tasks_completed;
}

void batch_remove_mean(ProgramStatus& program_status)
{
    std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
    program_status.state.store(processing);
    program_status.tasks_completed = 0;
    std::vector<int> id_order{program_status.data_pool.get_iter(program_status.project)};
    program_status.total_tasks = static_cast<int>(id_order.size());
    for (std::size_t i{0}; i < id_order.size(); ++i)
    {
        program_status.thread_pool.enqueue(remove_mean, std::ref(program_status), id_order[i]);
    }
}

void remove_trend(ProgramStatus& program_status, int data_id)
{
    if (!program_status.project.is_project) { return; }
    std::shared_ptr<sac_1c> sac_ptr{program_status.data_pool.get_ptr(program_status.project, data_id, program_status.project.checkpoint_id_)};
    if (!sac_ptr) { return; }
    
    program_status.state.store(processing);
    {
        std::lock_guard<std::shared_mutex> lock_sac(sac_ptr->mutex_);
        program_status.project.add_data_processing(program_status.project.sq3_connection_memory, data_id, "REMOVE TREND");
        double mean_amplitude{0};
        // Static_cast just to be sure no funny business
        double mean_t{static_cast<double>(sac_ptr->sac.npts) / 2.0};
        // If depmen is not set, so no average to start from
        if (sac_ptr->sac.depmen == SAC::unset_float)
        {
            for (int i{0}; i < sac_ptr->sac.npts; ++i)
            {
                mean_amplitude += sac_ptr->sac.data1[0];
            }
            mean_amplitude /= static_cast<double>(sac_ptr->sac.npts);
        }
        else
        {
            mean_amplitude = sac_ptr->sac.depmen;
        }
        // Now to calculate the slope and y-intercept (y = amplitude)
        double numerator{0};
        double denominator{0};
        double t_diff{0};
        for (int i{0}; i < sac_ptr->sac.npts; ++i)
        {
            t_diff = i - mean_t;
            numerator += t_diff * (sac_ptr->sac.data1[i] - mean_amplitude);
            denominator += t_diff * t_diff;
        }
        double slope{numerator / denominator};
        double amplitude_intercept{mean_amplitude - (slope * mean_t)};
        // Now to remove the linear trend from the data
        for (int i{0}; i < sac_ptr->sac.npts; ++i)
        {
            sac_ptr->sac.data1[i] -= (slope * i) + amplitude_intercept;
        }
    }
    program_status.data_pool.return_ptr(program_status.project, sac_ptr);
    ++program_status.tasks_completed;
}

void batch_remove_trend(ProgramStatus& program_status)
{
    std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
    program_status.state.store(processing);
    program_status.tasks_completed = 0;
    std::vector<int> id_order{program_status.data_pool.get_iter(program_status.project)};
    program_status.total_tasks = static_cast<int>(id_order.size());
    for (std::size_t i{0}; i < id_order.size(); ++i)
    {
        program_status.thread_pool.enqueue(remove_trend, std::ref(program_status), id_order[i]);
    }
}

void apply_lowpass(ProgramStatus& program_status, int data_id, FilterOptions& lowpass_options)
{
    if (!program_status.project.is_project) { return; }
    std::shared_ptr<sac_1c> sac_ptr{program_status.data_pool.get_ptr(program_status.project, data_id, program_status.project.checkpoint_id_)};
    if (!sac_ptr) { return; }
    std::lock_guard<std::shared_mutex> lock_sac(sac_ptr->mutex_);
    
    program_status.state.store(processing);
    std::ostringstream oss{};
    oss << "LOWPASS; ORDER ";
    oss << lowpass_options.order;
    oss << "; FREQ LOW ";
    oss << lowpass_options.freq_low;
    oss << ";";
    program_status.project.add_data_processing(program_status.project.sq3_connection_memory, data_id, oss.str());
    // It is dying in here
    lowpass(program_status.fftw_planpool, sac_ptr, lowpass_options.order, lowpass_options.freq_low);
    program_status.data_pool.return_ptr(program_status.project, sac_ptr);
    ++program_status.tasks_completed;
}

void batch_apply_lowpass(ProgramStatus& program_status, FilterOptions& lowpass_options)
{
    std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
    program_status.state.store(processing);
    program_status.tasks_completed = 0;
    std::vector<int> id_order{program_status.data_pool.get_iter(program_status.project)};
    program_status.total_tasks = static_cast<int>(id_order.size());
    for (std::size_t i{0}; i < id_order.size(); ++i)
    {
        program_status.thread_pool.enqueue(apply_lowpass, std::ref(program_status), id_order[i], std::ref(lowpass_options));
    }
}

void apply_highpass(ProgramStatus& program_status, int data_id, FilterOptions& highpass_options)
{
    if (!program_status.project.is_project) { return; }
    std::shared_ptr<sac_1c> sac_ptr{program_status.data_pool.get_ptr(program_status.project, data_id, program_status.project.checkpoint_id_)};
    if (!sac_ptr) { return; }
    std::lock_guard<std::shared_mutex> lock_sac(sac_ptr->mutex_);
    
    program_status.state.store(processing);
    std::ostringstream oss{};
    oss << "HIGHPASS; ORDER ";
    oss << highpass_options.order;
    oss << "; FREQ LOW ";
    oss << highpass_options.freq_low;
    oss << ";";
    program_status.project.add_data_processing(program_status.project.sq3_connection_memory, data_id, oss.str());
    highpass(program_status.fftw_planpool, sac_ptr, highpass_options.order, highpass_options.freq_low);
    program_status.data_pool.return_ptr(program_status.project, sac_ptr);
    ++program_status.tasks_completed;
}

void batch_apply_highpass(ProgramStatus& program_status, FilterOptions& highpass_options)
{
    std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
    program_status.state.store(processing);
    program_status.tasks_completed = 0;
    std::vector<int> id_order{program_status.data_pool.get_iter(program_status.project)};
    program_status.total_tasks = static_cast<int>(id_order.size());
    for (std::size_t i{0}; i < id_order.size(); ++i)
    {
        program_status.thread_pool.enqueue(apply_highpass, std::ref(program_status), id_order[i], std::ref(highpass_options));
    }
}

void apply_bandpass(ProgramStatus& program_status, int data_id, FilterOptions& bandpass_options)
{
    if (!program_status.project.is_project) { return; }
    std::shared_ptr<sac_1c> sac_ptr{program_status.data_pool.get_ptr(program_status.project, data_id, program_status.project.checkpoint_id_)};
    if (!sac_ptr) { return; }
    std::lock_guard<std::shared_mutex> lock_sac(sac_ptr->mutex_);
    
    program_status.state.store(processing);
    std::ostringstream oss{};
    oss << "BANDPASS; ORDER ";
    oss << bandpass_options.order;
    oss << "; FREQ LOW ";
    oss << bandpass_options.freq_low;
    oss << "; FREQ HIGH ";
    oss << bandpass_options.freq_high;
    oss << ";";
    program_status.project.add_data_processing(program_status.project.sq3_connection_memory, data_id, oss.str());
    bandpass(program_status.fftw_planpool, sac_ptr, bandpass_options.order, bandpass_options.freq_low, bandpass_options.freq_high);
    program_status.data_pool.return_ptr(program_status.project, sac_ptr);
    ++program_status.tasks_completed;
}

void batch_apply_bandpass(ProgramStatus& program_status, FilterOptions& bandpass_options)
{
    std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
    program_status.state.store(processing);
    program_status.tasks_completed = 0;
    std::vector<int> id_order{program_status.data_pool.get_iter(program_status.project)};
    program_status.total_tasks = static_cast<int>(id_order.size());
    for (std::size_t i{0}; i < id_order.size(); ++i)
    {
        program_status.thread_pool.enqueue(apply_bandpass, std::ref(program_status), id_order[i], std::ref(bandpass_options));
    }
}

void read_sac(ProgramStatus& program_status, const std::filesystem::path file_name)
{
    program_status.state.store(in);
    sac_1c sac{};
    {
        std::lock_guard<std::shared_mutex> lock_sac(sac.mutex_);
        sac.file_name = file_name;
        sac.sac = SAC::SacStream(sac.file_name);
        sac.data_id = program_status.project.add_sac(sac.sac, file_name.string());
    }
    std::shared_lock<std::shared_mutex> lock_sac(sac.mutex_);
    if (program_status.data_pool.n_data() < program_status.data_pool.max_data)
    {
        program_status.data_pool.add_data(program_status.project, sac.data_id, program_status.project.checkpoint_id_);
    }
    ++program_status.tasks_completed;
}

void scan_and_read_dir(ProgramStatus& program_status, std::filesystem::path directory)
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
    program_status.state.store(in);
    program_status.tasks_completed = 0;
    program_status.total_tasks = static_cast<int>(file_names.size());
    // Queue them up!
    for (std::string file_name : file_names)
    {
        program_status.thread_pool.enqueue(read_sac, std::ref(program_status), file_name);
    }
}
//------------------------------------------------------------------------
// Graphical Backend functions
//------------------------------------------------------------------------
// Setup the graphicaly libraries, figure out version info depending on OS
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

ImGuiIO& start_graphics(GLFWwindow* window, const char* glsl_version, std::filesystem::path program_path)
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
    //---------------------------------------------------------------
    // Resize fonts
    //---------------------------------------------------------------
    // Will need to provide the font with the Application package
    // Will also need to keep it linked to the programs location, not the directory the program was called from...
    //ImFont* font = io.Fonts->AddFontFromFileTTF("./fonts/Hack/HackNerdFontMono-Regular.ttf", 18);
    std::string font_path{program_path.string()};
    font_path += "/HackNerdFontMono-Regular.ttf";
    constexpr int font_size{18};
    ImFont* font = io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size);
    io.FontDefault = font;
    //---------------------------------------------------------------
    // End Resize fonts
    //---------------------------------------------------------------
    // Dark style
    // This is the winner as far as default color schemes
    ImGui::StyleColorsDark();
    // Classic light style
    //ImGui::StyleColorsClassic();
    // Light style
    //ImGui::StyleColorsLight();
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
//------------------------------------------------------------------------
// End Graphical Backend functions
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// General GUI functions
//------------------------------------------------------------------------
void glfw_error_callback(int error, const char *description)
{
    std::cerr << "GLFW Error " << error << ": " << description << '\n'; 
}

void prep_newframe()
{
    // Check for user input (mouse, keyboard, etc)
    glfwPollEvents();
    // Tell the backends to prepare for a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    // Tell Dear ImGui to prepare for a new frame
    ImGui::NewFrame();
}

void finish_newframe(GLFWwindow* window, ImVec4 clear_color)
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
//------------------------------------------------------------------------
// End General GUI functions
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Checkpoint data (inside thread_pool)
//------------------------------------------------------------------------
// This deadlocks on exactly one file sometimes when waiting to lock the sac_ptr->mutex_
void checkpoint_data(ProgramStatus& program_status, const int data_id, const int checkpoint_id)
{
    if (!program_status.project.is_project) { return; }
    std::shared_ptr<sac_1c> sac_ptr{program_status.data_pool.get_ptr(program_status.project, data_id, checkpoint_id)};
    if (!sac_ptr) { return; }
    std::lock_guard<std::shared_mutex> lock_sac(sac_ptr->mutex_);
    program_status.state.store(out);
    {
        program_status.project.add_data_checkpoint(sac_ptr->sac, data_id, true);
        std::unordered_map<std::string, std::string> checkpoint_metadata{program_status.project.get_current_checkpoint_metadata()};
        std::ostringstream oss{};
        oss << "CHECKPOINT; NAME " << checkpoint_metadata["name"] << "; CREATED: " << checkpoint_metadata["created"] << ";";
        std::string checkpoint_string{oss.str()};
        program_status.project.add_data_processing(program_status.project.sq3_connection_file, data_id, checkpoint_string.c_str());
    }
    ++program_status.tasks_completed;
}
//------------------------------------------------------------------------
// End checkpoint data (inside thread_pool)
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Unload data from memory
//------------------------------------------------------------------------
void unload_data(ProgramStatus& program_status)
{
    program_status.state.store(in);
    program_status.tasks_completed = 0;
    program_status.total_tasks = 2;
    // Remove the SQLite3 connections and the file paths
    program_status.project.unload_project();
    ++program_status.tasks_completed;
    ++program_status.tasks_completed;
}
//------------------------------------------------------------------------
// End Unload data from memory
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Load a single bit of data to the data pool
//------------------------------------------------------------------------
void load_2_data_pool(ProgramStatus& program_status, const int data_id)
{
    program_status.data_pool.add_data(program_status.project, data_id, program_status.project.checkpoint_id_);
    ++program_status.tasks_completed;
}
//------------------------------------------------------------------------
// End Load a single bit of data to the data pool
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Load a project from a project file
//------------------------------------------------------------------------
void load_data(ProgramStatus& program_status, const std::filesystem::path project_file, int checkpoint_id)
{
    {
        std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
        program_status.state.store(in);
        program_status.tasks_completed = 0;
        program_status.total_tasks = 3;
    }
    // First make sure we unload the present project
    unload_data(program_status);
    ++program_status.tasks_completed;
    // Connection to the project file
    program_status.project.connect_2_existing(project_file);
    if (checkpoint_id == -1){ checkpoint_id = program_status.project.get_latest_checkpoint_id(); }
    // Set the checkpoint id to the latest checkpoint
    program_status.project.set_checkpoint_id(checkpoint_id);
    // Get the data-ids to load
    program_status.project.current_data_ids = program_status.project.get_data_ids_for_current_checkpoint();
    program_status.project.updated = true;
    std::size_t total_ids{program_status.project.current_data_ids.size()};
    std::size_t to_load{};
    ++program_status.tasks_completed;
    if (program_status.data_pool.max_data < total_ids)
    {
        to_load = program_status.data_pool.max_data;
    }
    else
    {
        to_load = total_ids;
    }
    {
        std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
        program_status.total_tasks = to_load;
        program_status.tasks_completed = 0;
        program_status.state.store(in);
        // Empty the fucking pool man!
        program_status.data_pool.empty_pool();
    }
    // If we have space to load more data
    for (std::size_t i{0}; i < to_load; ++i)
    {
        program_status.thread_pool.enqueue(load_2_data_pool, std::ref(program_status), program_status.project.current_data_ids[i]);
    }
}
//------------------------------------------------------------------------
// End load a project from a project file
//------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Shitty Butterworth lowpass filter for testing (not correct, but useful)
//-----------------------------------------------------------------------------
void lowpass(FFTWPlanPool& plan_pool, std::shared_ptr<sac_1c> sac_ptr, int order, double cutoff)
{
    // Do the fft
    std::vector<std::complex<double>> complex_spectrum = fft_time_series(plan_pool, sac_ptr->sac.data1);
    double frequency{};
    const double sampling_freq{1.0 / sac_ptr->sac.delta};
    const double freq_step{sampling_freq / complex_spectrum.size()};
    double denominator{};
    double gain{};
    for (std::size_t i{0}; i < complex_spectrum.size(); ++i)
    {
        frequency = i * freq_step;
        denominator = std::pow(frequency / cutoff, 2.0 * order);
        denominator = std::sqrt(1.0 + denominator);
        gain = 1.0 / denominator;
        complex_spectrum[i] *= gain;
    }
    // Do the ifft
    sac_ptr->sac.data1 = pssp::ifft_spectrum(plan_pool, complex_spectrum);
}
//-----------------------------------------------------------------------------
// End Shitty Butterworth lowpass filter for testing (not correct, but useful)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Shitty Butterworth highpass filter for testing (not correct, but useful)
//-----------------------------------------------------------------------------
void highpass(FFTWPlanPool& plan_pool, std::shared_ptr<sac_1c> sac_ptr, int order, double cutoff)
{
    // Do the fft
    std::vector<std::complex<double>> complex_spectrum = fft_time_series(plan_pool, sac_ptr->sac.data1);
    double frequency{};
    const double sampling_freq{1.0 / sac_ptr->sac.delta};
    const double freq_step{sampling_freq / complex_spectrum.size()};
    double denominator{};
    double gain{};
    for (std::size_t i{0}; i < complex_spectrum.size(); ++i)
    {
        frequency = i * freq_step;
        denominator = std::pow(cutoff / frequency, 2.0 * order);
        denominator = std::sqrt(1.0 + denominator);
        gain = 1.0 / denominator;
        complex_spectrum[i] *= gain;
    }
    // Do the ifft
    sac_ptr->sac.data1 = pssp::ifft_spectrum(plan_pool, complex_spectrum);
}
//-----------------------------------------------------------------------------
// End Shitty Butterworth highpass filter for testing (not correct, but useful)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Shitty Butterworth bandpass filter for testing (not correct, but useful)
//-----------------------------------------------------------------------------
void bandpass(FFTWPlanPool& plan_pool, std::shared_ptr<sac_1c> sac_ptr, int order, double lowpass, double highpass)
{
    // Do the fft
    std::vector<std::complex<double>> complex_spectrum = fft_time_series(plan_pool, sac_ptr->sac.data1);
    double frequency{};
    const double sampling_freq{1.0 / sac_ptr->sac.delta};
    const double freq_step{sampling_freq / complex_spectrum.size()};
    double denominator_lp{};
    double denominator_hp{};
    double denominator{};
    double gain{};
    for (std::size_t i{0}; i < complex_spectrum.size(); ++i)
    {
        frequency = i * freq_step;
        denominator_lp = std::pow(frequency / highpass, 2.0 * order);
        denominator_lp = std::sqrt(1.0 + denominator_lp);
        denominator_hp = std::pow(lowpass / frequency, 2.0 * order);
        denominator_hp = std::sqrt(1.0 + denominator_hp);
        denominator = denominator_lp * denominator_hp;
        gain = 1.0 / denominator;
        complex_spectrum[i] *= gain;
    }
    // Do the ifft
    sac_ptr->sac.data1 = pssp::ifft_spectrum(plan_pool, complex_spectrum);
}
//-----------------------------------------------------------------------------
// End Shitty Butterworth bandpass filter for testing (not correct, but useful)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Shitty Butterworth bandreject filter for testing (not correct, but useful)
//-----------------------------------------------------------------------------
void bandreject(FFTWPlanPool& plan_pool, std::shared_ptr<sac_1c> sac_ptr, int order, double lowreject, double highreject)
{
    (void) plan_pool;
    (void) sac_ptr;
    (void) order;
    (void) lowreject;
    (void) highreject;
    // To be implemented eventually
    return;
}
//-----------------------------------------------------------------------------
// End Shitty Butterworth bandreject filter for testing (not correct, but useful)
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Write a checkpoint
//-----------------------------------------------------------------------------
void write_checkpoint(ProgramStatus& program_status, bool author, bool cull)
{
    std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
    program_status.state.store(out);
    std::vector<int> id_input = program_status.data_pool.get_iter(program_status.project);
    program_status.total_tasks = static_cast<int>(id_input.size());
    program_status.tasks_completed = 0;
    int parent_checkpoint_id{program_status.project.checkpoint_id_};
    // Add a checkpoint to the list (made by user)
    // This step updates the checkpoint id, se we need to pass the old checkpoint id
    program_status.project.write_checkpoint(author, cull);
    // Checkpoint each piece of data
    for (std::size_t i{0}; i < id_input.size(); ++i)
    {
        program_status.thread_pool.enqueue(checkpoint_data, std::ref(program_status), id_input[i], parent_checkpoint_id);
    }
}
//-----------------------------------------------------------------------------
// End Write a checkpoint
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Delete a checkpoint
//-----------------------------------------------------------------------------
void delete_data_id_checkpoint(ProgramStatus& program_status, Project& project, int checkpoint_id, int data_id)
{
    project.delete_data_id_checkpoint(data_id, checkpoint_id);
    ++program_status.tasks_completed;
}

void delete_checkpoint(ProgramStatus& program_status, Project& project, int checkpoint_id)
{
    project.delete_checkpoint_from_list(checkpoint_id);
    std::vector<int> data_ids{project.get_data_ids()};
    {
        std::lock_guard<std::shared_mutex> lock_program(program_status.program_mutex);
        program_status.state.store(out);
        program_status.total_tasks = static_cast<int>(data_ids.size());
        program_status.tasks_completed = 0;
    }
    // Clear the actual data values
    for (int data_id : data_ids)
    {
        program_status.thread_pool.enqueue(delete_data_id_checkpoint, std::ref(program_status), std::ref(project), checkpoint_id, data_id);
    }
    // Tell SQLite3 to issue vacuum on closing database
    //project.vacuum();
}
//-----------------------------------------------------------------------------
// End Delete a checkpoint
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// End Misc functions
//-----------------------------------------------------------------------------
}
