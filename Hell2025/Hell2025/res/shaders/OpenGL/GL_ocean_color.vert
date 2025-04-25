#version 450
#include "../common/types.glsl"

readonly restrict layout(std430, binding = 2) buffer viewportDataBuffer { ViewportData viewportData[]; };
readonly restrict layout(std430, binding = 6) buffer oceanPatchTransformsBuffer { mat4 oceanPatchTransforms[]; };

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;

out vec3 position;
out vec3 normal;

uniform float u_normalModifier;

void main () {

    int viewportIndex = 0;    
    mat4 projectionView = viewportData[viewportIndex].projectionView; 

    mat4 model = oceanPatchTransforms[gl_InstanceID];
    vec4 worldPos = model * vec4(vPosition, 1.0);

    gl_Position = projectionView * worldPos;
    position = worldPos.xyz;
    normal = vNormal * u_normalModifier;
}
