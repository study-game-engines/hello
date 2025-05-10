#include "../common/pbr_functions.glsl"

vec3 GetDirectLighting(vec3 lightPos, vec3 lightColor, float radius, float strength, vec3 Normal, vec3 WorldPos, vec3 baseColor, float roughness, float metallic, vec3 viewPos) {       
    float fresnelReflect = 1.0;
	vec3 viewDir = normalize(viewPos - WorldPos);
	float lightRadiance = strength;
	vec3 lightDir = normalize(lightPos - WorldPos);   
	float lightAttenuation = smoothstep(radius, 0, length(lightPos - WorldPos));
	float irradiance = max(dot(lightDir, Normal), 0.0) ;
	irradiance *= lightAttenuation * lightRadiance;
	vec3 brdf = microfacetBRDF(lightDir, viewDir, Normal, baseColor, metallic, fresnelReflect, roughness);
    return brdf * irradiance * clamp(lightColor, 0, 1);
}

vec3 GetSpotlightLighting2(vec3 lightPos, vec3 lightDir, vec3 lightColor, float radius, float strength, float innerAngle, float outerAngle, vec3 Normal, vec3 WorldPos, vec3 baseColor, float roughness, float metallic, vec3 viewPos) {       
    float fresnelReflect = 1.0;
	vec3 viewDir = normalize(viewPos - WorldPos);
	float lightRadiance = strength;
	vec3 toLight = normalize(lightPos - WorldPos);   
	float lightAttenuation = smoothstep(radius, 0, length(lightPos - WorldPos));

	// Spotlight effect
	float spotIntensity = dot(toLight, normalize(-lightDir));
	float spotFactor = smoothstep(outerAngle, innerAngle, spotIntensity);
	
	float irradiance = max(dot(toLight, Normal), 0.0);
	irradiance *= lightAttenuation * lightRadiance * spotFactor;
	
	vec3 brdf = microfacetBRDF(toLight, viewDir, Normal, baseColor, metallic, fresnelReflect, roughness);
    return brdf * irradiance * clamp(lightColor, 0, 1);
}

vec3 GetSpotlightLighting(
    vec3 lightPos, vec3 lightDir, vec3 lightColor, 
    float radius, float strength, 
    float innerAngle, float outerAngle, 
    vec3 Normal, vec3 WorldPos, 
    vec3 baseColor, float roughness, float metallic, 
    vec3 viewPos, mat4 LightViewProj
) {       
    float fresnelReflect = 1.0;
    vec3 viewDir = normalize(viewPos - WorldPos);
    float lightRadiance = strength;
    vec3 toLight = normalize(lightPos - WorldPos);
    
    // **Compute standard distance attenuation**
    float lightAttenuation = smoothstep(radius, 0, length(lightPos - WorldPos));

    // **Spotlight intensity based on angle**
    float spotIntensity = dot(toLight, normalize(-lightDir));
    float spotFactor = smoothstep(outerAngle, innerAngle, spotIntensity);

    // **Project world position into light space (same as cookie projection)**
    vec4 lightSpacePos = LightViewProj * vec4(WorldPos, 1.0);
    vec2 projectedUV = lightSpacePos.xy / lightSpacePos.w;
    projectedUV = projectedUV * 0.5 + 0.5;

    // **Depth factor to match cookie projection**
    float depthFactor = 1.0 / max(lightSpacePos.w, 0.001);
    projectedUV *= depthFactor; // Perspective warp

    // **Weaken light over distance (same as cookie)**
    float distanceFactor = clamp(1.0 - (length(lightPos - WorldPos) / radius), 0.0, 1.0);
    distanceFactor *= distanceFactor; // Quadratic falloff for smooth fade

    // **Blend spotFactor with perspective-based fade**
    spotFactor *= distanceFactor;

    // **Compute final light intensity**
    float irradiance = max(dot(toLight, Normal), 0.0);
    irradiance *= lightAttenuation * lightRadiance * spotFactor;

    vec3 brdf = microfacetBRDF(toLight, viewDir, Normal, baseColor, metallic, fresnelReflect, roughness);
    return brdf * irradiance * clamp(lightColor, 0, 1);
}

float SpotlightShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, vec3 fragWorldPos, vec3 lightPos, vec3 viewPos, sampler2DArray shadowMapArray, int layerIndex) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMapArray, vec3(projCoords.xy, layerIndex)).r; 
    float currentDepth = projCoords.z;    
    const float slopeFactor = 0.003;
    float slopeBias = slopeFactor * max(0.0, (1.0 - dot(normal, lightDir)));

    float dist = distance(fragWorldPos, lightPos);
    float scaleFactor = 0.028;
    float epsilon = 0.001;
    float baseBias = 0.0001;
    float bias = baseBias + (scaleFactor / (dist + epsilon));

    // PCF Filtering
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMapArray, 0).xy);
    const int kernelSize = 2;
    for (int x = -kernelSize; x <= kernelSize; ++x) {
        for (int y = -kernelSize; y <= kernelSize; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            float closestDepth = texture(shadowMapArray, vec3(projCoords.xy + offset, layerIndex)).r;
            shadow += (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= pow((2 * kernelSize + 1), 2);

    return shadow;
}

vec3 ApplyCookie(mat4 LightViewProj, vec3 worldPos, vec3 lightPos, vec3 lightColor, float maxDistance, sampler2D cookieTexture) {
    vec4 lightSpacePos = LightViewProj * vec4(worldPos, 1.0);
    vec2 cookieUV = lightSpacePos.xy / lightSpacePos.w;
    float depthFactor = 1.0 / max(lightSpacePos.w, 0.001);
    cookieUV *= depthFactor;
    cookieUV = cookieUV * 0.5 + 0.5;
    vec2 clampedUV = clamp(cookieUV, vec2(0.0), vec2(1.0));
    float fadeFactor = 1.0 - length(cookieUV - clampedUV) * 15.0;
    fadeFactor = clamp(fadeFactor, 0.0, 1.0);

    // Distance weakening
    float dist = length(worldPos - lightPos);
    float distanceFactor = clamp(1.0 - (dist / maxDistance), 0.0, 1.0);
    distanceFactor = distanceFactor * distanceFactor;

    float cookieFactor = texture(cookieTexture, clampedUV).r;
    return lightColor * cookieFactor * fadeFactor * distanceFactor;
}

//vec3 GetDirectionalLighting(vec3 WorldPos, vec3 Normal, vec3 baseColor, float roughness, float metallic, vec3 viewPos, vec3 lightDir, vec3 lightColor, float strength, float fresnelReflect) {
//	vec3 viewDir = normalize(viewPos - WorldPos);
//	float irradiance = max(dot(lightDir, Normal), 0.0) * strength;
//	vec3 brdf = microfacetBRDF(lightDir, viewDir, Normal, baseColor, metallic, fresnelReflect, roughness);
//    return brdf * irradiance * clamp(lightColor, 0, 1);
//}














// These are duplicates of what is also in the pbr common shader, only the ocean frag shader uses these so maybe organise better bro
// These are duplicates of what is also in the pbr common shader, only the ocean frag shader uses these so maybe organise better bro
// These are duplicates of what is also in the pbr common shader, only the ocean frag shader uses these so maybe organise better bro
// These are duplicates of what is also in the pbr common shader, only the ocean frag shader uses these so maybe organise better bro
// These are duplicates of what is also in the pbr common shader, only the ocean frag shader uses these so maybe organise better bro
// These are duplicates of what is also in the pbr common shader, only the ocean frag shader uses these so maybe organise better bro
// These are duplicates of what is also in the pbr common shader, only the ocean frag shader uses these so maybe organise better bro
// These are duplicates of what is also in the pbr common shader, only the ocean frag shader uses these so maybe organise better bro

float D_GGX(float NoH, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NoH2 = NoH * NoH;
    float b = (NoH2 * (alpha2 - 1.0) + 1.0);
    return alpha2 / (PI * b * b);
}

float G1_GGX_Schlick(float NdotV, float roughness) {
  //float r = roughness; // original
  float r = 0.5 + 0.5 * roughness; // Disney remapping
  float k = (r * r) / 2.0;
  float denom = NdotV * (1.0 - k) + k;
  return NdotV / denom;
}

float G_Smith(float NoV, float NoL, float roughness) {
  float g1_l = G1_GGX_Schlick(NoL, roughness);
  float g1_v = G1_GGX_Schlick(NoV, roughness);
  return g1_l * g1_v;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 microfacetBRDF(in vec3 L, in vec3 V, in vec3 N, in vec3 baseColor, in float metallicness, in float fresnelReflect, in float roughness, in vec3 WorldPos) {
  // Half vector
  vec3 H = normalize(V + L);

  // Dot products
  float NoV = clamp(dot(N, V), 0.0, 1.0);
  float NoL = clamp(dot(N, L), 0.0, 1.0);
  float NoH = clamp(dot(N, H), 0.0, 1.0);
  float VoH = clamp(dot(V, H), 0.0, 1.0);
  
  // Base reflectance (F0)
  vec3 f0 = vec3(0.16 * (fresnelReflect * fresnelReflect));
  f0 = mix(f0, baseColor, metallicness);

  // Fresnel term
  vec3 F = fresnelSchlick(VoH, f0);
  
  // Specular microfacet BRDF
  float D = D_GGX(NoH, roughness);
  float G = G_Smith(NoV, NoL, roughness);
  vec3 specular = (D * G * F) / max(4.0 * NoV * NoL, 0.001);

  // Energy-conserving diffuse
  vec3 notSpecular = (1.0 - F) * (1.0 - metallicness);
  vec3 diffuse = notSpecular * baseColor / PI;

  return diffuse + specular;
}

vec3 microfacetSpecular(in vec3 L, in vec3 V, in vec3 N, in vec3 F0, in float roughness) {  
    vec3 H = normalize(L + V);  

    float NoV = clamp(dot(N, V), 0.0, 1.0);  
    float NoL = clamp(dot(N, L), 0.0, 1.0);  
    float NoH = clamp(dot(N, H), 0.0, 1.0);  
    float VoH = clamp(dot(V, H), 0.0, 1.0);  

    float D = D_GGX(NoH, roughness);  
    float G = G_Smith(NoV, NoL, roughness);  
    vec3  F = fresnelSchlick(VoH, F0);  

    return (D * G * F) / max(4.0 * NoV * NoL, 0.001);  
}