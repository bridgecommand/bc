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

//float4x4	View;
float4x4	WorldViewProj;  // World * View * Projection transformation
float4x4	WorldReflectionViewProj;  // World * Reflection View * Projection transformation

float		WaveLength;

float		Time;
float		WindForce;
float2		WindDirection;

// Vertex shader output structure
struct VS_OUTPUT
{
	float4 position					: POSITION;   // vertex position
	
	float2 bumpMapTexCoord			: TEXCOORD0;
	float3 refractionMapTexCoord	: TEXCOORD1;
	float3 reflectionMapTexCoord	: TEXCOORD2;
	
	float3 position3D				: TEXCOORD3;
};

struct VS_INPUT
{
	float4 position		: POSITION;
	float4 color		: COLOR0;
	float2 texCoord0	: TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	// transform position to clip space
	float4 pos = mul(input.position, WorldViewProj);
	output.position = pos;
	
	// calculate vawe coords
	output.bumpMapTexCoord = input.texCoord0 / WaveLength + Time * WindForce * WindDirection;

	// refraction texcoords
	output.refractionMapTexCoord.x = 0.5 * (pos.w + pos.x);
	output.refractionMapTexCoord.y = 0.5 * (pos.w - pos.y);
	output.refractionMapTexCoord.z = pos.w;
								
	// reflection texcoords
	pos = mul(input.position, WorldReflectionViewProj);	
	output.reflectionMapTexCoord.x = 0.5 * (pos.w + pos.x);
	output.reflectionMapTexCoord.y = 0.5 * (pos.w - pos.y);
	output.reflectionMapTexCoord.z = pos.w;
	
	// position of the vertex
	output.position3D = input.position;
	
	return output;
}