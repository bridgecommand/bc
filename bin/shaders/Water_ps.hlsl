/*
 * Copyright (c) 2013, elvman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY elvman ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL elvman BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

float3		CameraPosition;  // Position of main position
float		WaveHeight;

float4		WaterColor;
float		ColorBlendFactor;

sampler2D	WaterBump; //coverage
sampler2D	RefractionMap; //coverage
sampler2D	ReflectionMap; //coverage

// Pixel shader output structure
struct PS_OUTPUT
{
    float4 color : COLOR0;  // Pixel color    
};

struct PS_INPUT
{
	float4 position					: POSITION;   // vertex position
	
	float2 bumpMapTexCoord			: TEXCOORD0;
	float3 refractionMapTexCoord	: TEXCOORD1;
	float3 reflectionMapTexCoord	: TEXCOORD2;
		
	float3 position3D				: TEXCOORD3;
};
	
PS_OUTPUT main( PS_INPUT input )
{ 
	PS_OUTPUT output;
	
	//bump color
	float4 bumpColor = tex2D(WaterBump, input.bumpMapTexCoord);
	float2 perturbation = WaveHeight * (bumpColor.rg - 0.5);
	
	//refraction
	float2 ProjectedRefractionTexCoords = saturate(input.refractionMapTexCoord.xy / input.refractionMapTexCoord.z + perturbation);
	//calculate final refraction color
	float4 refractiveColor = tex2D(RefractionMap, ProjectedRefractionTexCoords );
	
	//reflection
	float2 ProjectedReflectionTexCoords = saturate(input.reflectionMapTexCoord.xy / input.reflectionMapTexCoord.z + perturbation);
	//calculate final reflection color
	float4 reflectiveColor = tex2D(ReflectionMap, ProjectedReflectionTexCoords );

	//fresnel
	float3 eyeVector = normalize(CameraPosition - input.position3D);
	float3 upVector = float3(0.0, 1.0, 0.0);
	
	//fresnel can not be lower than 0
	float fresnelTerm = max( dot(eyeVector, upVector), 0.0 );
	
	float4 combinedColor = refractiveColor * fresnelTerm + reflectiveColor * (1.0 - fresnelTerm);
	
	output.color = ColorBlendFactor * WaterColor + (1.0 - ColorBlendFactor) * combinedColor;

	return output;
}

