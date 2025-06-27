#version 450
#extension GL_ARB_shader_texture_lod : require

#ifndef ENABLE_BINDLESS
    #define ENABLE_BINDLESS 1
#endif

#if ENABLE_BINDLESS == 1
    #extension GL_ARB_bindless_texture : enable        
    readonly restrict layout(std430, binding = 0) buffer textureSamplersBuffer {
	    uvec2 textureSamplers[];
    };        
    in flat int TextureIndex;

#else
    layout (binding = 2) uniform sampler2D DecalTex0;
    layout (binding = 3) uniform sampler2D DecalTex1;
    layout (binding = 4) uniform sampler2D DecalTex2;
    layout (binding = 5) uniform sampler2D DecalTex3;
#endif

layout (location = 0) out vec4 DecalMaskOut;

layout (binding = 0) uniform sampler2D WorldPositionTexture;
layout (binding = 1) uniform sampler2D GBufferNormalTexture;

in flat int Type;
in mat4 InverseModelMatrix;

uniform vec3 _DecalForwardDirection;

float saturate(float value) {
	return clamp(value, 0.0, 1.0);
}

void main2() {
    DecalMaskOut = vec4(1,0,0,1);

}

void main() {
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

    // Prevents barcode effect
    if(abs(angle) < 0.125 && angleToFloor < 0.5) {
     // discard;
    }

    // Backstab test. If world normal is facing away from original bullet angle.
    if (angle < -0.5) {
        //discard;
        // you almost certainly dont need this
    }
        
    vec3 gbufferWorldPosition = texture(WorldPositionTexture, screenCoords).rgb;

    // Don't draw on ceiling
	if (gbufferWorldPosition.y > 2.39) {
	//	discard;
        // you almost certainly dont need this
    }

	vec4 objectPosition = InverseModelMatrix * vec4(gbufferWorldPosition, 1.0);
    vec3 stepVal = (vec3(0.5, 0.5, 0.5) - abs(objectPosition.xyz)) * 1000;
    stepVal.x = saturate(stepVal.x);
    stepVal.y = saturate(stepVal.y);
    stepVal.z = saturate(stepVal.z);
    float projClipFade = stepVal.x * stepVal.y * stepVal.z;
	// Add 0.5 to get texture coordinates.
	vec2 decalTexCoord = vec2(objectPosition.x, objectPosition.z) + 0.5;

    
    #if ENABLE_BINDLESS == 1
        vec4 textureData  = texture(sampler2D(textureSamplers[TextureIndex]), decalTexCoord);
    #else
        vec4 textureData = vec4(0);
        if (Type == 0) {     
            textureData = texture(DecalTex0, decalTexCoord); 
        }
        if (Type == 1) {     
            textureData = texture(DecalTex1, decalTexCoord); 
        }
        if (Type == 2) {     
            textureData = texture(DecalTex2, decalTexCoord); 
        }
        if (Type == 3) {     
            textureData = texture(DecalTex3, decalTexCoord); 
        }
    #endif





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

    if (mask.a * 2 * projClipFade < 0.1) {
      //  discard;
    }

    vec3 _TintColor = vec3(0.32, 0, 0);
    float colorMask = (mask.a * 5) * res.a;
    float alphaMask = (mask.a  * 20) * res.a;
    alphaMask = clamp(alphaMask, 0, 1);
    colorMask = clamp(colorMask , 0, 1);
    colorMask = mask.a * 0.5;
    res.a = mask.a;
    decalTexCoord = clamp(decalTexCoord, 0, 1);
    res.rgb = mix(_TintColor.rgb, _TintColor.rgb * 0.2, mask.z * colorMask * 0.75);
    float magic = 0.67;




    vec3 decalColor = textureData.rgb;         
    float decalAlpha = textureData.a;

    decalColor = vec3(decalAlpha);
    DecalMaskOut.rgb  = vec3(decalColor);
}
