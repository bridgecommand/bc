#version 130

//From Mel demo (http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=51130&start=15#p296723)

    /*Shader for Open GL*/
    uniform sampler2D baseMap;
    uniform samplerCube reflectionMap;

    uniform float LightLevel; //

    varying vec2 Texcoord;
    varying vec3 Normal;
    varying vec3 ViewDirection;

    void main()
    {
        vec4 color = texture(baseMap,Texcoord);
        vec3 reflection = reflect(ViewDirection,Normal);

        //vec4 refl = texture(reflectionMap,reflection); //Temporarily not used - seems to break on my implementation of GLSL for some reason

        //Alternative simple shading:
        float brightness = 0.5+0.5*dot(reflection,vec3(0,1,0));
        vec4 simpleShading = vec4(brightness,brightness,brightness,1.0);


        //James: Fog
        float fogFactor;
		float z = gl_FragCoord.z / gl_FragCoord.w;
		//Assume linear fog
        fogFactor = (gl_Fog.end - z) / (gl_Fog.end - gl_Fog.start);
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        //gl_FragColor = mix(gl_Fog.color, color*refl, fogFactor ); //Cubemap
        gl_FragColor = LightLevel*mix(gl_Fog.color, color*simpleShading, fogFactor ); //Simple shading

    }

