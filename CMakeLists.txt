cmake_minimum_required(VERSION 3.6)
project (triangle LANGUAGES CXX)
set(CMAKE_CX_C)

# engine code
add_subdirectory(engine)
add_subdirectory(sandbox)

find_package(Vulkan REQUIRED)

function(COPY_SOURCES SOURCE_PATH)
  set(SOURCE_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_PATH}/)
  file(COPY ${SOURCE_DIR_NAME} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/build/${SOURCE_PATH}/)
endfunction(COPY_SOURCES SOURCE_PATH)

if (NOT Vulkan_FOUND)
  message(FATAL_ERROR "Could not find Vulkan library!")
else ()
  message(STATUS "FOUND Vulkan library at ${Vulkan_LIBRARY}")
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(CompileShaders)

# find the glfw3 binaries
find_package(glfw3 3.3 REQUIRED)

if (NOT glfw3_FOUND)
  message(FATAL_ERROR "Could not find GLFW3 library!")
else ()
  message(STATUS "Found GLFW3 at ${glfw_LIBRARY}")
endif ()
# get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

# stb image dependency
include_directories(${CMAKE_SOURCE_DIR}/deps/stb)
link_directories(${CMAKE_SOURCE_DIR}/deps/stb) 

# tiny obj loader dependency
include_directories(${CMAKE_SOURCE_DIR}/deps/tinyobjloader)
link_directories(${CMAKE_SOURCE_DIR}/deps/tinyobjloader)

# Include sources to copy: textures, models
copy_sources(textures)
copy_sources(models)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# The main for testing
add_executable(demo_main "src/vulkan/main.cpp")
target_compile_features(demo_main PRIVATE cxx_std_17)
target_include_directories(demo_main PUBLIC "${CMAKE_SOURCE_DIR}/*h")
target_link_libraries( demo_main glfw)
target_link_libraries( demo_main ${Vulkan_LIBRARIES})

