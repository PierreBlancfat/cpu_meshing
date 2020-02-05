cmake_minimum_required(VERSION 2.6)

#Project configuration
project(gpu_project )
set( EXECUTABLE_OUTPUT_PATH ${gpu_project_SOURCE_DIR}/bin )

#Include Eigen
find_package( Eigen3 REQUIRED )
include_directories(${EIGEN3_INCLUDE_DIR})

#include SFML

link_directories("./sfml/lib")
include_directories("./SFML/include")
find_package(SFML 2.5.1 COMPONENTS system window graphics network audio)

#Executable configuration
file( GLOB_RECURSE sources src/*)

include_directories(include)
add_subdirectory(src)
