cmake_minimum_required(VERSION 3.6)
project (triangle LANGUAGES CXX)
set(CMAKE_CX_C)

find_package(Vulkan REQUIRED)

if (NOT Vulkan_FOUND)
  message(FATAL_ERROR "Could not find Vulkan library!")
else ()
  message(STATUS ${Vulkan_LIBRARY})
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(CompileShaders)

# find the glfw3 binaries
find_package(glfw3 3.3 REQUIRED)

if (NOT glfw3_FOUND)
  message(FATAL_ERROR "Could not find Vulkan library!")
else ()
  message(STATUS "${glfw3_LIBRARY} hello")
endif ()
get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

file( GLOB  cpp  *.cpp )

set(entry_point main.cpp)

add_executable(main ${entry_point} ${cpp})
target_compile_features(main PRIVATE cxx_std_17)
target_include_directories(main PUBLIC "${CMAKE_SOURCE_DIR}/*h")
target_link_libraries( main glfw)
target_link_libraries( main ${Vulkan_LIBRARIES})