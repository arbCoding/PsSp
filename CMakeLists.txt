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
# Logging
FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG v1.12.0)
# GUI
set(FLTK_BUILD_TEST OFF CACHE BOOL " " FORCE)
# Doesn't work on GitHub
#if(UNIX AND NOT APPLE)
#    set(OPTION_USE_PANGO ON CACHE BOOL " " FORCE)
#endif()
FetchCOntent_Declare(fltk
  GIT_REPOSITORY https://github.com/fltk/fltk
  # v1.4.0, doesn't have a release tag yet
  GIT_TAG ed5ee81ccd042f08373da1f49486075ec0e40480)
FetchContent_MakeAvailable(spdlog fltk)
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
add_executable(PsSp WIN32 MACOSX_BUNDLE
  src/main.cpp
  src/Windows/About.cpp
  src/Windows/Main.cpp
  src/Windows/Welcome.cpp
  src/Application/Application.cpp
)

if (APPLE)
  target_link_libraries(PsSp PRIVATE "-framework cocoa")
endif()

# System prevents warnings from fltk!
target_include_directories(PsSp SYSTEM PRIVATE ${fltk_BINARY_DIR}
  ${fltk_SOURCE_DIR} ${fltk_INCLUDE_DIRS})
target_link_libraries(PsSp PRIVATE
  fltk
  spdlog::spdlog_header_only)
# If (macOS or Linux) and Release build
# then strip the binary of debug symbols to make it smaller
# Windows doesn't need this
if (UNIX AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
  add_custom_command(TARGET PsSp
    COMMAND strip "$<TARGET_FILE:PsSp>"
    VERBATIM)
endif()
#===============================================================================
# macOS Bundle
#===============================================================================
set(MACOSX_BUNDLE_BUNDLE_NAME "PsSp")
set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
set(MACOSX_BUNDLE_COPYRIGHT
  "2023 Alexander R. Blanchette <arbCoding@gmail.com>")
#===============================================================================
# CPack Installation
#===============================================================================
# In the future I want to build a app-directory that this then installs in
# /Applications
if (APPLE)
  install(TARGETS PsSp RUNTIME DESTINATION .
  BUNDLE DESTINATION ${CMAKE_SOURCE_DIR} COMPONENT Program)
else()
  # Windows/Linux
  install(TARGETS PsSp COMPONENT Program)
  install(FILES ${PsSp_SOURCE_DIR}/LICENSE DESTINATION .
    COMPONENT license)
  set(CPACK_RESOURCE_FILE_LICENSE ${PsSp_SOURCE_DIR}/LICENSE)
endif()
# Group components
cpack_add_component(license DISPLAY_NAME "license"
  DESCRIPTON "PsSp license file.")
cpack_add_component(Program DISPLAY_NAME "PsSp"
  DESCRIPTION "Passive-source Seismic-processing")
set(CPACK_COMPONENT_license_HIDDEN TRUE)
set(CPACK_COMPONENT_license_REQUIRED TRUE)
set(CPACK_COMPONENT_Program_REQUIRED TRUE)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "PsSp")
set(CPACK_PACKAGE_VENDOR "Alexander R. Blanchette <arbCoding@gmail.com>")
# Allow separating components instead of monolithic
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
# Meta-data
set(CPACK_PACKAGE_NAME "PsSp")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDER "https://arbCoding.github.io/PsSp/")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://arbCoding.github.io/PsSp/")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "Passive-source Seismic Processing")
# Filename/installation directory
set(CPACK_PACKAGE_FILE_NAME "PsSp")
# Archive filename
set(CPACK_ARCHIVE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-\
${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
# Make checksum
set(CPACK_PACKAGE_CHECKSUM SHA512)
# Only specified components
set(CPACK_COMPONENTS_ALL license Program)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
# Windows Specific NSIS installer
set(CPACK_NSIS_HELP_LINK "https://arbCoding.github.io/PsSp/")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
# Debian Specific
# Installs into /usr/bin
set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
set(CPACK_DEB_COMPONENT_INSTALL OFF)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Alexander R. Blanchette \
<arbCoding@gmail.com>")
# RedHat Specific
# Installs into /usr/bin
set(CPACK_RPM_COMPONENT_INSTALL OFF)
# Don't strip binaries
set(CPACK_RPM_SPEC_MORE_DEFINE "%define __spec_install_post /bin/true")
include(CPack)
