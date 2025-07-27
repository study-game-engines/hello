#version 460 core

layout (location = 0) out vec4 FragOut;

layout (binding = 0) uniform sampler3D u_lightVolume;
layout(r32ui, binding = 1) uniform uimage3D LightVolumeMaskImage;

flat in ivec3 VoxelCoord;
in vec3 WorldPos;

uniform float u_spacing;
uniform vec3 u_offset;
uniform int u_textureWidth;
uniform int u_textureHeight;
uniform int u_textureDepth;
uniform int u_worldSpaceWidth;
uniform int u_worldSpaceHeight;
uniform int u_worldSpaceDepth;
uniform int u_showMask;

void main() {
    discard;
    vec3 color = texelFetch(u_lightVolume, VoxelCoord, 0).rgb;

    vec3 lightVolumeWorldSize = vec3(u_worldSpaceWidth, u_worldSpaceHeight, u_worldSpaceDepth);
    vec3 uvw = (WorldPos - u_offset) / lightVolumeWorldSize;

    vec3 indirectLighting = texture(u_lightVolume, uvw).rgb;
           
    float mask = imageLoad(LightVolumeMaskImage, VoxelCoord).r;
    if (mask == 0) {
        discard;
    }

    FragOut.rgb = color;
	FragOut.a = 1.0;

    
   // FragOut.rgb = vec3(0, 1, 1);
}
