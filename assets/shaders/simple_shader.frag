#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projectionMatrix;
  mat4 viewMatrix;
  vec4 ambientLightColor; // w is intensity
  vec3 lightPosition;
  vec4 lightColor;
  vec3 cameraPos;
} ubo;
layout(set = 0, binding = 1) uniform sampler2D albedo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

layout (location = 0) out vec4 outColor;

void main() {

    vec3 test = texture(albedo, uv).rgb;
  outColor = vec4(test, 1.0);
}