#version 450
#extension GL_GOOGLE_include_directive : enable
#include "assets/shaders/core.glsl"

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outEmissive;

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
layout(set = 1, binding = 0) uniform samplerCube samplerEnv;

void main() {
    vec3 color = texture(samplerEnv, inUVW).rgb;
    outColor = vec4(color, 1.0);
    outEmissive = vec4(0.0);
}