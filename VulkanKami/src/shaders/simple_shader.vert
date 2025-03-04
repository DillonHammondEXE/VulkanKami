#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push { // Ideally we would want to push projection,view, and model matrices but for now we can only do 2 4x4
    mat4 transform; // projection * view * model
    mat4 normalMatrix;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0)); // always ensure input vectors for lighting calculations are normalized
const float AMBIENT = 0.02;

void main() {
    gl_Position = push.transform * vec4(position, 1.0); // 1.0 is the homogeneous coordinate

    // Need to convert the normals into world space since the light direction is in world space, this is done by * with modelMatrix
    // Converted to mat3 since we do not need the translation data in mat4 since normals represent directions and not positions and are not affected
    // by translations
    // temporary: this is only correct in certain situations!
    // vec3 normalWorldSpace = normalize(mat3(push.modelMatrix) * normal);

    // calculating the inverse in a shader can be expensive and should be avoided
    // mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
    // vec3 normalWorldSpace = normalize(normalMatrix * normal);

    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

    float lightIntensity = AMBIENT + max(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0); // Normals facing away are negative so we set them to 0

    fragColor = lightIntensity * color;
}