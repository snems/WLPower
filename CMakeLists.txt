cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

project(wlpower)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_BUILD_TYPE Release)

include_directories(.)
include_directories(/usr/include/hidapi/)

add_executable(wlpower main.cxx)

target_link_libraries(wlpower hidapi-libusb)


