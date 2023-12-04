find_package(Vulkan REQUIRED COMPONENTS shaderc_combined dxc GLOBAL)
if (NOT ${Vulkan_FOUND})
    message(STATUS "Vulkan SDK not found.")
endif ()

# vma-hpp
add_subdirectory(vma-hpp)

# FmtLib
if (NOT TARGET fmt)
    option(FMT_INSTALL "Generate the install target." OFF)
    add_subdirectory(fmt)
endif ()