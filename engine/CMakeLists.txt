cmake_minimum_required(VERSION 3.6)
project (engine LANGUAGES CXX)
set(CMAKE_CX_C)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# https://stackoverflow.com/questions/17511496/how-to-create-a-shared-library-with-cmake
# https://stackoverflow.com/questions/34438357/cmake-one-build-directory-for-multiple-projects
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

include(GNUInstallDirs)

# Compile all sources into a library.
add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE src
)

set(public_headers
    "include/main.h"
)
set_target_properties(${PROJECT_NAME} PROPERTIES 
    POSITION_INDEPENDENT_CODE TRUE)

if (UNIX)
  # note that it is not CMAKE_INSTALL_PREFIX we are checking here
  if(DEFINED CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      message(
          STATUS
          "CMAKE_INSTALL_PREFIX is not set\n"
          "Default value: ${CMAKE_INSTALL_PREFIX}\n"
          "Will set it to ${CMAKE_SOURCE_DIR}/install"
      )
      set(CMAKE_INSTALL_PREFIX
          "${CMAKE_SOURCE_DIR}/install"
          CACHE PATH "Where the library will be installed to" FORCE
      )
  else()
      message(
          STATUS
          "CMAKE_INSTALL_PREFIX was already set\n"
          "Current value: ${CMAKE_INSTALL_PREFIX}"
      )
  endif()
endif (UNIX)


set(PROJECT_SHARE EngineLib)
set(PROJECT_CONFIG "${PROJECT_SHARE}Config")

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_CONFIG}
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(EXPORT ${PROJECT_CONFIG} DESTINATION share/${PROJECT_SHARE}/cmake)

export(TARGETS ${PROJECT_NAME} FILE "${PROJECT_CONFIG}.cmake")
