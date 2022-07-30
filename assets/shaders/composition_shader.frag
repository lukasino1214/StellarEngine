#version 450
#extension GL_GOOGLE_include_directive : enable
#include "assets/shaders/core.glsl"

layout (location = 0) out vec4 outColor;

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

layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput samplerAlbedo;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput samplerposition;
layout (input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput samplerNormal;
layout (input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput samplerMetallicRoughness;

void main() {
    outColor = subpassLoad(samplerAlbedo);
}