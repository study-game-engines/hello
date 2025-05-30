#version 450
#extension GL_ARB_shader_texture_lod : require

layout (location = 0) out vec4 BaseColorOut;
layout (location = 1) out vec4 RMAOut;

layout (binding = 0) uniform sampler2D WorldPositionTexture;
layout (binding = 1) uniform sampler2D GBufferNormalTexture;
layout (binding = 2) uniform sampler2D DecalTex;

 uniform mat4 u_inverseModel;

//in vec3 Normal;
in mat3 TBN;
in mat4 inverseModelMatrix;

uniform vec3 _DecalForwardDirection;

float saturate(float value) {
	return clamp(value, 0.0, 1.0);
}

void main() {
    BaseColorOut = vec4(1,0,0, 1);
    RMAOut = vec4(0.5,0.5,1, 1);
}

void main2() {
    vec2 gbufferDimensions = textureSize(GBufferNormalTexture, 0);
    vec2 screenCoords = gl_FragCoord.xy / gbufferDimensions;

    // Discard if the pixel is in the blockout map 
    //float blockout = texture(GBufferNormalTexture, screenCoords).a;
    //if (blockout > 0.5)
    //    discard;

    // world normal
    vec3 worldNormal = texture(GBufferNormalTexture, screenCoords).rgb;
    float angleToFloor = abs(dot(worldNormal, vec3(0, 1, 0)));
    float angle = dot(worldNormal, _DecalForwardDirection);  
    if(abs(angle) < 0.125 && angleToFloor < 0.5) 
        discard;

    // Backstab test. If world normal is facing away from original bullet angle.
    if (angle < -0.5)
        discard;

        
    vec3 gbufferWorldPosition = texture(WorldPositionTexture, screenCoords).rgb;

    // Don't draw on ceiling
	if (gbufferWorldPosition.y > 2.39)
		discard;

	vec4 objectPosition = u_inverseModel * vec4(gbufferWorldPosition, 1.0);
    vec3 stepVal = (vec3(0.5, 0.5, 0.5) - abs(objectPosition.xyz)) * 1000;
    stepVal.x = saturate(stepVal.x);
    stepVal.y = saturate(stepVal.y);
    stepVal.z = saturate(stepVal.z);
    float projClipFade = stepVal.x * stepVal.y * stepVal.z;
	// Add 0.5 to get texture coordinates.
	vec2 decalTexCoord = vec2(objectPosition.x, objectPosition.z) + 0.5;

     vec4 mask = vec4(0.0);
   //if (u_Type == 4)
   //    mask = texture(DecalTex4, decalTexCoord);
   //if (u_Type == 6)
   //    mask = texture(DecalTex6, decalTexCoord);
   //if (u_Type == 7)
   //    mask = texture(DecalTex7, decalTexCoord);
   //if (u_Type == 9)
   //    mask = texture(DecalTex9, decalTexCoord);   
   //    
    vec4 res = vec4(0); 
    res.a = saturate(mask.a * 2);
    res.a *= projClipFade;

    if (mask.a * 2 * projClipFade < 0.1) 
       discard;

   vec3 _TintColor = vec3(0.32, 0, 0);
   float colorMask = (mask.a * 5) * res.a;
   float alphaMask = (mask.a  * 20) * res.a;
   alphaMask = clamp(alphaMask, 0, 1);
   colorMask = clamp(colorMask , 0, 1);
   colorMask = mask.a * 0.5;
   res.a = mask.a;


    res.rgb = mix(_TintColor.rgb, _TintColor.rgb * 0.2, mask.z * colorMask * 0.75);
    float magic = 0.67;

    BaseColorOut.rgb = mix(vec3(magic), res.rgb * 1.45, res.a * projClipFade);
    BaseColorOut.rgb *= vec3(0.725);
    BaseColorOut.a = 1.0;
   
   // Roughness / metallic / ambient
   // gAlbedo = vec4(0.125 , 0.25, 1, 0);
    RMAOut = vec4(0.125, 0.25, 1.0, 1.0);
}
