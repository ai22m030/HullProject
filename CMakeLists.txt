cmake_minimum_required(VERSION 3.23)
project(HullProject)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
include_directories(src)

add_executable(HullProject main.cpp src/Hull.cpp include/Hull.h)

INCLUDE(FindPkgConfig)

set(SDL2_LIB "/Library/Frameworks/SDL2.framework/SDL2")
target_include_directories(HullProject PRIVATE "/Library/Frameworks/SDL2.framework/Headers")

target_link_libraries(HullProject ${SDL2_LIB})