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
debug_imgui = -fanalyzer -Wextra -Wsign-conversion -Werror -Wshadow -ggdb
# Release params only if debug is false
release_param = -O2 -DNDEBUG
# Debug (big/slow/strict) or Release (small/fast/relaxed)
debug = true

ifeq ($(debug), true)
	params = $(param) $(debug_param)
	params_imgui = $(param) $(debug_imgui)
else
	params = $(param) $(release_param)
	params_imgui = $(param) $(debug_imgui)
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
# sac-format
sf_lib = $(CURDIR)/sac-format/lib/
sf_header = $(CURDIR)/sac-format/src/header/
#------------------------------------------------------------------------------
# End directory structure
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# FFTW
#------------------------------------------------------------------------------
# Note that FFTW is optional and only used for spectral functions
# If you don't want to use the spectral functions don't worry about this
# If you do, you'll need to setup fftw_params according to your installation
# Spectral = FFT, IFFT, Filters (lowpass, highpass, bandpass)
ifeq ($(uname_s), Linux)
	fftw_include := /usr/include/
	fftw_lib := /usr/lib/x86_64-linux-gnu/
else
	fftw_loc := /opt/homebrew/Cellar/fftw/3.3.10_1/
	fftw_include := $(fftw_loc)include/
	fftw_lib := $(fftw_loc)lib/
endif
fftw_params = -I$(fftw_include) -L$(fftw_lib) -lfftw3 -lm
#------------------------------------------------------------------------------
# End FFTW
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Dear ImGui
#------------------------------------------------------------------------------
# Note you'll need to compile Dear ImGui yourself
# How I do it:
# Go to `$(imgui_dir)examples/example_glfw_opengl3/`
# run `make`
# Copy all the object files into an object directory
# `mkdir objects`
# `mv *.o ./objects/`
# `mv ./objects $(imgui_dir)`
# Note that on MacOS, Dear ImGui compiles with Clang (not G++)
# However, I can still use those object files to compile my codes with G++
# Just cannot use -Weffc++ on programs that depend on Dear ImGui
# ImGui directory, needed for GUI
imgui_dir = $(CURDIR)/../imgui-1.89.5/

ifeq ($(uname_s), Darwin)
	imgui_libs = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	imgui_libs += -L/usr/local/lib -L/opt/homebrew/lib
	imgui_libs += -lglfw
	imgui_flags = -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include
else
	imgui_libs = -lGL `pkg-config --static --libs glfw3`
	imgui_flags = `pkg-config --cflags glfw3`
endif

imgui_params = $(imgui_flags) $(imgui_libs)
imgui_cxx = g++-12 $(params_imgui) -I$(imgui_dir) -I$(imgui_dir)backends
#------------------------------------------------------------------------------
# End Dear ImGui
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Tiny file dialogs
#------------------------------------------------------------------------------
# Version 3.13
# https://sourceforge.net/projects/tinyfiledialogs/
# This is what I use for OS-independent file dialogs
# To compile tfd:
# `cd tfd_dir`
# `gcc -c tinyfiledialogs.c -o tinyfiledialogs.o`
# We then link to the `tinyfiledialogs.o` file to
# use the OS-default file dialog without needing to
# worry about which OS is being used on our end
tfd_dir = $(CURDIR)/../tinyfiledialogs/
imgui_params += -I$(tfd_dir) $(tfd_dir)tinyfiledialogs.o -I$(hdr_prefix)
#------------------------------------------------------------------------------
# End tiny file dialogs
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Include my sac headers
#------------------------------------------------------------------------------
# Compilation command with inclusion of my headers
cxx := $(cxx) -I$(hdr_prefix)
#------------------------------------------------------------------------------
# End include my sac headers
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
$(spectral_sac): %:$(test_prefix)%.cpp $(spectral_modules)
	@echo "Building $(test_bin_prefix)$@"
	@echo "Build start:  $$(date)"
	@test -d $(test_bin_prefix) || mkdir -p $(test_bin_prefix)
	$(cxx) -L$(sf_lib) -lsac_format -I$(sf_header) -o $(test_bin_prefix)$@ $< $(spectral_obj) $(fftw_params)
	@echo -e "Build finish: $$(date)\n"

# imgui_srcs are all needed
imgui_srcs := $(imgui_dir)imgui.cpp $(imgui_dir)imgui_draw.cpp $(imgui_dir)imgui_tables.cpp $(imgui_dir)imgui_widgets.cpp
# GUI backends
imgui_srcs += $(imgui_dir)backends/imgui_impl_glfw.cpp $(imgui_dir)backends/imgui_impl_opengl3.cpp
# Name of ImGui object files
imgui_raw_objs = $(addsuffix .o, $(basename $(notdir $(imgui_srcs))))
# Where they exist in our build
imgui_objs = $(addprefix $(imgui_dir)objects/, $(imgui_raw_objs))

imgui_test: $(test_prefix)imgui_test.cpp $(imgui_objs) $(stream_modules)
	@test -d $(test_bin_prefix) || mkdir -p $(test_bin_prefix)
	$(imgui_cxx) -L$(sf_lib) -lsac_format -I$(sf_header) -o $(test_bin_prefix)$@ $< $(imgui_objs) $(imgui_params) $(stream_obj)
#------------------------------------------------------------------------------
# End compilation patterns
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Cleanup
#------------------------------------------------------------------------------
clean:
	rm -rf $(bin_prefix) $(obj_prefix) *.dSYM *.csv
#------------------------------------------------------------------------------
# End cleanup
#------------------------------------------------------------------------------
