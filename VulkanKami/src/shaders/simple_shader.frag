#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

struct PointLight {
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo { // Set and Binding numbers must match what we set when setting up the descriptorSetLayout
    mat4 projection; // Could send projectView precomputed for a slight performance increase
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor; // W is intensity
    PointLight pointLights[100]; // Can use Specialization Constants instead of this
    int numLights;
} ubo;

layout(push_constant) uniform Push { // Ideally we would want to push projection,view, and model matrices but for now we can only do 2 4x4
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraPosWorld = ubo.invView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld); // Vector pointing to the viewer

    for(int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 directionToLight = light.position.xyz - fragPosWorld;
    // Always ensure attenuation is calculated BEFORE normazlizing the directionToLight vector
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // 1 divided by the direction length squared
    directionToLight = normalize(directionToLight);

    float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
    vec3 intensity = light.color.xyz * light.color.w * attenuation;

    diffuseLight += intensity * cosAngIncidence;

    // specular lighting using Blinn-phong (Gaussian is more realistic*)
    vec3 halfAngle = normalize(directionToLight + viewDirection);
    float  blinnTerm = dot(surfaceNormal, halfAngle);
    blinnTerm = clamp(blinnTerm, 0, 1);
    blinnTerm = pow(blinnTerm, 512.0); // higher value means sharper highlight
    specularLight = intensity * blinnTerm;
    }
    outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}