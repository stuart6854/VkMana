#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;

layout(set = 0, binding = 0) uniform sampler2D uGlobalTextures[];

layout(push_constant) uniform PushConstants
{
    mat4 modelMatrix;
    uint albedoMapIdx;
    uint normalMapIdx;
} uConsts;

void main()
{
    outPosition = vec4(inWorldPos, 1.0);

    // Calculate normal in tangent space
    vec3 normal = normalize(inNormal);
    vec3 tangent = normalize(inTangent);
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    vec3 localNormal = normalize(TBN * (texture(uGlobalTextures[uConsts.normalMapIdx], inTexCoord).xyz * 2.0 - 1.0));
    outNormal = vec4(localNormal, 1.0);

    outAlbedo = texture(uGlobalTextures[uConsts.albedoMapIdx], inTexCoord);
}
