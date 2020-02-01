cmake_minimum_required(VERSION 2.6)

#Project configuration
project(gpu_project )
set( EXECUTABLE_OUTPUT_PATH ${gpu_project_SOURCE_DIR}/bin )



#Include Eigen
find_package( Eigen3 REQUIRED )
include_directories(${EIGEN3_INCLUDE_DIR})

#Executable configuration
file( GLOB_RECURSE sources src/*)

include_directories( include)
add_executable( exe ${sources} )