cmake_minimum_required(VERSION 3.23)
project(HullProject)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif()

set(CMAKE_CXX_STANDARD 20)

include_directories(include)
include_directories(src)

add_executable(HullProject main.cpp src/Hull.cpp include/Hull.h)

INCLUDE(FindPkgConfig)

if(MACOSX)
    set(SDL2_LIB "/Library/Frameworks/SDL2.framework/SDL2")
    target_include_directories(HullProject PRIVATE "/Library/Frameworks/SDL2.framework/Headers")

    target_link_libraries(HullProject ${SDL2_LIB})
endif()