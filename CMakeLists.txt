cmake_minimum_required(VERSION 2.8.11)

#Project configuration
project(gpu_project )
set( EXECUTABLE_OUTPUT_PATH ${gpu_project_SOURCE_DIR}/bin )

# Eigen
find_package( Eigen3 REQUIRED )
include_directories(${EIGEN3_INCLUDE_DIR})

# SFML
link_directories("./sfml/lib")



include_directories(include/)

add_subdirectory(src)



