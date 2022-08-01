#version 450
#extension GL_GOOGLE_include_directive : enable
#include "assets/shaders/core.glsl"


layout (location = 0) out vec4 outColor;

layout(location = 0) in vec2 inUV;

layout (set = 0, binding = 0) uniform sampler2D image;
layout(set = 1, binding = 0) uniform GlobalUbo {
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

void main() {
    vec2 uv = vec2(inUV.x, 1.0 - inUV.y);

    vec3 color = texture(image, uv).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    if(ACES == 1) {
        color = ACESFilm(color);
    }
    outColor = vec4(color, 1.0);

}