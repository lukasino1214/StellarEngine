#version 450
#extension GL_GOOGLE_include_directive : enable
#include "assets/shaders/core.glsl"

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 position;
layout(location = 2) in mat3 TBN;

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
layout(set = 0, binding = 1) uniform samplerCube irradianceMap;
layout(set = 0, binding = 2) uniform sampler2D brdfLUT;
layout(set = 0, binding = 3) uniform samplerCube prefilterMap;
layout(set = 0, binding = 4) uniform samplerCube samplerEnv;

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
//layout(set = 2, binding = 0) uniform sampler2D shadowMap;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

layout (location = 0) out vec4 outColor;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(normal_map, uv).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(position);
    vec3 Q2  = dFdy(position);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N   = normalize(TBN[2]);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main() {
    vec3 albedo = pow(texture(albedo_map, uv).rgb, vec3(2.2));
    vec3 emissive = pow(texture(emissive_map, uv).rgb, vec3(2.2));
    float metallic = texture(metallic_roughness_map, uv).r;
    float roughness = texture(metallic_roughness_map, uv).g;
    float ao = texture(occlusion_map, uv).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(ubo.cameraPos.xyz - position);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 4; ++i) {
        vec3 L = normalize(ubo.pointLights[i].position.xyz - position);
        vec3 H = normalize(V + L);
        float distance = length(ubo.pointLights[i].position.xyz - position);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = ubo.pointLights[i].color * attenuation * ubo.pointLights[i].intensity;

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 color = ambient + Lo;
    color += emissive;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, 1.0);
}