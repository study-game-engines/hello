
// By Tokyospliff ©
//
// All rights reserved 2025 till forever

#version 450 core

layout (location = 0) out vec4 FinalLightingOut;

layout (binding = 0) uniform sampler2D DepthTexture;

uniform vec3 color;
uniform float alpha;

uniform vec2 screensize;
uniform float near;
uniform float far;
in vec3 normal;
in vec3 viewDir;

float LinearizeDepth(float depth) {
	float zNdc = 2 * depth - 1;
	float zEye = (2 * far * near) / ((far + near) - zNdc * (far - near));
	float linearDepth = (zEye - near) / (far - near);
	return linearDepth;
}

void main666() {

}

void main() {

	// - Job: Rim lighting -
	vec3 viewAngle = normalize(-viewDir);

	// The more orthogonal the camera is to the fragment, the stronger the rim light.
	// abs() so that the back faces get treated the same as the front, giving a rim effect.
	float rimStrength = 1 - abs(dot(viewAngle, normal)); // The more orthogonal, the stronger

	float rimFactor = pow(rimStrength, 4); // higher power = sharper rim light
	vec4 rim = vec4(rimFactor);

	// - Create the intersection line -
	// Turn frag coord from screenspace -> NDC, which corresponds to the UV
	vec2 depthUV = gl_FragCoord.xy / screensize;
	float sceneDepth = texture(DepthTexture, depthUV).r;
	float bubbleDepth = LinearizeDepth(gl_FragCoord.z);

	float distance = abs(bubbleDepth - sceneDepth); // linear difference in depth 

	float threshold = 0.001;	
	threshold = 0.5;
	float normalizedDistance = clamp(distance / threshold, 0.0, 1.0); // [0, threshold] -> [0, 1]

	vec4 intersection = mix(vec4(1), vec4(0), normalizedDistance); // white to transparent gradient

	vec4 bubbleBase = vec4(color, alpha);

	FinalLightingOut = bubbleBase + intersection + rim;
}