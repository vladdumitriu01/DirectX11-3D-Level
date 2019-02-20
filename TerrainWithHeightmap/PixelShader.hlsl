#include "Header.hlsli"


//Fog

cbuffer FogBuffer {
	float3 eyePos;
	float fogStart;
	float fogRange;
	float4 fogColour;

};
// Image data and sampler
Texture2D imageData : register(t1);
SamplerState colourSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_Target
{
	float4 texColour = imageData.Sample(colourSampler, input.texCoord);

	float3 toEye = eyePos;
	float dist = length(toEye);
	float linearFog = saturate((dist - fogStart) / fogRange);
	texColour = lerp(texColour, fogColour, linearFog);
	return texColour;

}
