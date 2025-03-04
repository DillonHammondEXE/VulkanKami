#version 450

layout(location = 0) in vec3 fragColor;
layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push { // Ideally we would want to push projection,view, and model matrices but for now we can only do 2 4x4
    mat4 transform; // projection * view * model
    mat4 normalMatrix;
} push;

void main() {
    outColor = vec4(fragColor, 1.0);
}