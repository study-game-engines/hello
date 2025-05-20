#version 460 core
#include "../common/util.glsl"
#include "../common/types.glsl"

layout (location = 0) in vec3 inPosition;

readonly restrict layout(std430, binding = 2) buffer viewportDataBuffer {
	ViewportData viewportData[];
};

out vec3 TexCoords;
out vec4 WorldPos;

uniform mat4 u_modelMatrix;

void main() {
    TexCoords = inPosition;
    int viewportIndex = gl_BaseInstance;
	mat4 projectionView = viewportData[viewportIndex].skyboxProjectionView;	
    vec3 viewPos = viewportData[viewportIndex].inverseView[3].xyz;    
    //gl_Position = projectionView * vec4((inPosition * 8) + viewPos, 1.0);

    WorldPos = u_modelMatrix * vec4(inPosition, 1.0);
    gl_Position = projectionView * WorldPos;
}