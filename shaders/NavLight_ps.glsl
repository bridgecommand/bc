#version 130

    /*Shader for Open GL*/

uniform sampler2D tex;
varying vec4 vertexColor;
uniform float lightLevel; //0 for dark, 1.0 for full brightness

void main()
{
    //Basic vertex colour multiplied by texture
    vec4 color = texture2D(tex,gl_TexCoord[0].st)*vertexColor;

    //Fade out when light is bright (lightLevel approaches 1.0)
    float alpha = clamp(1.0-lightLevel,0.0,1.0);
    color.a = alpha*color.a;

    //Assume linear fog
    float z = gl_FragCoord.z / gl_FragCoord.w;
    float fogFactor = (gl_Fog.end - z) / (gl_Fog.end - gl_Fog.start);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    //Apply fog
    gl_FragColor = mix(gl_Fog.color, color, fogFactor );

}
