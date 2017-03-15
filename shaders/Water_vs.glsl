#version 130

//From Mel demo (http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=51130&start=15#p296723)
    /*Shader for Open GL*/

    uniform mat4 matViewInverse; //We need to move the normal into world space so the reflections are accurate
    uniform mat4 WorldReflectionViewProj; // World * Reflection View * Projection transformation

    varying vec3 reflectionMapTexCoord;

    //varying vec2 Texcoord;
    varying vec3 Normal;
    //varying vec3 ViewDirection;

    //varying float distToCamera;

    void main()
    {
       gl_Position = ftransform();
       //Texcoord    = gl_MultiTexCoord0.xy;
       Normal      = gl_NormalMatrix * gl_Normal;
       //ViewDirection = normalize(gl_Position.xyz);
       Normal      = normalize((matViewInverse*vec4(Normal,0)).xyz);
       //ViewDirection = (matViewInverse*vec4(ViewDirection,0)).xyz;

       // reflection texcoords
       vec4 pos = WorldReflectionViewProj * gl_Vertex;
       reflectionMapTexCoord.x = 0.5 * (pos.w + pos.x);
       reflectionMapTexCoord.y = 0.5 * (pos.w + pos.y);
       reflectionMapTexCoord.z = pos.w;

    }

