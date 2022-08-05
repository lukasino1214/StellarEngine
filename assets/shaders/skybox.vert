#version 450
#extension GL_GOOGLE_include_directive : enable
#include "assets/shaders/core.glsl"

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 WorldPos;

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

layout(push_constant) uniform Push {
    mat4 model_matrix;
} push;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    WorldPos = position;
    vec4 pos = ubo.projectionMatrix * push.model_matrix * vec4(position, 1.0);
    gl_Position = pos.xyww;
}