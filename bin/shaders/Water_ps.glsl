#version 130

    /*Shader for Open GL*/

    /*Based on shader for realisticWaterSceneNode:
    Copyright (c) 2007, elvman

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    */

    uniform sampler2D	reflectionMap; //Reflection texture

    uniform float lightLevel;
    uniform float seaState;

    varying vec3 Normal;
    varying vec3 ViewDirection;
    varying vec3 reflectionMapTexCoord;

    void main()
    {

        //Assume linear fog
        float z = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = (gl_Fog.end - z) / (gl_Fog.end - gl_Fog.start);
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        //Use distance for smoothing as well
        float distanceSmoothing = clamp(1-z/300,0.0,1.0);

        //Simple shading
        vec3 color = vec3(0.18,0.29,0.31)*lightLevel;
        vec3 reflection = reflect(ViewDirection,Normal);
        float brightness = 0.5+0.5*dot(reflection,vec3(0,1,0));
        //flatten shading at long range (to avoid clear repetition of waves)
        brightness = mix(1.0,brightness,distanceSmoothing);
        vec4 simpleShading = vec4(color*brightness,1.0);

        //Reflections
        vec2 perturbation = Normal.xz*distanceSmoothing;
        vec2 ProjectedReflectionTexCoords = reflectionMapTexCoord.xy / reflectionMapTexCoord.z + perturbation;

        ProjectedReflectionTexCoords = ProjectedReflectionTexCoords;
        ProjectedReflectionTexCoords = clamp(ProjectedReflectionTexCoords,0.0,1.0);
        vec4 reflectionColour = texture2D(reflectionMap, ProjectedReflectionTexCoords );

        //Fade out reflection at high sea state:
        reflectionColour = mix(reflectionColour,vec4(0.0,0.0,0.0,1.0),seaState/12.0);

        //Mix the two, more reflection at night so lights show up
        vec4 outputColour = mix(reflectionColour, simpleShading,0.8+lightLevel/6.0);

        //Todo: Think about white shading if normal is near upwards, and height is high (i.e. at a crest)

        gl_FragColor = mix(gl_Fog.color, outputColour, fogFactor );

    }

