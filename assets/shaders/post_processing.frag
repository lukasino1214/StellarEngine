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

const float gamma     = 2.2;
const float exposure  = 1.0;
const float pureWhite = 1.0;

void main() {
    vec2 uv = vec2(inUV.x, 1.0 - inUV.y);

    /*if(FXXA == 1) {
        vec2 v_rgbNW;
        vec2 v_rgbNE;
        vec2 v_rgbSW;
        vec2 v_rgbSE;
        vec2 v_rgbM;
        vec2 fragCoord = uv * vec2(ubo.width, ubo.height);
        texcoords(fragCoord, vec2(ubo.width, ubo.height), v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);
        vec3 color = fxaa(image, fragCoord, vec2(ubo.width, ubo.height), v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM).rgb;
        if(ACES == 1) {
            color = ACESFilm(color);
        }
        outColor = vec4(color, 1.0);
    } else {
        vec3 color = texture(image, uv).rgb;
        if(ACES == 1) {
            color = ACESFilm(color);
        }
        outColor = vec4(color, 1.0);
    }*/

    vec3 color = texture(image, uv).rgb;

    /*float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float mappedLuminance = (luminance * (1.0 + luminance/(pureWhite*pureWhite))) / (1.0 + luminance);

    // Scale color by ratio of average luminances.
    vec3 mappedColor = (mappedLuminance / luminance) * color;

    // Gamma correction.
    outColor = vec4(pow(mappedColor, vec3(1.0/gamma)), 1.0);*/
    outColor = vec4(color, 1.0);
}