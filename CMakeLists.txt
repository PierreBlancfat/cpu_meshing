cmake_minimum_required(VERSION 2.6)

#Project configuration
project(gpu_project )
set( EXECUTABLE_OUTPUT_PATH ${gpu_project_SOURCE_DIR}/bin )



#Include Eigen
find_package( Eigen3 REQUIRED )
include_directories(${EIGEN3_INCLUDE_DIR})


set(SFML_INCLUDE_DIR "/home/polo/work/siam/GPU/gpu_project/sfml/sfmlbuild/include")
set(SFML_LIBRARY_DIR "/home/polo/work/siam/GPU/gpu_project/sfml/sfmlbuild/lib")
link_directories("/home/polo/work/siam/GPU/gpu_project/sfml/sfmlbuild/lib")
include_directories("/home/polo/work/siam/GPU/gpu_project/sfml/sfmlbuild/include")
set(SFML_DIR "/home/polo/work/siam/GPU/gpu_project/sfml/sfmlbuild/lib/cmake/SFML")
find_package(SFML 2.5.0 COMPONENTS system window graphics network audio)

if(SFML_FOUND)

    message(STATUS "SFML_INCLUDE_DIRS: ${SFML_INCLUDE_DIR}")
    message(STATUS "SFML_LIBRARIES: ${SFML_LIBRARIES}")
    message(STATUS "SFML_VERSION: ${SFML_VERSION}")

    include_directories(${SFML_INCLUDE_DIRS})

endif()

#Executable configuration
file( GLOB_RECURSE sources src/*)

include_directories(include)
add_executable( exe ${sources} )