message(STATUS "Env VULKAN_SDK=$ENV{VULKAN_SDK}")
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined dxc GLOBAL)

if(NOT ${Vulkan_FOUND})
    message(STATUS "Vulkan SDK not found.")
endif()

# vma-hpp
option(VMA_HPP_INSTALL "Install vma-hpp" OFF)
add_subdirectory(vma-hpp)

CPMAddPackage(
    NAME fmt
    GIT_TAG 10.1.1
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL OFF"
)