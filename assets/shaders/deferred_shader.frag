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
layout (location = 4) out vec4 outEmissive;

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

layout(set = 1, binding = 0) uniform sampler2D albedo_map;
layout(set = 1, binding = 1) uniform sampler2D metallic_roughness_map;
layout(set = 1, binding = 2) uniform sampler2D normal_map;
layout(set = 1, binding = 3) uniform sampler2D occlusion_map;
layout(set = 1, binding = 4) uniform sampler2D emissive_map;
layout(set = 1, binding = 5) uniform PBRParameters {
    vec4 base_color_factor;
    vec3 emissive_factor;
    float metallic_factor;
    float roughness_factor;
    float scale;
    float strength;
    float alpha_cut_off;
    float alpha_mode;

    int has_base_color_texture;
    int has_metallic_roughness_texture;
    int has_normal_texture;
    int has_occlusion_texture;
    int has_emissive_texture;
} pbr_parameters;

void main() {
    vec4 color = texture(albedo_map, fragUV);
    if(color.w < 0.0001) {
        discard;
    }
    color = vec4(pow(color.rgb, vec3(2.2)), 1.0);

    vec3 N;

    if(texture(normal_map, fragUV).xyz == vec3(1.0)) {
        N = TBN[2];
    } else {
        N = TBN * normalize(texture(normal_map, fragUV).xyz * 2.0 - vec3(1.0));
    }

    outAlbedo = color;
    outPosition = vec4(fragPosWorld, 1.0);
    outNormal = vec4(N, 1.0);
    outMetallicRoughness = texture(metallic_roughness_map, fragUV);
    outEmissive = vec4(0.0);
    if(pbr_parameters.has_emissive_texture == 1) {
        outEmissive = pow(texture(emissive_map, fragUV), vec4(2.2));
    }
}