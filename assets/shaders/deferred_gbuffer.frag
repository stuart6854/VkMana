#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

layout(set = 0, binding = 0) uniform sampler2D uGlobalTextures[];

layout(push_constant) uniform PushConstants
{
    mat4 modelMatrix;
    uint albedoIdx;
} uConsts;


void main()
{
    outPosition = vec4(inWorldPos, 1.0);
    outNormal = vec4(inNormal, 1.0);
    outAlbedo = texture(uGlobalTextures[uConsts.albedoIdx], inTexCoord);
}
