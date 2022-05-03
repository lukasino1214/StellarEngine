#version 450

const vec2 OFFSETS[6] = vec2[](
vec2(-1.0, -1.0),
vec2(-1.0, 1.0),
vec2(1.0, -1.0),
vec2(1.0, -1.0),
vec2(-1.0, 1.0),
vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

struct PointLight {
    vec4 position;
    vec3 color;
    float intensity;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 cameraPos;
    PointLight pointLights[10];
    int numLights;
} ubo;

const float LIGHT_RADIUS = 0.75;

layout(push_constant) uniform Push {
    vec4 position;
    vec4 color;
} push;

void main() {
    fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = {ubo.viewMatrix[0][0], ubo.viewMatrix[1][0], ubo.viewMatrix[2][0]};
    vec3 cameraUpWorld = {ubo.viewMatrix[0][1], ubo.viewMatrix[1][1], ubo.viewMatrix[2][1]};

    vec3 positionWorld = push.position.xyz
    + LIGHT_RADIUS * fragOffset.x * cameraRightWorld
    + LIGHT_RADIUS * fragOffset.y * cameraUpWorld;

    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * vec4(positionWorld, 1.0);
}