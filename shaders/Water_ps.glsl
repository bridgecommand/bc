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

const float LOG2 = 1.442695;

uniform vec3		CameraPosition;  // Position of main position
uniform float		WaveHeight;

uniform vec4		WaterColor;
uniform float		ColorBlendFactor;

uniform sampler2D	WaterBump; //coverage
//uniform sampler2D	RefractionMap; //coverage
uniform sampler2D	ReflectionMap; //coverage

//Hard code for FogEnabled, FogMode == Linear
//uniform bool		FogEnabled;
//uniform int		FogMode;

varying vec2 bumpMapTexCoord;
varying vec2 bumpMapTexCoord2;
//varying vec3 refractionMapTexCoord;
varying vec3 reflectionMapTexCoord;
varying vec3 position3D;

void main()
{
	//bump color
	vec4 bumpColor = texture2D(WaterBump, bumpMapTexCoord);
	vec4 rippleColor = texture2D(WaterBump, bumpMapTexCoord2);
	vec2 perturbation = WaveHeight * (0.8*(bumpColor.rg - 0.5) + 0.2*(rippleColor.rg - 0.5));

	//refraction
	//vec2 ProjectedRefractionTexCoords = clamp(refractionMapTexCoord.xy / refractionMapTexCoord.z + perturbation, 0.0, 1.0);
	//calculate final refraction color
	//vec4 refractiveColor = texture2D(RefractionMap, ProjectedRefractionTexCoords );

	//reflection
	vec2 ProjectedReflectionTexCoords = clamp(reflectionMapTexCoord.xy / reflectionMapTexCoord.z + perturbation, 0.0, 1.0);
	//calculate final reflection color

	/*
	vec2 ProjectedReflectionTexCoords1 = clamp(ProjectedReflectionTexCoords+vec2(-0.0025,-0.0025), 0.0, 1.0);
	vec2 ProjectedReflectionTexCoords2 = clamp(ProjectedReflectionTexCoords+vec2(-0.0025, 0.0  ), 0.0, 1.0);
	vec2 ProjectedReflectionTexCoords3 = clamp(ProjectedReflectionTexCoords+vec2(-0.0025, 0.0025), 0.0, 1.0);
	vec2 ProjectedReflectionTexCoords4 = clamp(ProjectedReflectionTexCoords+vec2( 0.0,  -0.0025), 0.0, 1.0);
	vec2 ProjectedReflectionTexCoords5 = clamp(ProjectedReflectionTexCoords+vec2( 0.0,   0.0025), 0.0, 1.0);
	vec2 ProjectedReflectionTexCoords6 = clamp(ProjectedReflectionTexCoords+vec2( 0.0025,-0.0025), 0.0, 1.0);
	vec2 ProjectedReflectionTexCoords7 = clamp(ProjectedReflectionTexCoords+vec2( 0.0025, 0.0  ), 0.0, 1.0);
	vec2 ProjectedReflectionTexCoords8 = clamp(ProjectedReflectionTexCoords+vec2( 0.0025, 0.0025), 0.0, 1.0);
    */

	vec4 color0 = texture2D(ReflectionMap, ProjectedReflectionTexCoords );
	/*vec4 color1 = texture2D(ReflectionMap, ProjectedReflectionTexCoords1);
	vec4 color2 = texture2D(ReflectionMap, ProjectedReflectionTexCoords2);
	vec4 color3 = texture2D(ReflectionMap, ProjectedReflectionTexCoords3);
	vec4 color4 = texture2D(ReflectionMap, ProjectedReflectionTexCoords4);
	vec4 color5 = texture2D(ReflectionMap, ProjectedReflectionTexCoords5);
	vec4 color6 = texture2D(ReflectionMap, ProjectedReflectionTexCoords6);
	vec4 color7 = texture2D(ReflectionMap, ProjectedReflectionTexCoords7);
	vec4 color8 = texture2D(ReflectionMap, ProjectedReflectionTexCoords8);*/

	//vec4 reflectiveColor = (color0+color1+color2+color3+color4+color5+color6+color7+color8) / 9.0;
	vec4 reflectiveColor = color0;

	float xDisp = ProjectedReflectionTexCoords.x-0.5; //-0.5->0.5
	float yDisp = ProjectedReflectionTexCoords.y-0.5; //-0.5->0.5
	float centralDisp = sqrt(xDisp*xDisp)+sqrt(yDisp*yDisp); //0.0-1.0
	vec4 ambientColor= (1.0-centralDisp)*WaterColor;

	//fresnel
	/*
	vec3 eyeVector = normalize(CameraPosition - position3D);
	vec3 upVector = vec3(0.0, 1.0, 0.0);

	//fresnel can not be lower than 0
	float fresnelTerm = max( dot(eyeVector, upVector), 0.0 );
	fresnelTerm=0;
	*/

	//float fogFactor = 1.0;
	float fogFactor;




    //if (FogEnabled)
	//{
		float z = gl_FragCoord.z / gl_FragCoord.w;
		/*
		if (FogMode == 0) //exp
		{
			fogFactor = exp2(-gl_Fog.density * z * LOG2);
			fogFactor = clamp(fogFactor, 0.0, 1.0);
		}
		else if (FogMode == 1) //linear
		{
		*/
		//Assume linear fog
        fogFactor = (gl_Fog.end - z) / (gl_Fog.end - gl_Fog.start);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        /*
		}
		else if (FogMode == 2) //exp2
		{
			fogFactor = exp2(-gl_Fog.density * gl_Fog.density * z * z * LOG2);
			fogFactor = clamp(fogFactor, 0.0, 1.0);
		}
	}
	*/

    //vec4 combinedColor = refractiveColor * fresnelTerm + reflectiveColor * (1.0 - fresnelTerm);
    vec4 combinedColor = reflectiveColor;

	vec4 finalColor = ColorBlendFactor * ambientColor + (1.0 - ColorBlendFactor) * combinedColor;

    gl_FragColor = mix(gl_Fog.color, finalColor, fogFactor );

}

