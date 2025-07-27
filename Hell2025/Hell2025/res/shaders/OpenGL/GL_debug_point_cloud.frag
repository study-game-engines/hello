#version 460 core
layout (location = 0) out vec4 FragOut;

in vec4 v_normal;
in vec4 v_directLighting;

void main() {
    discard;
    if (v_directLighting.rgb == vec3(0,0,0)) {
       //discard;
       FragOut.rgb = vec3(1,0,0);;
    }    
    else {
        FragOut.rgb = v_directLighting.rgb;
        //FragOut.rgb = vec3(0,1,0);;
    }
    
	FragOut.a = 1.0;
}
