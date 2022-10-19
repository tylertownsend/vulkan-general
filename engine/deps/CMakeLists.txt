cmake_minimum_required(VERSION 3.6)
project(imgui)

include(GNUInstallDirs)

add_library(${PROJECT_NAME} STATIC)
file (GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp)
file (GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h)
target_sources(imgui
    PRIVATE
        ${SOURCES}
        ${HEADERS}
        )
target_include_directories(${PROJECT_NAME} PUBLIC HEADERS)