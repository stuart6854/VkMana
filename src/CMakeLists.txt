add_library(VkMana
)

target_include_directories(VkMana PRIVATE . PUBLIC ../include)

target_link_libraries(VkMana PUBLIC vulkan vma-hpp #[[shaderc]])

target_compile_features(VkMana PRIVATE cxx_std_17)
target_compile_definitions(VkMana PRIVATE #[[WIN32_LEAN_AND_MEAN NOMINMAX]])