#version 450
layout(location = 0) in vec3 inPosition;

uniform mat4 u_projectionView;

struct ScreenSpaceDecal {
    mat4 modelMatrix;
    mat4 inverseModelMatrix;
    int type;
    int textureIndex;
    int padding1;
    int padding2;
};

readonly restrict layout(std430, binding = 12) buffer ScreenSpaceDecalBuffer { ScreenSpaceDecal screenSpaceDecals[]; };

out vec3 FragPos;
out mat4 InverseModelMatrix;
out flat int Type;
out flat int TextureIndex;

void main() {
    mat4 modelMatrix = screenSpaceDecals[gl_InstanceID].modelMatrix;
    InverseModelMatrix = screenSpaceDecals[gl_InstanceID].inverseModelMatrix;
    Type = screenSpaceDecals[gl_InstanceID].type;
    TextureIndex = screenSpaceDecals[gl_InstanceID].textureIndex;

	vec4 worldPos = modelMatrix * vec4(inPosition, 1.0);
	FragPos = worldPos.xyz;
	gl_Position = u_projectionView * worldPos;
}


