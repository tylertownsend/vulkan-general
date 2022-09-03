cmake_minimum_required(VERSION 3.6)
project (sandbox LANGUAGES CXX)
set(CMAKE_CX_C)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Make variables referring to all the sources and test files.
file(GLOB_RECURSE SOURCES "*.cpp")

# # Attempting to find the installed package
# find_package(EngineLib REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
find_package(EngineLib REQUIRED)

if (NOT EngineLib_FOUND)
  message(FATAL_ERROR "Could not find EngineLib library!")
else ()
  message (STATUS "EngineLib Found Lib!!")
endif ()

# get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

# The main for testing
add_executable(sandbox "main.cpp")
target_compile_features(sandbox PRIVATE cxx_std_17)
target_link_libraries(sandbox engine)