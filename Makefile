#------------------------------------------------------------------------------
# Setup compiler
#------------------------------------------------------------------------------
# Use the correct shell for bash scripts
# seemed to default to /bin/sh when I use /bin/bash
SHELL := /bin/bash
# Compiler
compiler = g++-12
# Param is always used
param = -std=c++20 -pedantic-errors -Wall
# Debug params only if debug is true
# G++
# This is needed to Dear ImGui only, as it cannot compile with G++ on MacOs
debug_param = -fanalyzer -Weffc++ -Wextra -Wsign-conversion -Werror -Wshadow -ggdb
# Specific for Dear ImGui
# Dear ImGui fails with -Weffc++ unfortuantely
#debug_imgui = -fanalyzer -Wextra -Wsign-conversion -Werror -Wshadow -ggdb
# ImPlot doesn't jive with -Wsign-conversion
debug_imgui = -fanalyzer -Wextra -Werror -Wshadow -ggdb
# Release params only if debug is false
release_param = -O2 -DNDEBUG
# Debug (big/slow/strict) or Release (small/fast/relaxed)
debug = false

ifeq ($(debug), true)
	params = $(param) $(debug_param)
	params_imgui = $(param) $(debug_imgui)
else
	params = $(param) $(release_param)
	params_imgui = $(param) $(release_param)
endif
# Compilation command
cxx := $(compiler) $(params)
#------------------------------------------------------------------------------
# End setting up compiler
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Directory structure
#------------------------------------------------------------------------------
# Linux or mac
uname_s := $(shell uname -s)
# Project directory structure
# Code base starts here
base_prefix = $(CURDIR)/src/
# Built programs will go here
bin_prefix = $(CURDIR)/bin/
# Test programs will go here
test_bin_prefix = $(bin_prefix)tests/
# Where the source code files for tests are stored
test_prefix = $(base_prefix)tests/
# Where header (interface) files are stored
hdr_prefix = $(base_prefix)header/
# Where the source code (implementation) files are stored
imp_prefix = $(base_prefix)implementation/
# Built object files will go here
obj_prefix = $(base_prefix)objects/
# Submodules directory
submod_prefix = $(CURDIR)/submodules/
#------------------------------------------------------------------------------
# End directory structure
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# sac-format
#------------------------------------------------------------------------------
# My sac-format files for reading/writing binary seismic data
sf_dir = $(submod_prefix)sac-format/
sf_obj_prefix = $(sf_dir)src/objects/
sf_obj = $(sf_obj_prefix)sac_format.o
sf_header = $(sf_dir)src/header/
#------------------------------------------------------------------------------
# End sac-format
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# FFTW
#------------------------------------------------------------------------------
# Same between MacOS and Linux
fftw_include = `pkg-config --cflags fftw3`
fftw_lib = `pkg-config --static --libs fftw3`
fftw_params = $(fftw_include) $(fftw_lib)
#------------------------------------------------------------------------------
# End FFTW
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Dear ImGui
#------------------------------------------------------------------------------
# Dead ImGui provides the OS-independent GUI framework
imgui_dir = $(submod_prefix)imgui/
imgui_ex_dir = $(imgui_dir)examples/example_glfw_opengl3/

# On both MacOS and Linux we can use pkg-config to deal with *most* of this
imgui_flags = `pkg-config --cflags glfw3`
imgui_libs = `pkg-config --static --libs glfw3`

# Slightly different between MacOS and Linux
ifeq ($(uname_s), Darwin)
	imgui_libs += -framework OpenGL
else
	imgui_libs += -lGL
endif

imgui_params = $(imgui_flags) $(imgui_libs)
imgui_cxx = g++-12 $(params_imgui) -I$(imgui_dir) -I$(imgui_dir)backends
#------------------------------------------------------------------------------
# End Dear ImGui
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# ImGuiFileDialog
#------------------------------------------------------------------------------
# ImGuiFileDialog adds a Filesystem Acess GUI that is OS-independent and works
# great with Dear ImGui
im_file_diag_dir = $(submod_prefix)ImGuiFileDialog/
imgui_params += -I$(im_file_diag_dir)
imgui_file_cxx = g++-12 $(param) $(release_param) -I$(imgui_dir) -I$(imgui_dir)backends
#------------------------------------------------------------------------------
# End ImGuiFileDialog
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# ImPlot
#------------------------------------------------------------------------------
implot_dir = $(submod_prefix)implot/
# This library triggers sign-conversion warnings. So we
# should treat as a system header and ignore warnings
# That way we're only confirming for out code
# If this is an issue later, we can include it normally to track down
# IT WORKS
imgui_cxx += -isystem$(implot_dir)
#------------------------------------------------------------------------------
# End ImPlot
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Include my headers
#------------------------------------------------------------------------------
# Compilation command with inclusion of my headers
cxx := $(cxx) -I$(hdr_prefix)
#------------------------------------------------------------------------------
# End include my headers
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Program definitions
#------------------------------------------------------------------------------
# All programs
all: tests

