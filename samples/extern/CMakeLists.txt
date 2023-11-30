# sdl2
option(SDL2_DISABLE_SDL2MAIN ON)
option(SDL2_DISABLE_INSTALL ON)
option(SDL2_DISABLE_UNINSTALL ON)
option(SDL_SHARED OFF)
option(SDL_TEST OFF)
option(SDL_TESTS OFF)
option(SDL_OPENGL OFF)
option(SDL_OPENGLES OFF)
option(SDL_DIRECTX OFF)
option(SDL_METAL OFF)
option(SDL_AUDIO OFF)
option(SDL_HAPTIC OFF)
option(SDL_Hidapi OFF)
option(SDL_Sensor OFF)
option(SDL_Locale OFF)
add_subdirectory(sdl2)
set_property(TARGET SDL2-static PROPERTY FOLDER "ThirdParty")

# GLM
if (NOT TARGET glm)
    add_subdirectory(glm)
endif ()

#stb
add_subdirectory(stb)

# tinygltf
add_subdirectory(tinygltf)

# tinyobjloader
add_subdirectory(tinyobjloader)