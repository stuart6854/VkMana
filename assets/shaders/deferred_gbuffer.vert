#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoord;

layout(set = 1, binding = 0) uniform Camera
{
    mat4 projMatrix;
    mat4 viewMatrix;
} uCamera;

layout(push_constant) uniform PushConstants
{
    mat4 modelMatrix;
    uint albedoIdx;
} uConsts;

void main()
{
    outWorldPos = vec3(uConsts.modelMatrix * vec4(aPosition, 1.0));

    gl_Position = uCamera.projMatrix * uCamera.viewMatrix * vec4(outWorldPos, 1.0);

    mat3 normalMat = transpose(inverse(mat3(uConsts.modelMatrix)));
    outNormal = normalMat * normalize(aNormal);

    outTexCoord = aTexCoord;
}
