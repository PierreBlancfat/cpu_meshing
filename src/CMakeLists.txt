
project (gpu_project)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/meshing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hull.cpp
)

add_library(${PROJECT_NAME} ${SOURCES})


add_executable(meshing meshing.cpp)
target_link_libraries(meshing sfml-graphics sfml-window sfml-system)


# add_executable(DeviceInfo deviceinfo.cpp)
# target_link_libraries(DeviceInfo PUBLIC ${OpenCL_LIBRARY})