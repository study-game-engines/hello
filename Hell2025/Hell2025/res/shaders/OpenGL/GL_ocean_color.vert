#version 450
#include "../common/types.glsl"

readonly restrict layout(std430, binding = 2) buffer viewportDataBuffer { ViewportData viewportData[]; };

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;

uniform mat4 u_model;
out vec3 position;
out vec3 normal;

void main () {
    int viewportIndex = 0;    
    mat4 projectionView = viewportData[viewportIndex].projectionView; 

    vec4 worldPos = u_model * vec4(vPosition, 1.0);
    gl_Position = projectionView * worldPos;
    position = worldPos.xyz;
    normal = vNormal;
}
