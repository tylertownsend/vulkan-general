cmake_minimum_required(VERSION 3.6)
project (engine LANGUAGES CXX)
set(CMAKE_CX_C)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Make variables referring to all the sources and test files.
file(GLOB_RECURSE SOURCES "*.cpp")

find_package(EngineLib REQUIRED)

# The main for testing
set(entry_point "main.cpp")
add_executable(sandbox ${entry_point})
target_compile_features(sandbox PRIVATE cxx_std_17)
# include_directories(${CMAKE_SOURCE_DIR}/engine) # include files from ProjectB
# target_include_directories(main PUBLIC "${CMAKE_SOURCE_DIR}/*h")
target_link_libraries(sandbox engine)