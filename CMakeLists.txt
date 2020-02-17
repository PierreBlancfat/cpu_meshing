cmake_minimum_required(VERSION 2.8.11)

#Project configuration
project(gpu_project)

add_compile_options("-fopenmp")

set( EXECUTABLE_OUTPUT_PATH ${gpu_project_SOURCE_DIR}/bin )

# Eigen
find_package( Eigen3 REQUIRED )
include_directories(${EIGEN3_INCLUDE_DIR})

# SFML
link_directories("./sfml/lib/")
include_directories("SFML-master/include")

include_directories(utils PUBLIC Common)

# OPENMP
OPTION (USE_OpenMP "Use OpenMP" ON)
IF(USE_OpenMP)
  FIND_PACKAGE(OpenMP)
  IF(OPENMP_FOUND)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  ENDIF()
ENDIF()

#OpenCL

# # Recherche le package OpenCL
 find_package(OpenCL REQUIRED)
 if(NOT OPENCL_HAS_CXX)
   message(WARNING "C++ bindings not found.  Using Kronos supplied reference OpenCL v1.2 C++ bindings.")
   list(APPEND OpenCL_INCLUDE_DIRS Common)
 endif()


# # Ajoute les dépendences sur la bibliothèque OPENCL
 set(LIBS ${LIBS} ${OpenCL_LIBRARY})
 include_directories(${OpenCL_INCLUDE_DIR})
 include_directories(SYSTEM ${OPENCL_INCLUDE_DIRS})
 add_definitions(${OPENCL_CXX_DEFINITIONS})


include_directories(include)
add_subdirectory(src)