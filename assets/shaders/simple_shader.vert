#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uv_out;
layout(location = 2) out vec3 out_position;
layout(location = 3) out vec3 out_normal;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projectionMatrix;
  mat4 viewMatrix;
  vec4 cameraPos;
  PointLight pointLights[10];
  int numberPointLights;
} ubo;
layout(set = 0, binding = 1) uniform sampler2D albedo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
  vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
  mat4 projectionViewMatrix = ubo.projectionMatrix * ubo.viewMatrix;
  gl_Position = projectionViewMatrix * positionWorld;

  fragColor = color;
  uv_out = uv;
  out_position = positionWorld.xyz;
  out_normal = normal;
  //fragColor *= texture(albedo, uv);
  //fragColor = vec3(1.0);
}