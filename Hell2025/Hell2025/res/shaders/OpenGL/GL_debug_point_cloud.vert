#version 460 core

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_normal;
layout(location = 2) in vec4 in_directLighting;

uniform int u_viewportIndex;
uniform mat4 u_projectionView;
out vec4 v_normal;
out vec4 v_directLighting;

void main() {
    gl_Position = u_projectionView * vec4(in_position.xyz, 1.0);
    v_normal = in_normal;
    v_directLighting = in_directLighting;
}