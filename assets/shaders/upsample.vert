#version 450

layout (location = 0) out vec2 outUV;

layout(push_constant) uniform Push {
    ivec2 srcResolution;
    int mip_level;
    float filter_radius;
} push;

void main()
{
    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}