#version 460 core

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D uPositionTexture;
layout(set = 0, binding = 1) uniform sampler2D uNormalTexture;
layout(set = 0, binding = 2) uniform sampler2D uAlbedoTexture;

void main()
{
    // Get G-Buffer values
    vec3 fragPos = texture(uPositionTexture, inTexCoord).rgb;
    vec3 normal = texture(uNormalTexture, inTexCoord).rgb;
    vec4 albedo = texture(uAlbedoTexture, inTexCoord);

    outFragColor = vec4(fragPos, 1.0);
}
