#version 450
#extension GL_GOOGLE_include_directive : enable
#include "assets/shaders/core.glsl"

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in mat3 TBN;

layout (location = 0) out vec4 outAlbedo;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outMetallicRoughness;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 cameraPos;
    PointLight pointLights[10];
    DirectionalLight directionalLights[10];
    int numPointLights;
    int numDirectionalLights;
    float width;
    float height;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D albedo;
layout(set = 1, binding = 1) uniform sampler2D normal;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughness;

void main() {
    vec4 color = texture(albedo, fragUV);
    if(color.w < 0.0001) {
        discard;
    }

    vec3 N;

    if(texture(normal, fragUV).xyz == vec3(1.0)) {
        N = TBN[2];
    } else {
        N = TBN * normalize(texture(normal, fragUV).xyz * 2.0 - vec3(1.0));
    }

    outAlbedo = color;
    outPosition = vec4(fragPosWorld, 1.0);
    outNormal = vec4(N, 1.0);
    outMetallicRoughness = texture(metallicRoughness, fragUV);
}