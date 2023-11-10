#version 460 core

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D uPositionTexture;
layout(set = 0, binding = 1) uniform sampler2D uNormalTexture;
layout(set = 0, binding = 2) uniform sampler2D uAlbedoTexture;

struct Light
{
    vec4 Position;
    vec3 Color;
    float Radius;
};
const Light gLight = Light(vec4(-3, 3, 3, 0), vec3(1, 1, 1), 2.0f);
const float gAmbient = 0.1f;

void main()
{
    // Get G-Buffer values
    vec3 fragPos = texture(uPositionTexture, inTexCoord).rgb;
    vec3 normal = texture(uNormalTexture, inTexCoord).rgb;
    vec4 albedo = texture(uAlbedoTexture, inTexCoord);

    vec3 fragColor = albedo.rgb * gAmbient;

    // For each light
    {
        // Vector to light
        vec3 L = normalize(gLight.Position.xyz - fragPos);
        // Distance from light to fragment position
        float dist = length(L);

        // Attentuation
        float atten = gLight.Radius / (pow(dist, 2.0) + 1.0);

        // Diffuse part
        vec3 N = normalize(normal);
        float NdotL = max(0.0, dot(N, L));
        vec3 diff = gLight.Color * albedo.rgb * NdotL * atten;

        fragColor += diff;
    }

    outFragColor = vec4(fragColor, 1.0);
}
