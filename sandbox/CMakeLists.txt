cmake_minimum_required(VERSION 3.6)
project (engine LANGUAGES CXX)
set(CMAKE_CX_C)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Make variables referring to all the sources and test files.
file(GLOB_RECURSE SOURCES "*.cpp")

# find_package(MyLib REQUIRED)

# The main for testing
set(entry_point "main.cpp")
add_executable(main ${entry_point})
target_compile_features(main PRIVATE cxx_std_17)
# include_directories(${CMAKE_SOURCE_DIR}/engine) # include files from ProjectB
# target_include_directories(main PUBLIC "${CMAKE_SOURCE_DIR}/*h")
target_link_libraries( main engine)
install(TARGETS main RUNTIME DESTINATION bin)