//From Mel demo (http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=51130&start=15#p296723)

    /*Shader for Open GL*/
    uniform sampler2D baseMap;
    uniform samplerCube reflectionMap;

    varying vec2 Texcoord;
    varying vec3 Normal;
    varying vec3 ViewDirection;

    void main()
    {
        vec4 color = texture2D(baseMap,Texcoord); //was texture(baseMap,Texcoord)
        vec3 reflection = normalize(reflect(ViewDirection,Normal));
        vec4 refl = textureCube(reflectionMap,reflection); //was texture(reflectionMap,reflection)
        gl_FragColor = color*refl;
    }

