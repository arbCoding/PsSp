#===============================================================================
# Basic Setup
#===============================================================================
cmake_minimum_required(VERSION 3.25)
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
#===============================================================================
# Dependencies
#===============================================================================
# No longer using FetchContent -> faster builds
# and can cache on github
# FLTK 1.4.0 commit hash ed5ee81ccd042f08373da1f49486075ec0e40480
# spdlog v1.12.0 commit hash ac55e60488032b9acde8940a5de099541c4515da
#-------------------------------------------------------------------------------
# Manual
#-------------------------------------------------------------------------------
set(FLTK_DIR $ENV{HOME}/other_git/fltk)
find_package(FLTK CONFIG REQUIRED)
set(spdlog_DIR $ENV{HOME}/other_git/spdlog)
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
set(Sources
  src/main.cpp
  src/Widgets/Datasheet.cpp
  src/Widgets/Status_Bar.cpp
  src/Windows/About.cpp
  src/Windows/Main.cpp
  src/Windows/Welcome.cpp
  src/Application/Application.cpp
)

add_executable(PsSp WIN32 MACOSX_BUNDLE ${Sources})

if (APPLE)
  target_link_libraries(PsSp PRIVATE "-framework cocoa")
endif()

# System prevents warnings from fltk!
target_include_directories(PsSp SYSTEM PRIVATE ${FLTK_INCLUDE_DIRS})
target_include_directories(PsSp SYSTEM PRIVATE ${spdlog_DIR}/include)

target_link_libraries(PsSp PRIVATE
  fltk)
# If (macOS or Linux) and Release build
# then strip the binary of debug symbols to make it smaller
# Windows doesn't need this
if (UNIX AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
  add_custom_command(TARGET PsSp
    COMMAND strip "$<TARGET_FILE:PsSp>"
    VERBATIM)
endif()
#===============================================================================
# CPack Installation
#===============================================================================
# In the future I want to build a app-directory that this then installs in
if (APPLE)
  install(TARGETS PsSp BUNDLE DESTINATION . COMPONENT program)
else()
  install(TARGETS PsSp RUNTIME DESTINATION bin COMPONENT program)
endif()
# /Applications
install(FILES ${PsSp_SOURCE_DIR}/LICENSE DESTINATION .
  COMPONENT license)
set(CPACK_RESOURCE_FILE_LICENSE ${PsSp_SOURCE_DIR}/LICENSE)
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
set(CPACK_COMPONENTS_ALL license program)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
# Windows Specific NSIS installer
set(CPACK_NSIS_HELP_LINK "https://arbCoding.github.io/PsSp/")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
# Debian Specific
# Installs into /usr/bin
set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Alexander R. Blanchette \
<arbCoding@gmail.com>")
# RedHat Specific
# Installs into /usr/bin
set(CPACK_RPM_COMPONENT_INSTALL ON)
# Don't strip binaries
set(CPACK_RPM_SPEC_MORE_DEFINE "%define __spec_install_post /bin/true")
include(CPack)
