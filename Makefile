#------------------------------------------------------------------------------
# We need to know what OS we're on as it determines which compiler we use (and 
# therefore which compiler parameters are appropriate) and how we link to the
# necessary libraries
#------------------------------------------------------------------------------
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
#
# This uses the thread-sanitizer, useful for finding data-races
# though it also flags many false positives (better than missing
# real positives)
#common_debug = -fsanitize=thread -pthread -Wextra -Werror -Wshadow -ggdb
#
# This does not use the thread-sanitizer (no pthreads needed either)
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
#
# This is middle of the road, safe optimizations, quick build
#release_param = -O2 -DNDEBUG
#
# This is the most optimized without doing fine-grain optimizations, while
# not using fast-math, so if you're worried about any numerical inaccuracy made by the
# use of fast-math, then swap over to this level of optimization
# I think -O3 is the best choice for now. -Ofast has caused some data-corruption twice in a few tests
# both in terms of locking the GUI, but also in terms of completely losing the time-series data
# after applying a filter.
release_param = -O3 -DNDEBUG
#
# This is fast and dangerous. It does not play nice with FFTW3 resulting in data corruption.
#release_param = -Ofast -DNDEBUG
#
# This is supposed to make the smallest binary possible, it barely does anything to our size
# probably because we're super small anyway
#release_param = -Oz -DNDEBUG

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
# Where the source code files for PsSp are stored
code_prefix = $(base_prefix)code/
# Where experimental code lives
exp_prefix = $(code_prefix)experiment/
# Where experimental programs will go
exp_bin_prefix = $(bin_prefix)experiment/
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
# Dear ImGui provides the OS-independent GUI framework
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
	boost_dir = /opt/homebrew/Cellar/boost/1.82.0/
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
# Include my implementations
#------------------------------------------------------------------------------
my_imp_files := $(wildcard $(imp_prefix)*.cpp)
#------------------------------------------------------------------------------
# End Include my implementations
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Program definitions
#------------------------------------------------------------------------------
# All programs
all: PsSp

# Not necessary for MacOS, but necessary if you want a free floating application
# on MacOS
macos: PsSp.app

# Experimental programs
exp: tree_exp
#------------------------------------------------------------------------------
# End program definitions
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Compilation patterns
#------------------------------------------------------------------------------

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
# End Dear ImGui example (glfw+opengl3)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# PsSp
#------------------------------------------------------------------------------
pssp_param_list = -I$(hdr_prefix) -I$(sf_header) $(sf_obj) $(imgui_objs) $(imgui_dir)misc/cpp/imgui_stdlib.cpp $(im_file_diag_dir)ImGuiFileDialog.o $(implot_dir)implot.cpp 
pssp_param_list += $(implot_dir)implot_items.cpp $(my_imp_files) $(imgui_params) -lsqlite3 $(fftw_params) $(boost_params) $(msgpack_params) 
PsSp: $(code_prefix)main.cpp $(imgui_objs) $(im_file_diag_dir)ImGuiFileDialog.o $(sf_obj)
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
# NTreeNode experimentation
#------------------------------------------------------------------------------
tree_exp: $(exp_prefix)tree_exp.cpp
	@echo "Building $@"
	@echo "Build start:  $$(date)"
	@test -d $(exp_bin_prefix) || mkdir -p $(exp_bin_prefix)
	$(cxx) -o $(exp_bin_prefix)$@ $< $(imp_prefix)pssp_data_trees.cpp
	@echo -e "Build finish: $$(date)\n"
#------------------------------------------------------------------------------
# End NTreeNode experimentation
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# End compilation patterns
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Cleanup
#------------------------------------------------------------------------------
clean:
	rm -rf $(bin_prefix) $(obj_prefix) *.dSYM $(im_file_diag_dir)ImGuiFileDialog.o $(imgui_dir)objects/ $(imgui_ex_dir)example_glfw_opengl3 *.ini *.csv *.msgpack *.db
	make -C $(sf_dir) clean
#------------------------------------------------------------------------------
# End cleanup
#------------------------------------------------------------------------------
