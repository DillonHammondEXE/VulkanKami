#version 450

layout(location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo { // Set and Binding numbers must match what we set when setting up the descriptorSetLayout
    mat4 projection; // Could send projectView precomputed for a slight performance increase
    mat4 view;
    vec4 ambientLightColor; // W is intensity
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

void main() {
    float dis = sqrt(dot(fragOffset, fragOffset));
    if(dis >= 1.0)  {
        discard;
    }
    outColor = vec4(ubo.lightColor.xyz, 1.0);
}