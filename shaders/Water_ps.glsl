#version 130

//From Mel demo (http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=51130&start=15#p296723)

    /*Shader for Open GL*/
    //uniform sampler2D baseMap;
    //uniform samplerCube reflectionMap;
    uniform sampler2D	reflectionMap; //coverage

    //uniform float LightLevel; //

    //varying vec2 Texcoord;
    varying vec3 Normal;
    //varying vec3 ViewDirection;

    varying vec3 reflectionMapTexCoord;

    //varying float distToCamera; //From vertex shader

    void main()
    {

        /*

        vec4 color = vec4(0.18,0.29,0.31,1.0);//texture(baseMap,Texcoord);
        vec3 reflection = reflect(ViewDirection,Normal);

        //vec4 refl = texture(reflectionMap,reflection); //Temporarily not used - seems to break on my implementation of GLSL for some reason

        //Proportional to distance from camera
        float z = gl_FragCoord.z / gl_FragCoord.w;
        float smoothFactor;
        smoothFactor = (500 - z) / (500 - 50);
        smoothFactor = clamp(smoothFactor, 0.0, 1.0);

        //Alternative simple shading:
        float brightness = 0.5+0.5*dot(reflection,vec3(0,1,0));
        //flatten shading at long range (to avoid clear repetition of waves)
        brightness = mix(1.0,brightness,smoothFactor);
        vec4 simpleShading = vec4(brightness,brightness,brightness,1.0);


        //vec4 plainColor = vec4(0.1,0.1,1.0,1.0);
        //float distanceFactor = clamp(distToCamera,1,10)/10.0;
        //finalColor = mix(finalColor,plainColor,distanceFactor);

        //James: Fog
        float fogFactor;
		//Assume linear fog
        fogFactor = (gl_Fog.end - z) / (gl_Fog.end - gl_Fog.start);
        fogFactor = clamp(fogFactor, 0.0, 1.0);



        //gl_FragColor = mix(gl_Fog.color, color*refl, fogFactor ); //Cubemap
        //gl_FragColor = LightLevel*mix(gl_Fog.color, color*simpleShading, fogFactor ); //Simple shading
        gl_FragColor = LightLevel*mix(gl_Fog.color, color*simpleShading, fogFactor ); //Simple shading
        */
        vec2 perturbation = (Normal.xz);
        vec2 ProjectedReflectionTexCoords = clamp(reflectionMapTexCoord.xy / reflectionMapTexCoord.z + perturbation, 0.0, 1.0);
        vec4 color0 = texture2D(reflectionMap, ProjectedReflectionTexCoords );

        //Assume linear fog
        float z = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = (gl_Fog.end - z) / (gl_Fog.end - gl_Fog.start);
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        gl_FragColor = mix(gl_Fog.color, color0, fogFactor );


    }

