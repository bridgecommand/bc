// Burning Video undefine for OpenGL Shader Emulation
#undef vec2
#undef vec3
#undef vec4
#undef mat4
#undef uniform
#undef attribute
#undef varying
#undef gl_TexCoord
#undef gl_FrontColor
#undef gl_BackColor
#undef ftransform
#undef gl_NormalMatrix
#undef gl_ModelViewMatrix
#undef gl_ModelViewProjectionMatrix

#undef gl_FragColor
#undef sampler2D
#undef samplerCube
#undef max
#undef texture2D
#undef texture2DFix
#undef textureCube
#undef rgb
#undef xyz
#undef st

#undef return
#undef discard

#undef burning_shader_vertex
#undef burning_shader_fragment_fix
#undef burning_shader_fixpoint
#undef burning_shader_emu

#ifdef _MSC_VER
#pragma warning (default: 4244) // conversion
#pragma warning (default: 4305) // truncation
#endif
