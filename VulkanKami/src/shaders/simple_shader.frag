#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo { // Set and Binding numbers must match what we set when setting up the descriptorSetLayout
    mat4 projectionViewMatrix;
    // vec3 directionToLight;
    vec4 ambientLightColor; // W is intensity
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

layout(push_constant) uniform Push { // Ideally we would want to push projection,view, and model matrices but for now we can only do 2 4x4
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
    vec3 directionToLight = ubo.lightPosition - fragPosWorld;
    // Always ensure attenuation is calculated BEFORE normazlizing the directionToLight vector
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // 1 divided by the direction length squared

    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation; // Scale intensities using W
    vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    // Note: even though fragNormalWorld is normalized in the vertex shader, the linear interpolation of two normalized vectors is not guranteed to be normal
    vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

    outColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
}