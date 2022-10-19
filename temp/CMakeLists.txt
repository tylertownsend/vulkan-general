cmake_minimum_required(VERSION 3.13)
find_package(PkgConfig)
pkg_check_modules(GLEW glew)
find_package(GLEW REQUIRED)
