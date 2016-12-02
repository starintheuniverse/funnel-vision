cmake_minimum_required(VERSION 2.8.12.1)

PROJECT(funnel)
SET(VTK_DIR /path/to/VTK6.0.0)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(funnel funnel)

if(VTK_LIBRARIES)
  target_link_libraries(funnel ${VTK_LIBRARIES})
else()
  target_link_libraries(funnel vtkHybrid)
endif()

