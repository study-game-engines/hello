#version 420 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_projectionView;
uniform float u_spacing;
uniform vec3 u_offset;
uniform int u_textureWidth;
uniform int u_textureHeight;
uniform int u_textureDepth;
uniform int u_worldSpaceWidth;
uniform int u_worldSpaceHeight;
uniform int u_worldSpaceDepth;

flat out ivec3 VoxelCoord;
out vec3 WorldPos;

void main() {
    int instanceID = gl_InstanceID;

    int z = instanceID % u_textureDepth;
    int y = (instanceID / u_textureDepth) % u_textureHeight;
    int x = (instanceID / (u_textureDepth * u_textureHeight)) % u_textureWidth;
    VoxelCoord = ivec3(x, y, z);

    vec3 pos = vec3(x, y, z) * u_spacing + u_offset;

    mat4 translation = mat4(1.0);
    translation[3] = vec4(pos, 1.0);

    mat4 scaleMat = mat4(1.0);
    scaleMat[0][0] = 0.0625;
    scaleMat[1][1] = 0.0625;
    scaleMat[2][2] = 0.0625;

    mat4 model = translation * scaleMat;

    WorldPos = (model * vec4(aPosition, 1.0)).xyz;

    gl_Position = u_projectionView * model * vec4(aPosition, 1.0);

}
