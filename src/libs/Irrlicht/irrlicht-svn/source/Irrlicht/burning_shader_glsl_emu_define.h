// Burning Video define for OpenGL Shader Emulation
#define gl_Position dest->Pos
#define gl_FrontColor dest->Color[0]
#define gl_BackColor dest->Color[1]
#define vec2 glslEmu::vec2
#define vec3 glslEmu::vec3
#define vec4 glslEmu::vec4
#define sampler2D glslEmu::sampler2D
#define samplerCube glslEmu::samplerCube
#define mat4 core::matrix4
#define gl_NormalMatrix matrix[ETS_NORMAL]
#define gl_ModelViewMatrix matrix[ETS_MODEL_VIEW]
#define gl_ModelViewProjectionMatrix matrix[ETS_MODEL_VIEW_PROJ]
#define ftransform() (matrix[ETS_MODEL_VIEW_PROJ] * gl_Vertex)

#define rgb _xyz()
#define xyz _xyz()
#define st _st()

#define attribute

#define uniform bl_uniform
#define varying(var,name) var& name = (var&)dest->Color[glsl.varying_count++]

#define texture2D(sampler,coord) BL_Sampler_nearest(sampler,coord,2,glsl.MaterialLink)
#define textureCube(sampler,coord) BL_Sampler_nearest(sampler,coord,3,glsl.MaterialLink)

#if defined(burning_shader_emu)
#define discard _discard+=1,return
#else

#define gl_FragColor dst[_sx]

#if defined(burning_shader_vertex)
vec4 gl_Color;
#define gl_TexCoord dest->Tex
#elif defined(burning_shader_fragment_fix)
sVec4Fix gl_Color;
tFixPoint gl_TexCoord[4];
#elif defined(burning_shader_fragment_emu)
vec2 gl_TexCoord[2];
#else
#endif

#define return goto BL_shader_return
#define discard goto BL_shader_return

#endif // if defined(burning_shader_emu)


#ifdef _MSC_VER
#pragma warning (disable: 4244) // float/double conversion
#pragma warning (disable: 4305) // truncation
#endif
