#version 460

layout (location = 0) out vec4 FinalLighting;
layout (location = 1) out vec4 Normal;
layout (binding = 0) uniform samplerCube cubeMap;

in vec3 TexCoords;

void main() {
    vec3 color = texture(cubeMap, TexCoords).rgb;
    FinalLighting = vec4(color, 1.0);
}
