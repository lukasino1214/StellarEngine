#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 tangent;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 WorldPos;

layout(push_constant) uniform Push {
    mat4 mvp;
} push;

void main()
{
    WorldPos = position;
    gl_Position =  push.mvp * vec4(position, 1.0);
}