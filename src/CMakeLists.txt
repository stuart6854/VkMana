add_library(VkMana
        VkMana/VkMana.cpp
        VkMana/Ext/RenderDoc.cpp
)

target_include_directories(VkMana PRIVATE . PUBLIC ../include)

target_link_libraries(VkMana PUBLIC vulkan vma-hpp)

target_compile_features(VkMana PRIVATE cxx_std_17)