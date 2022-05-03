#version 450

layout (location = 0) out vec4 outColor;

layout(location = 0) in vec2 inUV;

layout (set = 0, binding = 0) uniform sampler2D image;

float vignatte(vec2 uv) {
    float dis = (1.0 - distance(vec2(0.5), uv));
    return pow(dis, 4.0);
}

void main() {
    vec2 uv = vec2(inUV.x, 1.0 - inUV.y);

    vec3 color = texture(image, uv).rgb;
    outColor = vec4(color, 1.0);
}