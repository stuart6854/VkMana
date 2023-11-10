#version 460 core

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D uTexture;

void main()
{
    outFragColor = vec4(texture(uTexture, inTexCoord).rgb, 1.0);
}
