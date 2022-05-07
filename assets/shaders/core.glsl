struct PointLight {
    vec4 position;
    vec3 color;
    float intensity;
};

struct DirectionalLight {
    vec4 mvp;
    vec4 position;
};

struct GlobalUbo {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec4 cameraPos;
    PointLight pointLights[10];
    // directionalLights;
    int numPointLights;
    //int numDirectionalLights;
    mat4 a;
    vec4 b;
};