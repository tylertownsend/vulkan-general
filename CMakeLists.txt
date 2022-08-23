cmake_minimum_required(VERSION 3.6)
project (triangle LANGUAGES CXX)
set(CMAKE_CX_C)

find_package(Vulkan REQUIRED)

function(COPY_SOURCES SOURCE_PATH)
  set(SOURCE_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_PATH}/)
  file(COPY ${SOURCE_DIR_NAME} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/build/${SOURCE_PATH}/)
endfunction(COPY_SOURCES SOURCE_PATH)

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
  message(STATUS "${glfw}")
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
set(entry_point "src/vulkan/main.cpp")
add_executable(main ${entry_point})
target_compile_features(main PRIVATE cxx_std_17)
target_include_directories(main PUBLIC "${CMAKE_SOURCE_DIR}/*h")
target_link_libraries( main glfw)
target_link_libraries( main ${Vulkan_LIBRARIES})