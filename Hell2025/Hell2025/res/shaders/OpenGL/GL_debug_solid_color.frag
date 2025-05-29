#version 460 core

layout (location = 0) out vec4 FragOut;
uniform vec3 u_color;

in vec4 WorldPos;
in vec2 TexCoords;

void main() {
    if (WorldPos.y < 30) {
        discard;
    }
    
    FragOut.rgb = u_color * 0.5;
    FragOut.rgb = vec3(TexCoords, 0);
	FragOut.a = 1.0;
}
