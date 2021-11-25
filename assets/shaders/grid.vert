layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec4 ambientLightColor; // w is intensity
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

// Grid position are in xy clipped space
vec3 gridPlane[6] = vec3[](
vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);
// normal vertice projection
void main() {
    gl_Position = view.proj * view.view * vec4(gridPlane[gl_VertexIndex].xyz, 1.0);
}