# These need sac_format.o and FFTW
sac_spectral_tests: sac_stream_fftw_test sac_stream_lowpass_test

# All tests
tests: sac_spectral_tests imgui_test
#------------------------------------------------------------------------------
# End program definitions
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Compilation patterns
#------------------------------------------------------------------------------
# By splitting into .o files I can make it so that only newly written code gets compiled
# Therefore cutting down on compilation times
# Also helps to simply the logic a little bit
sac_spectral: $(imp_prefix)sac_spectral.cpp
	@echo "Building $@"
	@echo "Build start:  $$(date)"
	@test -d $(obj_prefix) || mkdir -p $(obj_prefix)
	$(cxx) -c -o $(obj_prefix)$@.o $< -I$(sf_header) $(fftw_params)
	@echo -e "Build finish: $$(date)\n"

spectral_modules := sac_spectral
spectral_obj := $(addsuffix .o, $(addprefix $(obj_prefix), $(spectral_modules)))
spectral_sac := sac_stream_fftw_test sac_stream_lowpass_test
$(spectral_sac): %:$(test_prefix)%.cpp $(spectral_modules) $(sf_obj)
	@echo "Building $(test_bin_prefix)$@"
	@echo "Build start:  $$(date)"
	@test -d $(test_bin_prefix) || mkdir -p $(test_bin_prefix)
	$(cxx) -I$(sf_header) -o $(test_bin_prefix)$@ $< $(sf_obj) $(spectral_obj) $(fftw_params)
	@echo -e "Build finish: $$(date)\n"

#------------------------------------------------------------------------------
# ImGuiFileDialog
#------------------------------------------------------------------------------
ImGuiFileDialog: $(im_file_diag_dir)ImGuiFileDialog.cpp
	@echo "Building $@"
	@echo "Build start:  $$(date)"
	$(imgui_file_cxx) -c -o $(im_file_diag_dir)$@.o $<
	@echo -e "Building finish: $$(date)\n"
#------------------------------------------------------------------------------
# End ImGuiFileDialog
#------------------------------------------------------------------------------

# imgui_srcs are all needed
imgui_srcs := $(imgui_dir)imgui.cpp $(imgui_dir)imgui_draw.cpp $(imgui_dir)imgui_tables.cpp $(imgui_dir)imgui_widgets.cpp
# GUI backends
imgui_srcs += $(imgui_dir)backends/imgui_impl_glfw.cpp $(imgui_dir)backends/imgui_impl_opengl3.cpp
imgui_file_srcs = $(im_file_diag_dir)ImGuiFileDialog.cpp
# Name of ImGui object files
imgui_raw_objs = $(addsuffix .o, $(basename $(notdir $(imgui_srcs))))
# Where they exist in our build
imgui_objs = $(addprefix $(imgui_dir)objects/, $(imgui_raw_objs))

#------------------------------------------------------------------------------
# sac-format
#------------------------------------------------------------------------------
$(sf_obj): $(sf_dir)Makefile
	@echo "Building sac-format stuff"
	@echo "Build start:  $$(date)"
	make -C $(sf_dir) sac_format
#------------------------------------------------------------------------------
# end sac-format
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# The appropriate Dear ImGui example (glfw+opengl3)
#------------------------------------------------------------------------------
$(imgui_objs): $(imgui_ex_dir)Makefile
	@echo "Building Dear ImGui stuff"
	@echo "Build start:  $$(date)"
	make -C $(imgui_ex_dir)
	@test -d $(imgui_dir)objects || mkdir -p $(imgui_dir)objects
	@echo "Moving object files to $(imgui_dir)objects/"
	@mv $(imgui_ex_dir)*.o $(imgui_dir)objects/
	@echo -e "Build finish: $$(date)\n"
#------------------------------------------------------------------------------
# End Dead ImGui example (glfw+opengl3)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# imgui_test
#------------------------------------------------------------------------------
imgui_test: $(test_prefix)imgui_test.cpp $(imgui_objs) ImGuiFileDialog $(stream_modules)
	@echo "Building $@"
	@echo "Build start:  $$(date)"
	@test -d $(test_bin_prefix) || mkdir -p $(test_bin_prefix)
	$(imgui_cxx) -I$(sf_header) -o $(test_bin_prefix)$@ $< $(sf_obj) $(imgui_objs) $(imgui_file_objs) $(im_file_diag_dir)ImGuiFileDialog.o $(imgui_params) $(implot_dir)implot.cpp $(implot_dir)implot_items.cpp $(stream_obj)
	@echo -e "Build finish: $$(date)\n"
#------------------------------------------------------------------------------
# end imgui_test
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# End compilation patterns
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Cleanup
#------------------------------------------------------------------------------
clean:
	rm -rf $(bin_prefix) $(obj_prefix) *.dSYM $(im_file_diag_dir)ImGuiFileDialog.o $(imgui_dir)objects/ $(imgui_ex_dir)example_glfw_opengl3 *.ini
	make -C $(sf_dir) clean
#------------------------------------------------------------------------------
# End cleanup
#------------------------------------------------------------------------------
