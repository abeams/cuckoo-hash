cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(CuckooHash VERSION 2.2 LANGUAGES CXX)

if (NOT DEFINED _DEBUG_LEVEL)
    set(_DEBUG_LEVEL 0)
endif()
add_definitions(-D_DEBUG=${_DEBUG_LEVEL})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
add_subdirectory(src)

