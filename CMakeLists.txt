# CMakeLists.txt
# FunnelVision Project
# Masado Ishii

# To build project: From ./build/, invoke `cmake ..'

# Credits for CMake usage:
# - Hank Childs .... Building and linking a single source file against VTK libs.
# - Derek Molloy ... Specifying a multi-directory project. derekmolloy.ie/hello-world-introductions-to-cmake/


cmake_minimum_required(VERSION 2.8.12.1)

# Name of the project.
PROJECT(funnelvision)

# Include VTK.
#SET(VTK_DIR /path/to/VTK6.0.0) #Use this to set manually if without find_package().
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# Project headers.
include_directories(include)

# Source files - filenames matching the GLOB expression are assigned to SOURCES.
file(GLOB SOURCES "src/*.cxx")

# Add source files to the project.
add_executable(funnelvision ${SOURCES})

# Link VTK
if(VTK_LIBRARIES)
  target_link_libraries(funnelvision ${VTK_LIBRARIES})
else()
  target_link_libraries(funnelvision vtkHybrid)
endif()

