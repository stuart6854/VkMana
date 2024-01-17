cmake_minimum_required(VERSION 3.26)

project(VkMana VERSION 0.1.0 LANGUAGES C CXX)

option(VKMANA_BUILD_SHARED "Build as Shared library (Untested - Use at your own peril!)" OFF)
option(VKMANA_BUILD_SAMPLES "Build the sample projects" ON)

include(cmake/CPM.cmake)

add_subdirectory(extern)

add_subdirectory(src)

if (VKMANA_BUILD_SAMPLES)
    add_subdirectory(samples_app)
endif ()