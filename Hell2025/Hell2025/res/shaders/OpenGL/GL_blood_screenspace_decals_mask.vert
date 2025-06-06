#version 450
layout(location = 0) in vec3 inPosition;

uniform mat4 u_projectionView;
uniform mat4 u_model;

out vec3 FragPos;
out mat4 inverseModelMatrix;

void main() {
	vec4 worldPos = u_model * vec4(inPosition, 1.0);
	FragPos = worldPos.xyz;
	gl_Position = u_projectionView * worldPos;
}


