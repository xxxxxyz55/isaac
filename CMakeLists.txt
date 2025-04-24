cmake_minimum_required(VERSION 3.2)

project(isaac)

add_compile_options(-std=c++11)
set(CMAKE_VERBOSE_MAKEFILE on) 

set (CMAKE_BUILD_RPATH ${PROJECT_SOURCE_DIR}/build)

option(YS_SUPPORT_SSL "ssl support" on)
option(YS_MAVX2 "avx2 support" on)

if(YS_SUPPORT_SSL)
add_definitions(-DYS_SUPPORT_SSL=1)
endif()

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
include(posix.cmake)
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
include(windows.cmake)
else()
    message("platform unkonw")
endif()


