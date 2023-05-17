#------------------------------------------------------------------------------
# Bugs
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# End bugs
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# We need to know what OS we're on as it determines which compiler we use (and 
# therefore which compiler parameters are appropriate) and how we link to the
# necessary libraries
#------------------------------------------------------------------------------
# 
# Use the correct shell for bash scripts
# seemed to default to /bin/sh when I use /bin/bash
SHELL := /bin/bash
# Linux or mac
uname_s := $(shell uname -s)
# Debug mode or release mode
debug = true
#------------------------------------------------------------------------------
# Setup compiler
#------------------------------------------------------------------------------
# Param is always used
param = -std=c++20 -pedantic-errors -Wall
# Common debug params regardless of clang++ or g++
common_debug = -Wextra -Werror -Wshadow -ggdb
# Slightly different between MacOS and Linux
ifeq ($(uname_s), Darwin)
  compiler = clang++
	debug_param = $(common_debug) -Wsign-conversion -Weffc++
else
  compiler = g++-12
	debug_param = $(common_debug) -fanalyzer -Wsign-conversion -Weffc++
endif

# Specific to Dear ImGui
debug_imgui = $(common_debug)
# Release params only if debug is false
release_param = -O2 -DNDEBUG

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
# Project directory structure
# Code base starts here
base_prefix = $(CURDIR)/src/
# Built programs will go here
bin_prefix = $(CURDIR)/bin/
# Test programs will go here
test_bin_prefix = $(bin_prefix)tests/
# Where the source code files for tests are stored
test_prefix = $(base_prefix)tests/
# Where the source code files for PsSp are stored
code_prefix = $(base_prefix)code/
# Where header (interface) files are stored
hdr_prefix = $(base_prefix)header/
# Where the source code (implementation) files are stored
imp_prefix = $(base_prefix)implementation/
# Built object files will go here
obj_prefix = $(base_prefix)objects/
# Submodules directory
submod_prefix = $(CURDIR)/submodules/
# Font directory
font_dir = $(CURDIR)/fonts/
# Chosen font
chosen_font = $(font_dir)Hack/HackNerdFontMono-Regular.ttf
# MacOs exclusive directory (Info.plist)
# For making the PsSp.app for MacOS only!
macos_exc_dir = $(CURDIR)/macos_application/
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
#imgui_cxx = g++-12 $(params_imgui) -I$(imgui_dir) -I$(imgui_dir)backends
imgui_cxx = $(compiler) $(params_imgui) -I$(imgui_dir) -I$(imgui_dir)backends
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
#imgui_file_cxx = g++-12 $(param) $(release_param) -I$(imgui_dir) -I$(imgui_dir)backends
imgui_file_cxx = $(compiler) $(param) $(release_param) -I$(imgui_dir) -I$(imgui_dir)backends
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
# Boost
#------------------------------------------------------------------------------
ifeq ($(uname_s), Darwin)
	boost_dir = /opt/homebrew/Cellar/boost/1.81.0_1/
	boost_inc = $(boost_dir)include/
	boost_lib = $(boost_dir)lib/
else
	boost_inc = /usr/include/
	boost_lib = /usr/lib/x86_64-linux-gnu/
endif
boost_params = -I$(boost_inc) -L$(boost_lib)
#------------------------------------------------------------------------------
# End Boost
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# MessagePack
#------------------------------------------------------------------------------
ifeq ($(uname_s), Darwin)
	msgpack_dir = /opt/homebrew/Cellar/msgpack-cxx/6.0.0/
	msgpack_inc = $(msgpack_dir)include/
	msgpack_lib = $(msgpack_dir)lib/
	msgpack_params = -I$(msgpack_inc) -L$(msgpack_lib)
else
	msgpack_params =`pkg-config --cflags --libs msgpack` 
endif
#------------------------------------------------------------------------------
# End MessagePack
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
all: PsSp

# Not necessary for MacOS, but necessary if you want a free floating application
# on MacOS
macos: PsSp.app

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
$(obj_prefix)sac_spectral.o: $(imp_prefix)sac_spectral.cpp
	@echo "Building $@"
	@echo "Build start:  $$(date)"
	@test -d $(obj_prefix) || mkdir -p $(obj_prefix)
	$(cxx) -c -o $@ $< -I$(sf_header) $(fftw_include)
	@echo -e "Build finish: $$(date)\n"

