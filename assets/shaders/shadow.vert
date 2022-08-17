#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec2 uv;

struct PointLight {
    vec4 position;
    vec3 color;
    float intensity;
};

struct DirectionalLight {
    mat4 mvp;
    vec4 position;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 cameraPos;
    PointLight pointLights[10];
    DirectionalLight directionalLights[10];
    int numPointLights;
    int numDirectionalLights;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.directionalLights[0].mvp * push.modelMatrix * vec4(position, 1.0);
}