#version 450

layout (location = 0) out vec4 outColor;

layout(location = 0) in vec2 inUV;

layout (set = 0, binding = 0) uniform sampler2D image;
layout (set = 1, binding = 0) uniform sampler2D shdaow;

float vignatte(vec2 uv) {
    float dis = (1.0 - distance(vec2(0.5), uv));
    return pow(dis, 4.0);
}

float LinearizeDepth(float depth)
{
    float n = 1.00; // camera z near
    float f = 100.0; // camera z far
    float z = depth;
    return (2.0 * n) / (f + n - z * (f - n));
}


void main() {
    vec2 uv = vec2(inUV.x, 1.0 - inUV.y);

    /*if(uv.x > 0.5) {
        vec3 color = texture(image, uv).rgb;
        outColor = vec4(color, 1.0);
    } else {
        float depthValue = texture(shdaow, uv).r;
        outColor = vec4(vec3(1.0-LinearizeDepth(depthValue)), 1.0);
    }*/

    vec3 color = texture(image, uv).rgb;
    outColor = vec4(color, 1.0);
}