#version 460 core

layout (location = 0) out vec4 FragOut;

in vec4 v_normal;
in vec4 v_directLighting;

void main() {
    if (v_directLighting.rgb == vec3(0,0,0)) {
        //discard;
    }
    
    FragOut.rgb = v_directLighting.rgb;
    FragOut.rgb = v_directLighting.rgb * vec3(1, 0, 0);

    
    if (v_directLighting.rgb == vec3(0,0,0)) {    
        FragOut.rgb = vec3(1, 1, 0);
    }

    FragOut.rgb = v_normal.rgb;

	FragOut.a = 1.0;
}
