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

    uniform mat4 matViewInverse; //We need to move the normal into world space so the reflections are accurate
    uniform mat4 WorldReflectionViewProj; // World * Reflection View * Projection transformation

    varying vec3 reflectionMapTexCoord;
    varying vec3 Normal;
    varying vec3 ViewDirection;


    void main()
    {
       gl_Position = ftransform();
       Normal      = gl_NormalMatrix * gl_Normal;
       ViewDirection = normalize(gl_Position.xyz);
       Normal      = normalize((matViewInverse*vec4(Normal,0)).xyz);

       // reflection texcoords
       vec4 pos = WorldReflectionViewProj * gl_Vertex;
       reflectionMapTexCoord.x = 0.5 * (pos.w + pos.x);
       reflectionMapTexCoord.y = 0.5 * (pos.w + pos.y);
       reflectionMapTexCoord.z = pos.w;

    }

