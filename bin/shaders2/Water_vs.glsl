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

//uniform mat4	View;
uniform mat4	WorldViewProj;  // World * View * Projection transformation
uniform mat4	WorldReflectionViewProj;  // World * Reflection View * Projection transformation

uniform float	WaveLength;

uniform float	Time;
uniform float	WindForce;
uniform vec2	WindDirection;

// Vertex shader output structure
varying vec2 bumpMapTexCoord;
varying vec3 refractionMapTexCoord;
varying vec3 reflectionMapTexCoord;
varying vec3 position3D;

void main()
{
	//color = gl_Color;

	// transform position to clip space
	vec4 pos = WorldViewProj * gl_Vertex;
	gl_Position = pos;
	
	// calculate vawe coords
	bumpMapTexCoord = gl_MultiTexCoord0.xy / WaveLength + Time * WindForce * WindDirection;

	// refraction texcoords
	refractionMapTexCoord.x = 0.5 * (pos.w + pos.x);
	refractionMapTexCoord.y = 0.5 * (pos.w + pos.y);
	refractionMapTexCoord.z = pos.w;
								
	// reflection texcoords
	pos = WorldReflectionViewProj * gl_Vertex;
	reflectionMapTexCoord.x = 0.5 * (pos.w + pos.x);
	reflectionMapTexCoord.y = 0.5 * (pos.w + pos.y);
	reflectionMapTexCoord.z = pos.w;
	
	// position of the vertex
	position3D = gl_Vertex.xyz;
}