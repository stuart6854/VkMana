cmake_minimum_required(VERSION 3.10)

project(samples LANGUAGES C CXX)

# Dependencies
add_subdirectory(extern)

# Samples
add_subdirectory(hello_triangle)
add_subdirectory(model_loading)
add_subdirectory(sandbox)
