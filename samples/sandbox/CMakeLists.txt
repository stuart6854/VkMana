add_executable(sample_sandbox main.cpp)

target_link_libraries(sample_sandbox PRIVATE VkMana #[[SDL2-static glm]])

target_compile_features(sample_sandbox PRIVATE cxx_std_20)