#version 130

//From Mel demo (http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=51130&start=15#p296723)

    /*Shader for Open GL*/
    uniform sampler2D baseMap;
    uniform samplerCube reflectionMap;

    varying vec2 Texcoord;
    varying vec3 Normal;
    varying vec3 ViewDirection;

    void main()
    {
        vec4 color = texture(baseMap,Texcoord);
        vec3 reflection = normalize(reflect(ViewDirection,Normal));
        vec4 refl = texture(reflectionMap,reflection);

        //James: Fog
        float fogFactor;
		float z = gl_FragCoord.z / gl_FragCoord.w;
		//Assume linear fog
        fogFactor = (gl_Fog.end - z) / (gl_Fog.end - gl_Fog.start);
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        gl_FragColor = mix(gl_Fog.color, color*refl, fogFactor );
        //gl_FragColor = color*refl;
    }

