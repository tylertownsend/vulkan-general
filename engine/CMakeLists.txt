cmake_minimum_required(VERSION 3.6)
project (engine LANGUAGES CXX)
set(CMAKE_CX_C)

# find the glfw3 binaries
find_package(glfw3 3.3 REQUIRED)

if (NOT glfw3_FOUND)
  message(FATAL_ERROR "Could not find Vulkan library!")
else ()
  message(STATUS "${glfw}")
endif ()
# get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

# # stb image dependency
# include_directories(${CMAKE_SOURCE_DIR}/deps/stb)
# link_directories(${CMAKE_SOURCE_DIR}/deps/stb) 

# # tiny obj loader dependency
# include_directories(${CMAKE_SOURCE_DIR}/deps/tinyobjloader)
# link_directories(${CMAKE_SOURCE_DIR}/deps/tinyobjloader)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Make variables referring to all the sources and test files.
file(GLOB_RECURSE SOURCES "*.cpp")

# Compile all sources into a library.
add_library(engine STATIC ${SOURCES} )
# add_dependencies(engine eigen)
# target_link_libraries(engine eigen pybind11::embed)
# https://stackoverflow.com/questions/34438357/cmake-one-build-directory-for-multiple-projects
set_target_properties(engine PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
install(TARGETS engine
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)