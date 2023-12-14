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
# This uses fltk 1.4.0 (download from github and compile)
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
#===============================================================================
# CPack Installation
#===============================================================================
install(TARGETS PsSp)
# In the future I want to build a app-directory that this then installs in
# /Applications
if(CPACK_GENERATOR MATCHES "productbuild")
  # macOS
  set(CPACK_RESOURCE_FILE_README
    ${PsSp_SOURCE_DIR}/installers/macOS/Readme.txt)
  set(CPACK_RESOURCE_FILE_WELCOME
    ${PsSp_SOURCE_DIR}/installers/macOS/Welcome.txt)
  install(FILES ${PsSp_SOURCE_DIR}/License.txt DESTINATION .
    COMPONENT license)
  set(CPACK_RESOURCE_FILE_LICENSE ${PsSp_SOURCE_DIR}/License2.txt)
  set(CPACK_SET_DESTDIR "ON")
else()
  # Windows/Linux
  install(FILES ${PsSp_SOURCE_DIR}/LICENSE DESTINATION .
    COMPONENT license)
  set(CPACK_RESOURCE_FILE_LICENSE ${PsSp_SOURCE_DIR}/LICENSE)
endif()
set(CPACK_COMPONENT_license_HIDDEN TRUE)
set(CPACK_COMPONENT_license_REQUIRED TRUE)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "PsSp")
set(CPACK_PACKAGE_VENDOR "Alexander R. Blanchette <arbCoding@gmail.com>")
# Allow separating components instead of monolithic
set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)
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
