#===============================================================================
# Basic Setup
#===============================================================================
cmake_minimum_required(VERSION 3.20)
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
#===============================================================================
# Dependencies
#===============================================================================
#-------------------------------------------------------------------------------
# Fetch
#-------------------------------------------------------------------------------
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
#-------------------------------------------------------------------------------
# Include
#-------------------------------------------------------------------------------
# fltk must be installed separately
find_package(FLTK CONFIG REQUIRED)
#===============================================================================
# PsSp project
#===============================================================================
project(PsSp
  LANGUAGES CXX
  DESCRIPTION "Passive-source Seismic-processing"
  HOMEPAGE_URL https://arbCoding.github.io/PsSp
  VERSION 0.2.0)

include_directories(${PsSp_SOURCE_DIR}/src)
#-------------------------------------------------------------------------------
# PsSp program
#-------------------------------------------------------------------------------
add_executable(PsSp
  src/main.cpp
  src/Windows/Main.cpp
  src/Windows/Welcome.cpp
  src/Application/Application.cpp
)

if (APPLE)
  target_link_libraries(PsSp PRIVATE "-framework cocoa")
endif()

# System prevents warnings from fltk!
target_include_directories(PsSp SYSTEM PRIVATE ${FLTK_INCLUDE_DIRS})
target_link_libraries(PsSp PRIVATE fltk)
