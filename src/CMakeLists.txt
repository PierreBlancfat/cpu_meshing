
project (gpu_project)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/meshing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hull.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/triangulation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/performance.cpp

)
add_library(${PROJECT_NAME} ${SOURCES})

add_executable(triangulation triangulation.cpp)
target_link_libraries(triangulation  gpu_project sfml-graphics sfml-window sfml-system)
target_link_libraries(triangulation PUBLIC ${OpenCL_LIBRARY})

add_executable(performance performance.cpp)
target_link_libraries(performance ${OpenCL_LIBRARY}  gpu_project sfml-graphics sfml-window sfml-system)


# add_custom_command(TARGET ${EXEC} POST_BUILD
#                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                        ${CMAKE_CURRENT_SOURCE_DIR}/vadd.cl
#                        $<TARGET_FILE_DIR:${EXEC}>
#                    )
# add_executable(deviceinfo deviceinfo.cpp)
# target_link_libraries(deviceinfo ${OpenCL_LIBRARY}  gpu_project sfml-graphics sfml-window sfml-system)
