#version 450
#extension GL_GOOGLE_include_directive : enable
#include "assets/shaders/core.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uv_out;
layout(location = 2) out vec3 out_position;
layout(location = 3) out vec4 outShadowCoord;
layout(location = 4) out mat3 TBN;

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

layout(set = 0, binding = 1) uniform sampler2D albedo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

const mat4 biasMat = mat4(
0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 1.0, 0.0,
0.5, 0.5, 0.0, 1.0 );

void main() {
    vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
    mat4 projectionViewMatrix = ubo.projectionMatrix * ubo.viewMatrix;
    gl_Position = projectionViewMatrix * positionWorld;

    fragColor = color;
    uv_out = uv;
    out_position = positionWorld.xyz;
    vec4 tangents = normalize(push.modelMatrix * tangent.xyzw);
    vec3 N = normalize(mat3(push.modelMatrix) * normal);
    vec3 T = normalize(tangents.xyz);
    vec3 B = cross(N, tangents.xyz) * tangents.w;
    TBN = mat3(T, B, N);
    outShadowCoord = (biasMat * ubo.directionalLights[0].mvp * push.modelMatrix ) * vec4(position, 1.0);
}