spectral_sac := sac_stream_fftw_test sac_stream_lowpass_test
$(spectral_sac): %:$(test_prefix)%.cpp $(obj_prefix)sac_spectral.o $(sf_obj)
	@echo "Building $(test_bin_prefix)$@"
	@echo "Build start:  $$(date)"
	@test -d $(test_bin_prefix) || mkdir -p $(test_bin_prefix)
	$(cxx) -I$(sf_header) -o $(test_bin_prefix)$@ $< $(sf_obj) $(obj_prefix)sac_spectral.o $(fftw_params)
	@echo -e "Build finish: $$(date)\n"

#------------------------------------------------------------------------------
# ImGuiFileDialog
#------------------------------------------------------------------------------
#ImGuiFileDialog: $(im_file_diag_dir)ImGuiFileDialog.cpp
$(im_file_diag_dir)ImGuiFileDialog.o: $(im_file_diag_dir)ImGuiFileDialog.cpp
	@echo "Building $@"
	@echo "Build start:  $$(date)"
	$(imgui_file_cxx) -c -o $@ $<
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
imgui_test: $(test_prefix)imgui_test.cpp $(imgui_objs) $(im_file_diag_dir)ImGuiFileDialog.o $(sf_obj)
	@echo "Building $@"
	@echo "Build start:  $$(date)"
	@test -d $(test_bin_prefix) || mkdir -p $(test_bin_prefix)
	$(imgui_cxx) -I$(sf_header) -o $(test_bin_prefix)$@ $< $(sf_obj) $(imgui_objs) $(im_file_diag_dir)ImGuiFileDialog.o $(imgui_params) $(implot_dir)implot.cpp $(implot_dir)implot_items.cpp
	@echo -e "Build finish: $$(date)\n"
#------------------------------------------------------------------------------
# end imgui_test
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# PsSp
#------------------------------------------------------------------------------
pssp_param_list = -I$(hdr_prefix) -I$(sf_header) $(sf_obj) $(imgui_objs) $(im_file_diag_dir)ImGuiFileDialog.o $(imgui_params)
pssp_param_list += $(implot_dir)implot.cpp $(implot_dir)implot_items.cpp $(obj_prefix)sac_spectral.o $(fftw_params)
pssp_param_list += $(boost_params) $(msgpack_params) $(imp_prefix)pssp_projects.cpp
PsSp: $(code_prefix)main.cpp $(imgui_objs) $(im_file_diag_dir)ImGuiFileDialog.o $(sf_obj) $(obj_prefix)sac_spectral.o
	@echo "Building $@"
	@echo "Build start:  $$(date)"
	@test -d $(bin_prefix) || mkdir -p $(bin_prefix)
	$(imgui_cxx) -o $(bin_prefix)$@ $< $(pssp_param_list)
	@cp $(chosen_font) $(bin_prefix)
	@test imgui.ini && rm -f imgui.ini
	@echo -e "Build finish: $$(date)\n"
#------------------------------------------------------------------------------
# end PsSp
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# PsSp.app (MacOS only)
#------------------------------------------------------------------------------
PsSp.app: PsSp
	@echo "Building $@"
	@echo "Build start:  $$(date)"
	@test -d $(bin_prefix)$@ && rm -r $(bin_prefix)$@ || true
	@mkdir -p $(bin_prefix)$@/Contents/MacOS
	@mkdir -p $(bin_prefix)$@/Contents/Resources
	@cp $(macos_exc_dir)Info.plist $(bin_prefix)$@/Contents/Info.plist
	@cp $(bin_prefix)$< $(bin_prefix)$@/Contents/MacOS/$<
	@cp $(chosen_font) $(bin_prefix)$@/Contents/MacOS/
	@dylibbundler -s /opt/homebrew/lib -od -b -x $(bin_prefix)$@/Contents/MacOS/$< -d $(bin_prefix)$@/Contents/libs/
	@echo -e "Build finish: $$(date)\n"
#------------------------------------------------------------------------------
# End PsSp.app (MacOs only)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# End compilation patterns
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Cleanup
#------------------------------------------------------------------------------
clean:
	rm -rf $(bin_prefix) $(obj_prefix) *.dSYM $(im_file_diag_dir)ImGuiFileDialog.o $(imgui_dir)objects/ $(imgui_ex_dir)example_glfw_opengl3 *.ini *.csv *.msgpack
	make -C $(sf_dir) clean
#------------------------------------------------------------------------------
# End cleanup
#------------------------------------------------------------------------------
