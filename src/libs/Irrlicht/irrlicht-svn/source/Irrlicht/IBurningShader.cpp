// Copyright (C) 2002-2022 Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_

#include "SoftwareDriver2_compile_config.h"
#include "IBurningShader.h"
#include "CSoftwareDriver2.h"
#include "IShaderConstantSetCallBack.h"

burning_namespace_start

const tFixPointu IBurningShader::dithermask[] =
{
	0x00,0x80,0x20,0xa0,
	0xc0,0x40,0xe0,0x60,
	0x30,0xb0,0x10,0x90,
	0xf0,0x70,0xd0,0x50
};

// RenderTarget with attached buffers
void sBurningRenderTarget::shader_constructor_grab(sBurningRenderTarget& target)
{
	//todo. this links directly to driver
	backbuffer = 0;

	depth = target.depth;
	if (depth)
		depth->grab();

	stencil = target.stencil;
	if (stencil)
		stencil->grab();

	// interlaced_disabled
	interlaced.raw = 0;
	interlaced.m.bypass = 1;

	texture = 0;
	color = 0;
}

void sBurningRenderTarget::drop()
{
	if (backbuffer)
	{
		backbuffer->drop();
		backbuffer = 0;
	}

	if (depth)
	{
		depth->drop();
		depth = 0;
	}

	if (stencil)
	{
		stencil->drop();
		stencil = 0;
	}

	if (texture)
	{
		texture->drop();
		texture = 0;
	}

	if (color)
	{
		color->drop();
		color = 0;
	}

}

void IBurningShader::constructor_IBurningShader(CBurningVideoDriver* driver, E_MATERIAL_TYPE baseMaterial)
{
#ifdef _DEBUG
	setDebugName("IBurningShader");
#endif

#if defined(ENV64BIT)
	if (((unsigned long long) & scan & 15) || ((unsigned long long) & line & 15))
	{
		os::Printer::log("BurningVideo Shader not 16 byte aligned", ELL_ERROR);
		IRR_DEBUG_BREAK_IF(1);
	}
#endif

	//Interlaced.m.enable = 0;
	//Interlaced.m.bypass = 1;
	//Interlaced.m.nr = 0;

	EdgeTestPass = edge_test_pass;

	memset(IT, 0, sizeof(IT));

	Driver = driver;
	CallBack = 0;

	RenderTarget.shader_constructor_grab(driver->getRenderTarget());

	stencilOp[0] = StencilOp_KEEP;
	stencilOp[1] = StencilOp_KEEP;
	stencilOp[2] = StencilOp_KEEP;
	AlphaRef = 0;
	PrimitiveColor = COLOR_MAX_SAMPLE;
	TL_Flag = 0;
	fragment_draw_count = 0;

	VertexShaderProgram.init(BVT_Fix);
	FragmentShaderProgram.init(BFT_Fix);

	//set default Transparent/Solid
	BaseMaterial = baseMaterial;
	switch (BaseMaterial)
	{
	case EMT_TRANSPARENT_ADD_COLOR:
	case EMT_TRANSPARENT_ALPHA_CHANNEL:
	case EMT_TRANSPARENT_ALPHA_CHANNEL_REF:
	case EMT_TRANSPARENT_VERTEX_ALPHA:
	case EMT_TRANSPARENT_REFLECTION_2_LAYER:
	case EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR:
	case EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA:
	case EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR:
	case EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA:
	case EMT_ONETEXTURE_BLEND:
		RenderPass_ShaderIsTransparent = 1;
		break;
	default:
		RenderPass_ShaderIsTransparent = 0;
		break;
	}
	glsl.MaterialLink = 0;
#if defined(burning_glsl_emu_test)
	glsl.glsl.MaterialLink = 0;
#endif
}

IBurningShader::IBurningShader(CBurningVideoDriver* driver, E_MATERIAL_TYPE baseMaterial)
{
	constructor_IBurningShader(driver, baseMaterial);
}

void BVCompiledShader::init(eBurningVertexFragmentShaderID _id)
{
	id = _id;
	shaderType = _id == BVT_Fix ? BV_VERTEX_SHADER : BV_FRAGMENT_SHADER;
	vertexFormat.Format.raw = 0;
	vertexFormat.Pitch = 0;
	vertexFormat.TexSize = 0;
	vertexFormat.TexCooSize = 0;
	vertexFormat.ColSize = 0;
	vertexFormat.VaryingSize = 0;
}

/*
* https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_vertex_program.txt
*
| Generic Attribute |  Conventional Attribute  | Conventional Attribute Command |
|-------------------|--------------------------|--------------------------------|
| 0                 | vertex position          | Vertex                         |
| 1                 | vertex weights 0-3       | WeightARB, VertexWeightEXT     |
| 2                 | normal                   | Normal                         |
| 3                 | primary color            | Color                          |
| 4                 | secondary color          | SecondaryColorEXT              |
| 5                 | fog coordinate           | FogCoordEXT                    |
| 6                 | -                        | -                              |
| 7                 | -                        | -                              |
| 8+n               | texture coordinate set 0 | MultiTexCoord(TEXTURE0, ...    |

glVertexPointer = glVertexAttribPointer (0
glNormalPointer = glVertexAttribPointer (2
glColorPointer = glVertexAttribPointer (3
glTexCoordPointer = glVertexAttribPointer(8

gl_Vertex = Pos
gl_Color = Color
gl_Normal = Normal
gl_MultiTexCoord0 = TCoords

S3DVertex
	gl_MultiTexCoord1 = TCoords
S3DVertex2TCoords
	gl_MultiTexCoord1 = TCoords2
S3DVertexTangents
	gl_MultiTexCoord1.xyz = Tangent
	gl_MultiTexCoord2.xyz = Binormal

*/
static void BV_CompileShader(BVCompiledShader& out, eBurningVertexFragmentShaderID shaderType, const c8* source)
{
	unsigned hash = 0;
	unsigned len = 0;

	//lexer
	if (source)
	{
		const c8* ip = source;
		while (ip[len])
		{
			hash = ip[len] + (hash << 6) + (hash << 16) - hash;
			len += 1;
		}
	}

	out.shaderType = shaderType;
	out.id = BVT_Fix;
	out.vertexFormat.TexSize = 1;
	out.vertexFormat.TexCooSize = 1;
	out.vertexFormat.ColSize = 0;
	out.vertexFormat.VaryingSize = 0;

	//VertexShader
	if (shaderType == BV_VERTEX_SHADER)
	{
		if (len == 1100 && hash == 0x12c79d1c)
		{
			/* example 10 opengl.vert */
			out.id = BVT_IRR_1100_0x12c79d1c;
			out.vertexFormat.ColSize = 1;
		}
		else if (len == 815 && hash == 0x1f847599) out.id = BVT_IRR_0815_0x1f847599; /* example 27 pp_opengl.vert */
		else if (len == 837 && hash == 0x884022e6)
		{
			out.id = BVT_IRR_0837_0x884022e6; /* example 28 cubeMapReflection.vert */
			// use dest->Col[0] as gl_TexCoord[0] 
			out.vertexFormat.ColSize = 1;
			out.vertexFormat.TexCooSize = 0;
		}
#if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader)
		else if (len == 1259 && hash == 0xc8226e1a)
		{
			out.id = BVT_STK_1259_0xc8226e1a; /* supertuxkart bubble.vert */
			out.vertexFormat.ColSize = 1;
		}
		else if (len == 958 && hash == 0xa048973b)	out.id = BVT_STK_0958_0xa048973b; /* supertuxkart motion_blur.vert */
		else if (len == 1309 && hash == 0x1fd689c2)
		{
			out.id = BVT_STK_1309_0x1fd689c2; /* supertuxkart normalmap.vert */
			out.vertexFormat.TexSize = 3;
			out.vertexFormat.TexCooSize = 1;
			out.vertexFormat.VaryingSize = 3;
		}
		else if (len == 1204 && hash == 0x072a4094)
		{
			out.id = BVT_STK_1204_0x072a4094; /* supertuxkart splatting.vert */
			out.vertexFormat.TexSize = 6;
			out.vertexFormat.TexCooSize = 2;
			out.vertexFormat.ColSize = 1;
		}
		else if (len == 1060 && hash == 0x07eb3472)
		{
			out.id = BVT_STK_1060_0x07eb3472; /* supertuxkart splatting2.vert (vertex color)*/
			out.vertexFormat.TexSize = 6;
			out.vertexFormat.TexCooSize = 2;
			out.vertexFormat.ColSize = 1;
		}
		else if (len == 1303 && hash == 0xd872cdb6)
		{
			out.id = BVT_STK_1303_0xd872cdb6; /* supertuxkart water.vert */
			out.vertexFormat.TexSize = 3;
			out.vertexFormat.VaryingSize = 3;
		}
#endif // PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader
	}
	else if (shaderType == BV_FRAGMENT_SHADER)
	{
		//FragmentShader
		if (len == 225 && hash == 0x3bf0d5a1)
		{
			/* example 10 opengl.frag new */
			out.id = BFT_IRR_0225_0x3bf0d5a1;
			out.vertexFormat.ColSize = 1;
		}
		else if (len == 385 && hash == 0xa0194718) out.id = BFT_IRR_0385_0xa0194718; /* example 27 pp_opengl.frag */
		else if (len == 342 && hash == 0x11b0394b)
		{
			out.id = BFT_IRR_0342_0x11b0394b; /* example 28 cubeMapReflection.fraq */
			out.vertexFormat.ColSize = 1;
			out.vertexFormat.TexCooSize = 0;
		}
#if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader)
		else if (len == 2971 && hash == 0xd34c333c)	out.id = BFT_STK_2971_0xd34c333c; /* supertuxkart motion_blur.frag */
		else if (len == 2969 && hash == 0xe10a63fc)	out.id = BFT_STK_2971_0xd34c333c; /* supertuxkart motion_blur.frag */
		else if (len == 1840 && hash == 0x6c2f4ae2)
		{
			out.id = BFT_STK_1840_0x6c2f4ae2; /* supertuxkart splatting.frag */
			out.vertexFormat.TexSize = 6;
			out.vertexFormat.TexCooSize = 2;
			out.vertexFormat.ColSize = 1;
		}
		else if (len == 1612 && hash == 0x1e372102)
		{
			out.id = BFT_STK_1612_0x1e372102; /* supertuxkart splatting2.frag */
			out.vertexFormat.TexSize = 6;
			out.vertexFormat.TexCooSize = 2;
			out.vertexFormat.ColSize = 1;
		}
		else if (len == 1713 && hash == 0x9ca851d6)
		{
			out.id = BVT_STK_1713_0x9ca851d6; /* supertuxkart water.fraq */
			out.vertexFormat.TexSize = 3;
			out.vertexFormat.VaryingSize = 3;
		}
#endif
	}

	out.vertexFormat.Format.raw =
		out.vertexFormat.TexCooSize << 16 |
		(out.vertexFormat.ColSize + out.vertexFormat.VaryingSize) << 20;

	//save info
#if 0
	static int run = 0;
	FILE* f = fopen("shader_id.txt", run ? "a" : "wb");
	if (f)
	{
		fprintf(f, "--- start outMaterialTypeNr:%d len:%d hash: 0x%08x buildIn:%d\n"
			, outMaterialTypeNr, len, hash, ret);
		fprintf(f, "%s", source ? source : "<NULL>");
		fprintf(f, "\n-------------- end ---------------------------\n");
		fclose(f);
	}
	run += 1;
#endif

}

//Samplers materialrender here for now.
#if defined(SOFTWARE_DRIVER_2_BILINEAR)

#if 0 // old_version
// texture2D in fixpoint color range bilinear
void getSample_texture(tFixPoint& r, tFixPoint& g, tFixPoint& b,
	const sInternalTexture* burning_restrict t, const tFixPointu tx, const tFixPointu ty
)
{
	// mipmap nearest
#if 0
	if (t->lodFactor > 0)
	{
		size_t ofs;

		ofs = (((ty + FIX_POINT_ZERO_DOT_FIVE) & t->textureYMask) >> FIX_POINT_PRE) << t->pitchlog2;
		ofs += ((tx + FIX_POINT_ZERO_DOT_FIVE) & t->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

		// texel
		tTextureSample t00;
		t00 = *((tTextureSample*)((u8*)t->data + ofs));

		r = (t00 & MASK_R) >> (SHIFT_R - FIX_POINT_PRE);
		g = (t00 & MASK_G) << (FIX_POINT_PRE - SHIFT_G);
		b = (t00 & MASK_B) << (FIX_POINT_PRE - SHIFT_B);
		return;
	}
#endif

	tFixPointu r00, g00, b00;
	tFixPointu r01, g01, b01;
	tFixPointu r10, g10, b10;
	tFixPointu r11, g11, b11;

	size_t o0, o1, o2, o3;
	tTextureSample t00;

	//wraps positive (ignoring negative)
	o0 = (((ty)&t->textureYMask) >> FIX_POINT_PRE) << t->pitchlog2;
	o1 = (((ty + FIX_POINT_ONE) & t->textureYMask) >> FIX_POINT_PRE) << t->pitchlog2;
	o2 = ((tx)&t->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);
	o3 = ((tx + FIX_POINT_ONE) & t->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	t00 = *((tTextureSample*)((u8*)t->data + (o0 + o2)));
	r00 = (t00 & MASK_R) >> SHIFT_R;
	g00 = (t00 & MASK_G) >> SHIFT_G;
	b00 = (t00 & MASK_B);

	t00 = *((tTextureSample*)((u8*)t->data + (o0 + o3)));
	r10 = (t00 & MASK_R) >> SHIFT_R;
	g10 = (t00 & MASK_G) >> SHIFT_G;
	b10 = (t00 & MASK_B);

	t00 = *((tTextureSample*)((u8*)t->data + (o1 + o2)));
	r01 = (t00 & MASK_R) >> SHIFT_R;
	g01 = (t00 & MASK_G) >> SHIFT_G;
	b01 = (t00 & MASK_B);

	t00 = *((tTextureSample*)((u8*)t->data + (o1 + o3)));
	r11 = (t00 & MASK_R) >> SHIFT_R;
	g11 = (t00 & MASK_G) >> SHIFT_G;
	b11 = (t00 & MASK_B);


	tFixPointu fracx = tx & FIX_POINT_FRACT_MASK;
	tFixPointu fracy = ty & FIX_POINT_FRACT_MASK;

	//w00 w01 w10 w11
	tFixPointu w[4];
	w[0] = imulFixu(FIX_POINT_ONE - fracx, FIX_POINT_ONE - fracy);
	w[1] = imulFixu(FIX_POINT_ONE - fracx, fracy);
	w[2] = imulFixu(fracx, FIX_POINT_ONE - fracy);
	w[3] = imulFixu(fracx, fracy);

	r = (r00 * w[0]) +
		(r01 * w[1]) +
		(r10 * w[2]) +
		(r11 * w[3]);

	g = (g00 * w[0]) +
		(g01 * w[1]) +
		(g10 * w[2]) +
		(g11 * w[3]);

	b = (b00 * w[0]) +
		(b01 * w[1]) +
		(b10 * w[2]) +
		(b11 * w[3]);

}

#else // old_version

// texture2D in fixpoint color range bilinear
void getSample_texture(tFixPoint& r, tFixPoint& g, tFixPoint& b,
	const sInternalTexture* burning_restrict tex, const tFixPointu tx, const tFixPointu ty
)
{
#if 0
	if (tex->lodFactor > 1)
	{
		//nearest neighbor
		size_t ofs;
		ofs = (((ty + FIX_POINT_ZERO_DOT_FIVE) & tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
		ofs += ((tx + FIX_POINT_ZERO_DOT_FIVE) & tex->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

		tTextureSample t00;
		t00 = *((tTextureSample*)((u8*)tex->data + ofs));

		r = (t00 & MASK_R) >> (SHIFT_R - FIX_POINT_PRE);
		g = (t00 & MASK_G) << (FIX_POINT_PRE - SHIFT_G);
		b = (t00 & MASK_B) << (FIX_POINT_PRE - SHIFT_B);
		return;
	}
#endif

	//w00 w01 w10 w11
	tFixPointu ALIGN(16) w[4];
	{
		const tFixPointu fracx = tx & FIX_POINT_FRACT_MASK;
		const tFixPointu fracy = ty & FIX_POINT_FRACT_MASK;
		w[0] = imulFixu(FIX_POINT_ONE - fracx, FIX_POINT_ONE - fracy);
		w[1] = imulFixu(fracx, FIX_POINT_ONE - fracy);
		w[2] = imulFixu(FIX_POINT_ONE - fracx, fracy);
		w[3] = imulFixu(fracx, fracy);
	}

	//wraps positive (ignoring negative)
	tTextureSample ALIGN(16) t[4];
	{
		size_t o0, o1, o2, o3;

		// #if FIX_POINT_PRE > SOFTWARE_DRIVER_2_TEXTURE_MAXSIZE_LOG2 >> FIX_POINT_PRE - tex->pitchlog2

		o0 = (((ty)&tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
		o1 = (((ty + FIX_POINT_ONE) & tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
		o2 = ((tx)&tex->textureXMask) >> (unsigned)(FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);
		o3 = ((tx + FIX_POINT_ONE) & tex->textureXMask) >> (unsigned)(FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

		t[0] = *((tTextureSample*)((u8*)tex->data + (o0 + o2)));
		t[1] = *((tTextureSample*)((u8*)tex->data + (o0 + o3)));
		t[2] = *((tTextureSample*)((u8*)tex->data + (o1 + o2)));
		t[3] = *((tTextureSample*)((u8*)tex->data + (o1 + o3)));
	}

	r = (((t[0] & MASK_R) >> SHIFT_R) * w[0]) +
		(((t[1] & MASK_R) >> SHIFT_R) * w[1]) +
		(((t[2] & MASK_R) >> SHIFT_R) * w[2]) +
		(((t[3] & MASK_R) >> SHIFT_R) * w[3]);

	g = (((t[0] & MASK_G) >> SHIFT_G) * w[0]) +
		(((t[1] & MASK_G) >> SHIFT_G) * w[1]) +
		(((t[2] & MASK_G) >> SHIFT_G) * w[2]) +
		(((t[3] & MASK_G) >> SHIFT_G) * w[3]);

	b = (((t[0] & MASK_B)/*>> SHIFT_B*/) * w[0]) +
		(((t[1] & MASK_B)/*>> SHIFT_B*/) * w[1]) +
		(((t[2] & MASK_B)/*>> SHIFT_B*/) * w[2]) +
		(((t[3] & MASK_B)/*>> SHIFT_B*/) * w[3]);
}

#endif // old_version


// texture2D in fixpoint color range bilinear
void getSample_texture(tFixPoint& a, tFixPoint& r, tFixPoint& g, tFixPoint& b,
	const sInternalTexture* burning_restrict tex, const tFixPointu tx, const tFixPointu ty
)
{
	tFixPointu a00, r00, g00, b00;
	tFixPointu a01, r01, g01, b01;
	tFixPointu a10, r10, g10, b10;
	tFixPointu a11, r11, g11, b11;

	size_t o0, o1, o2, o3;
	tTextureSample t00;

	o0 = (((ty)&tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
	o1 = (((ty + FIX_POINT_ONE) & tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
	o2 = ((tx)&tex->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);
	o3 = ((tx + FIX_POINT_ONE) & tex->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	t00 = *((tTextureSample*)((u8*)tex->data + (o0 + o2)));
	a00 = (t00 & MASK_A) >> SHIFT_A;
	r00 = (t00 & MASK_R) >> SHIFT_R;
	g00 = (t00 & MASK_G) >> SHIFT_G;
	b00 = (t00 & MASK_B);

	t00 = *((tTextureSample*)((u8*)tex->data + (o0 + o3)));
	a10 = (t00 & MASK_A) >> SHIFT_A;
	r10 = (t00 & MASK_R) >> SHIFT_R;
	g10 = (t00 & MASK_G) >> SHIFT_G;
	b10 = (t00 & MASK_B);

	t00 = *((tTextureSample*)((u8*)tex->data + (o1 + o2)));
	a01 = (t00 & MASK_A) >> SHIFT_A;
	r01 = (t00 & MASK_R) >> SHIFT_R;
	g01 = (t00 & MASK_G) >> SHIFT_G;
	b01 = (t00 & MASK_B);

	t00 = *((tTextureSample*)((u8*)tex->data + (o1 + o3)));
	a11 = (t00 & MASK_A) >> SHIFT_A;
	r11 = (t00 & MASK_R) >> SHIFT_R;
	g11 = (t00 & MASK_G) >> SHIFT_G;
	b11 = (t00 & MASK_B);

	const tFixPointu txFract = tx & FIX_POINT_FRACT_MASK;
	const tFixPointu txFractInv = FIX_POINT_ONE - txFract;

	const tFixPointu tyFract = ty & FIX_POINT_FRACT_MASK;
	const tFixPointu tyFractInv = FIX_POINT_ONE - tyFract;

	const tFixPointu w00 = imulFixu(txFractInv, tyFractInv);
	const tFixPointu w10 = imulFixu(txFract, tyFractInv);
	const tFixPointu w01 = imulFixu(txFractInv, tyFract);
	const tFixPointu w11 = imulFixu(txFract, tyFract);

	a = (a00 * w00) +
		(a01 * w01) +
		(a10 * w10) +
		(a11 * w11);

	fix_alpha_color_max(a);

	r = (r00 * w00) +
		(r01 * w01) +
		(r10 * w10) +
		(r11 * w11);

	g = (g00 * w00) +
		(g01 * w01) +
		(g10 * w10) +
		(g11 * w11);

	b = (b00 * w00) +
		(b01 * w01) +
		(b10 * w10) +
		(b11 * w11);

}

// get Sample bilinear
void getSample_texture(tFixPoint& a,
	const sInternalTexture* burning_restrict tex, const tFixPointu tx, const tFixPointu ty
)
{
	tFixPointu a00;
	tFixPointu a01;
	tFixPointu a10;
	tFixPointu a11;

	size_t o0, o1, o2, o3;
	tTextureSample t00;

	o0 = (((ty)&tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
	o1 = (((ty + FIX_POINT_ONE) & tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
	o2 = ((tx)&tex->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);
	o3 = ((tx + FIX_POINT_ONE) & tex->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	t00 = *((tTextureSample*)((u8*)tex->data + (o0 + o2)));
	a00 = (t00 & MASK_A) >> SHIFT_A;

	t00 = *((tTextureSample*)((u8*)tex->data + (o0 + o3)));
	a10 = (t00 & MASK_A) >> SHIFT_A;

	t00 = *((tTextureSample*)((u8*)tex->data + (o1 + o2)));
	a01 = (t00 & MASK_A) >> SHIFT_A;

	t00 = *((tTextureSample*)((u8*)tex->data + (o1 + o3)));
	a11 = (t00 & MASK_A) >> SHIFT_A;

	const tFixPointu txFract = tx & FIX_POINT_FRACT_MASK;
	const tFixPointu txFractInv = FIX_POINT_ONE - txFract;

	const tFixPointu tyFract = ty & FIX_POINT_FRACT_MASK;
	const tFixPointu tyFractInv = FIX_POINT_ONE - tyFract;

	const tFixPointu w00 = imulFixu(txFractInv, tyFractInv);
	const tFixPointu w10 = imulFixu(txFract, tyFractInv);
	const tFixPointu w01 = imulFixu(txFractInv, tyFract);
	const tFixPointu w11 = imulFixu(txFract, tyFract);

	a = (a00 * w00) +
		(a01 * w01) +
		(a10 * w10) +
		(a11 * w11);

	fix_alpha_color_max(a);
}

#endif // defined(SOFTWARE_DRIVER_2_BILINEAR)


#if !defined(burning_glsl_emu_test) && 0
sVec4 BL_Sampler_nearest(const int sampler, const sVec4& coord, const s32 coord_count, const SBurningShaderMaterial* MaterialLink)
{
	// texel
	tTextureSample t00;
	t00 = COLOR_MAX_SAMPLE;

	sVec4 out;
	out.setTextureSample(t00);
	return out;
}
#endif

#if defined(burning_glsl_emu_test)


static inline void BL_textureWrap(sVec2& uv, const SMaterialLayer& texObj)
{
	switch (texObj.TextureWrapU)
	{
	case ETC_CLAMP_TO_BORDER:
	case ETC_CLAMP_TO_EDGE:
	case ETC_CLAMP:
		//uv.x = clampf01(uv.x);
		if (uv.x < 0.f) uv.x = 0.f;
		else if (uv.x > 1.f) uv.x = 1.f;
		break;
	default:
	case ETC_REPEAT:
		//uv.x = core::fract(uv.x);
		uv.x -= floorf(uv.x);
		break;
	}

	switch (texObj.TextureWrapV)
	{
	case ETC_CLAMP_TO_BORDER:
	case ETC_CLAMP_TO_EDGE:
	case ETC_CLAMP:
		//uv.y = clampf01(uy.x);
		if (uv.y < 0.f) uv.y = 0.f;
		else if (uv.y > 1.f) uv.y = 1.f;
		break;
	default:
	case ETC_REPEAT:
		//uv.y = core::fract(uv.y);
		uv.y -= floorf(uv.y);
		break;
	}

	//texture matrix?
	//openGL rtt
	// if (texObj.Texture->isRenderTarget())
	//	uv.y = 1.f - uv.y;

}


//MINSS - Return Minimum Scalar Single-Precision Floating-Point Value
static inline float _minss(const float a, const float b) {
	return a < b ? a : b;
}

//MAXSS - Return Maximum Scalar Single-Precision Floating-Point Value
static inline float _maxss(const float a, const float b) {
	return a > b ? a : b;
}


sVec4 BL_Sampler_nearest(const int sampler, const sVec4& coord, const s32 coord_count, const SBurningShaderMaterial* MaterialLink)
{
	const unsigned m = (unsigned)sampler < BURNING_MATERIAL_MAX_SAMPLER ? (unsigned)sampler : 0;
	const SMaterialLayer& texObj = MaterialLink->org.TextureLayer[m];
	const video::CSoftwareTexture2& tex = *(const video::CSoftwareTexture2*)texObj.Texture;
	const CSoftwareTexture2_Bound* b = tex.getTexBound_index();

	const sScanLineData& line = MaterialLink->shader_org->line;

	sVec2 uv;

	//select mipmap
	s32 lodFactor = 0;

	if (coord_count == 2) // if (tex.getType() == ETT_2D)
	{
		uv.x = coord.x * line.inversew[0];
		uv.y = coord.y * line.inversew[0];
		BL_textureWrap(uv, texObj);

		// mimap selection
#if 1  
		//dFdx ( next line.ipol_step() )
		sVec2 uv2;
		uv2.x = (coord.x + line.t[0][1].x) * line.inversew[1];
		uv2.y = (coord.y + line.t[0][1].y) * line.inversew[1];
		BL_textureWrap(uv2, texObj);
		//t[2] = uv2.x * b[0].mat[0];// + b[0].mat[1];
		//t[3] = uv2.y * b[0].mat[2];// + b[0].mat[3];


		//f32 tex_area = fabsf((t[2] - t[0]) * (t[3] - t[1]));
		ieee754 max_sqr;
		//max_sqr.f = (t[2] - t[0]) * (t[3] - t[1]);
		//max_sqr.f = b[0].area * (uv2.x - uv.x) * (uv2.y - uv.y);
		//max_sqr.f = b[0].mat[0] * (uv2.x - uv.x);
		max_sqr.f = _maxss(fabsf(b[0].mat[0] * (uv2.x - uv.x)), fabsf(b[0].mat[2] * (uv2.y - uv.y)));

		//log2
		lodFactor = max_sqr.fields.exp - 127 - 0; // max_sqr.fields.exp ? max_sqr.fields.exp - 127 : 10000000; /*denormal very high number*/
#endif
	}
	else if (coord_count == 3) // if (tex.getType() == ETT_CUBE)
	{
		// cubeProject
		sVec4 d;
		d.x = coord.x * line.inversew[0];
		d.y = coord.y * line.inversew[0];
		d.z = coord.z * line.inversew[0];
		d.w = 0.f;
		d.normalize_dir_xyz();

		//dominant axis
		sVec4 a(fabsf(d.x), fabsf(d.y), fabsf(d.z));

		E_CUBE_SURFACE face;
		f32 inv;
		if (a.x >= a.y && a.x >= a.z) {
			inv = reciprocal_zero_pos_underflow(a.x);
			if (d.x > 0) { // +X
				face = ECS_POSX;
				uv.x = -d.z * inv;
				uv.y = -d.y * inv;
			}
			else {       // -X
				face = ECS_NEGX;
				uv.x = d.z * inv;
				uv.y = -d.y * inv;
			}
		}
		else if (a.y >= a.x && a.y >= a.z) {
			inv = reciprocal_zero_pos_underflow(a.y);
			if (d.y > 0) { // +Y
				face = ECS_POSY;
				uv.x = d.x * inv;
				uv.y = d.z * inv;
			}
			else {       // -Y
				face = ECS_NEGY;
				uv.x = d.x * inv;
				uv.y = -d.z * inv;
			}
		}
		else {
			inv = reciprocal_zero_pos_underflow(a.z);
			if (d.z > 0) { // +Z
				face = ECS_POSZ;
				uv.x = d.x * inv;
				uv.y = -d.y * inv;
			}
			else {       // -Z
				face = ECS_NEGZ;
				uv.x = -d.x * inv;
				uv.y = -d.y * inv;
			}
		}
		// currently face == mipmap
		lodFactor = face;

		// [-1,1] -> [0,1]
		uv.x = uv.x * 0.5f + 0.5f;
		uv.y = uv.y * 0.5f + 0.5f;
		BL_textureWrap(uv, texObj);
	}

	//clamp level
	lodFactor = core::s32_clamp(lodFactor, 0, SOFTWARE_DRIVER_2_MIPMAPPING_MAX - 1);
	while (lodFactor > 0 && b[lodFactor].data == 0) lodFactor -= 1;

	// sample point
	tFixPointu tx00 = tofix((uv.x * b[lodFactor].mat[0] /* +b[lodFactor].mat[1] */), FIX_POINT_F32_MUL);
	tFixPointu ty00 = tofix((uv.y * b[lodFactor].mat[2] /* +b[lodFactor].mat[3] */), FIX_POINT_F32_MUL);

	// nearest
	//tx00 += FIX_POINT_ZERO_DOT_FIVE;
	//ty00 += FIX_POINT_ZERO_DOT_FIVE;

	// fetch
	size_t ofs;
	ofs = (ty00 >> FIX_POINT_PRE) * b[lodFactor].pitch;
	ofs += (tx00 & FIX_POINT_UNSIGNED_MASK) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	// nearest pot
	//ofs = (((ty00 + FIX_POINT_ZERO_DOT_FIVE) & b[lodFactor].textureYMask) >> FIX_POINT_PRE) << b[lodFactor].pitchlog2;
	//ofs |= ((tx00 + FIX_POINT_ZERO_DOT_FIVE) & b[lodFactor].textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	// texel
	tTextureSample t00;
	t00 = *((tTextureSample*)(b[lodFactor].data + ofs));

	sVec4 out;
	out.setTextureSample(t00);
	return out;
}
#endif // if defined(burning_glsl_emu_test)


// get Sample bilinear range fixpoint 0-1.
void BL_Sampler_linear_Fix2(sVec4Fix& out, const int sampler, const tFixPoint coord[2], const SBurningShaderMaterial* MaterialLink)
{
	const sInternalTexture* tex = MaterialLink->shader_org->IT + sampler;

	//w00 w01 w10 w11
	tFixPointu w[4];
	{
		const tFixPointu fracx = coord[0] & FIX_POINT_FRACT_MASK;
		const tFixPointu fracy = coord[1] & FIX_POINT_FRACT_MASK;
		w[0] = imulFixu(FIX_POINT_ONE - fracx, FIX_POINT_ONE - fracy);
		w[1] = imulFixu(fracx, FIX_POINT_ONE - fracy);
		w[2] = imulFixu(FIX_POINT_ONE - fracx, fracy);
		w[3] = imulFixu(fracx, fracy);
	}

	//wraps positive (ignoring negative)
	tTextureSample t[4];
	{
		size_t o0, o1, o2, o3;

		// #if FIX_POINT_PRE > SOFTWARE_DRIVER_2_TEXTURE_MAXSIZE_LOG2 >> FIX_POINT_PRE - tex->pitchlog2

		o0 = (((coord[1]) & tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
		o1 = (((coord[1] + FIX_POINT_ONE) & tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
		o2 = ((coord[0]) & tex->textureXMask) >> (unsigned)(FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);
		o3 = ((coord[0] + FIX_POINT_ONE) & tex->textureXMask) >> (unsigned)(FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

		t[0] = *((tTextureSample*)((u8*)tex->data + (o0 + o2)));
		t[1] = *((tTextureSample*)((u8*)tex->data + (o0 + o3)));
		t[2] = *((tTextureSample*)((u8*)tex->data + (o1 + o2)));
		t[3] = *((tTextureSample*)((u8*)tex->data + (o1 + o3)));
	}

	out.a = ((((t[0] & MASK_A) >> SHIFT_A) * w[0]) +
		(((t[1] & MASK_A) >> SHIFT_A) * w[1]) +
		(((t[2] & MASK_A) >> SHIFT_A) * w[2]) +
		(((t[3] & MASK_A) >> SHIFT_A) * w[3]) + FIX_POINT_BI_COLOR_ERROR) >> COLOR_MAX_LOG2;

	out.r = ((((t[0] & MASK_R) >> SHIFT_R) * w[0]) +
		(((t[1] & MASK_R) >> SHIFT_R) * w[1]) +
		(((t[2] & MASK_R) >> SHIFT_R) * w[2]) +
		(((t[3] & MASK_R) >> SHIFT_R) * w[3]) + FIX_POINT_BI_COLOR_ERROR) >> COLOR_MAX_LOG2;

	out.g = ((((t[0] & MASK_G) >> SHIFT_G) * w[0]) +
		(((t[1] & MASK_G) >> SHIFT_G) * w[1]) +
		(((t[2] & MASK_G) >> SHIFT_G) * w[2]) +
		(((t[3] & MASK_G) >> SHIFT_G) * w[3]) + FIX_POINT_BI_COLOR_ERROR) >> COLOR_MAX_LOG2;

	out.b = (((t[0] & MASK_B) * w[0]) +
		((t[1] & MASK_B) * w[1]) +
		((t[2] & MASK_B) * w[2]) +
		((t[3] & MASK_B) * w[3]) + FIX_POINT_BI_COLOR_ERROR) >> COLOR_MAX_LOG2;
}


//! Constructor
IBurningShader::IBurningShader(
	CBurningVideoDriver* driver,
	s32& outMaterialTypeNr,
	const c8* vertexShaderProgram,
	const c8* vertexShaderEntryPointName,
	E_VERTEX_SHADER_TYPE vsCompileTarget,
	const c8* pixelShaderProgram,
	const c8* pixelShaderEntryPointName,
	E_PIXEL_SHADER_TYPE psCompileTarget,
	const c8* geometryShaderProgram,
	const c8* geometryShaderEntryPointName,
	E_GEOMETRY_SHADER_TYPE gsCompileTarget,
	scene::E_PRIMITIVE_TYPE inType,
	scene::E_PRIMITIVE_TYPE outType,
	u32 verticesOut,
	IShaderConstantSetCallBack* callback,
	E_MATERIAL_TYPE baseMaterial,
	s32 userData)
{
	constructor_IBurningShader(driver, baseMaterial);
	UserData = userData;
	CallBack = callback;
	if (CallBack)
		CallBack->grab();


	//v0.53 compile. only buildin

	//VertexShaderProgram = vertexShaderProgram;
	//PixelShaderProgram = pixelShaderProgram;
	BV_CompileShader(VertexShaderProgram, BV_VERTEX_SHADER, vertexShaderProgram);
	BV_CompileShader(FragmentShaderProgram, BV_FRAGMENT_SHADER, pixelShaderProgram);


	// register myself as new material
	outMaterialTypeNr = Driver->addMaterialRenderer(this);

}


//! destructor
IBurningShader::~IBurningShader()
{
	RenderTarget.drop();

	for (u32 i = 0; i != BURNING_MATERIAL_MAX_SAMPLER; ++i)
	{
		if (IT[i].Texture)
			IT[i].Texture->drop();
	}

	if (CallBack)
		CallBack->drop();

}

void sBurningRenderTarget::grab_color(video::IImage* surface)
{
	if (color)
		color->drop();

	color = (video::CImage*)surface;

	if (color)
	{
		color->grab();

		//(fp24*) DepthBuffer->lock() = DepthBuffer->lock();
	}
}

//! sets a render target
void IBurningShader::linkRenderTarget(const sBurningRenderTarget& driver_target, const core::rect<s32>& viewPort)
{
	irr_unreferenced_parameter(viewPort);

	RenderTarget.interlaced = driver_target.interlaced;
	RenderTarget.grab_color(driver_target.color);
}


//! sets the Texture
void IBurningShader::setTextureParam(glslEmu::sampler2D sampler, video::CSoftwareTexture2* texture, s32 lodFactor)
{
	sInternalTexture& it = IT[sampler];

	if (it.Texture) it.Texture->drop();
	it.Texture = texture;

	if (it.Texture)
	{
		it.Texture->grab();
		it.setTextureParam(lodFactor);
	}
}

//emulate a line with degenerate triangle and special shader mode (not perfect...)
void IBurningShader::drawLine(const s4DVertex* a, const s4DVertex* b)
{
	sVec2 d;
	d.x = b->Pos.x - a->Pos.x;	d.x *= d.x;
	d.y = b->Pos.y - a->Pos.y;	d.y *= d.y;
	//if ( d.x * d.y < 0.001f ) return;

	if (a->Pos.x > b->Pos.x) swapVertexPointer(a, b);

	s4DVertex c = *a;

	const f32 w = (f32)RenderTarget.color->getDimension().Width - 1;
	const f32 h = (f32)RenderTarget.color->getDimension().Height - 1;

	if (d.x < 2.f) { c.Pos.x = b->Pos.x + 1.f + d.y; if (c.Pos.x > w) c.Pos.x = w; }
	else c.Pos.x = b->Pos.x;
	if (d.y < 2.f) { c.Pos.y = b->Pos.y + 1.f; if (c.Pos.y > h) c.Pos.y = h; EdgeTestPass |= edge_test_first_line; }

	drawTriangle(a, b, &c);
	EdgeTestPass &= ~edge_test_first_line;

}

void IBurningShader::drawPoint(const s4DVertex* a)
{
}

void IBurningShader::drawWireFrameTriangle(s4DVertex* a, s4DVertex* b, s4DVertex* c)
{
	if (EdgeTestPass & edge_test_pass)
	{
		drawTriangle(a, b, c);
	}
	else if (EdgeTestPass & edge_test_point)
	{
		drawPoint(a);
		drawPoint(b);
		drawPoint(c);
	}
	else
	{
		drawLine(a, b);
		drawLine(b, c);
		drawLine(a, c);
	}
}


void IBurningShader::OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
	bool resetAllRenderstates, IMaterialRendererServices* services)
{
	if (Driver)
		Driver->setFallback_Material(BaseMaterial, VertexShaderProgram, FragmentShaderProgram);
	Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
	if (CallBack)
		CallBack->OnSetMaterial(material);

}

void IBurningShader::OnUnsetMaterial()
{
	//restore previous state
}

bool IBurningShader::OnRender(IMaterialRendererServices* service, E_VERTEX_TYPE vtxtype)
{
	// call callback to set shader constants
	if (CallBack)
		CallBack->OnSetConstants(this, UserData);
	return true;
}


//! Returns if the material is transparent.
bool IBurningShader::isTransparent() const
{
	return RenderPass_ShaderIsTransparent != 0;
}

//! Access the callback provided by the users when creating shader materials
IShaderConstantSetCallBack* IBurningShader::getShaderConstantSetCallBack() const
{
	return CallBack;
}

// implementations for the render services
void IBurningShader::setBasicRenderStates(const SMaterial& material, const SMaterial& lastMaterial, bool resetAllRenderstates)
{
	// forward
	Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
}

static BurningUniform _empty = { "null",BL_UNIFORM_FLOAT,
	{0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f} };
const f32* IBurningShader::getUniform(const c8* name, EBurningUniformFlags flags) const
{
	const size_t size = UniformInfo.size();
	if (size && name && name[0])
	{
		const BurningUniform* b = &UniformInfo[0];
		for (size_t i = 0; i < size; ++i)
		{
			if (tiny_istoken(b[i].name, name))
				return b[i].data;
		}
	}
	return _empty.data;
}

s32 IBurningShader::getShaderConstantID(EBurningUniformFlags flags, const c8* name)
{
	if (!name || !name[0])
		return -1;

	BurningUniform add;
	tiny_strcpy(add.name, name);
	add.type = flags;

	s32 index = UniformInfo.linear_search(add);
	if (index < 0)
	{
		for (size_t i = 0; i < array_size(add.data); ++i)
		{
			add.data[i] = 0.f;
		}
		UniformInfo.push_back(add);
		index = UniformInfo.size() - 1;
	}

	return index;
}

const char* tiny_itoa(s32 value, int base)
{
	static char b[32];
	int p = 31;

	//int sign = 0;
	//if (value < 0) { sign = 1; value = -value; }

	b[p] = '\0';
	do {
		b[--p] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[value % base];
		value /= base;
	} while (value && p > 0);

	//if (sign && p > 0) { b[--p] = '-'; }

	return b + p;
}

bool IBurningShader::setShaderConstantID(EBurningUniformFlags flags, s32 index, const void* data, size_t u32_count)
{
	if ((u32)index >= UniformInfo.size())
		return false;

	BurningUniform& use = UniformInfo[index];
	use.type = flags;

	const u32* s = (u32*)data;
	u32* d = (u32*)use.data;

	if (!s) u32_count = 0;
	if (u32_count > array_size(use.data)) u32_count = array_size(use.data);
	for (size_t i = 0; i < u32_count; ++i)
	{
		d[i] = s[i];
	}

	return true;
}


s32 IBurningShader::getVertexShaderConstantID(const c8* name)
{
	return getShaderConstantID(BL_VERTEX_PROGRAM, name);
}

s32 IBurningShader::getPixelShaderConstantID(const c8* name)
{
	return getShaderConstantID(BL_FRAGMENT_PROGRAM, name);
}

void IBurningShader::setVertexShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
	c8 name[BL_ACTIVE_UNIFORM_MAX_LENGTH];
	tiny_strcpy(name, tiny_itoa(startRegister, 10));

	setShaderConstantID(BL_VERTEX_FLOAT, getShaderConstantID(BL_VERTEX_PROGRAM, name), data, constantAmount * 4);
}

void IBurningShader::setPixelShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
{
	c8 name[BL_ACTIVE_UNIFORM_MAX_LENGTH];
	tiny_strcpy(name, tiny_itoa(startRegister, 10));

	setShaderConstantID(BL_FRAGMENT_FLOAT, getShaderConstantID(BL_FRAGMENT_PROGRAM, name), data, constantAmount * 4);
}

bool IBurningShader::setVertexShaderConstant(s32 index, const f32* floats, int count)
{
	return setShaderConstantID(BL_VERTEX_FLOAT, index, floats, count);
}

bool IBurningShader::setVertexShaderConstant(s32 index, const s32* ints, int count)
{
	return setShaderConstantID(BL_VERTEX_INT, index, ints, count);
}

bool IBurningShader::setVertexShaderConstant(s32 index, const u32* ints, int count)
{
	return setShaderConstantID(BL_VERTEX_UINT, index, ints, count);
}

bool IBurningShader::setPixelShaderConstant(s32 index, const f32* floats, int count)
{
	return setShaderConstantID(BL_FRAGMENT_FLOAT, index, floats, count);
}

bool IBurningShader::setPixelShaderConstant(s32 index, const s32* ints, int count)
{
	return setShaderConstantID(BL_FRAGMENT_INT, index, ints, count);
}

bool IBurningShader::setPixelShaderConstant(s32 index, const u32* ints, int count)
{
	return setShaderConstantID(BL_FRAGMENT_UINT, index, ints, count);
}

void IBurningShader::setStencilOp(eBurningStencilOp sfail, eBurningStencilOp dpfail, eBurningStencilOp dppass)
{
	stencilOp[0] = sfail;
	stencilOp[1] = dpfail;
	stencilOp[2] = dppass;
}


// glsemu
void glslEmu::OnSetMaterial_BL(const SBurningShaderMaterial& material)
{
	MaterialLink = &material;
#if defined(burning_glsl_emu_test)
	glsl.MaterialLink = &material;

	if (material.FragmentShader.id == BFT_IRR_0342_0x11b0394b) fragmentShader = &glslEmu::frag_BFT_IRR_0342_0x11b0394b;
#if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader)
	else if (material.FragmentShader.id == BFT_STK_1612_0x1e372102) fragmentShader = &glslEmu::frag_BFT_STK_1612_0x1e372102;
#endif
	else fragmentShader = &glslEmu::frag_default;
#endif

}

#if defined(burning_glsl_emu_test)

#define burning_shader_emu
#include "burning_shader_glsl_emu_define.h"

/*
//cubeMapReflection.frag
#version 130
uniform samplerCube cubeTex;
uniform float Roughness;

void main( void )
{
//	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	vec3 uvw = vec3(gl_TexCoord[0]);
	//gl_FragColor = textureCube( cubeTex, uvw );
	gl_FragColor = textureLod( cubeTex, uvw, Roughness );
	//gl_FragColor = textureCube( cubeTex, uvw, Roughness );
}

*/
void glslEmu::frag_BFT_IRR_0342_0x11b0394b()
{
	uniform(samplerCube, cubeTex);
	uniform(float, Roughness);

	vec3 uvw = vec3(gl_TexCoord[0]);

	gl_FragColor = textureCube(cubeTex, uvw);
}

/*
//splatting2.frag

uniform sampler2D tex_layout;
uniform sampler2D tex_detail0;
uniform sampler2D tex_detail1;
uniform sampler2D tex_detail2;
uniform sampler2D tex_detail3;
uniform sampler2D tex_detail4;

void main()
{
	vec4 splatting = texture2D(tex_layout, gl_TexCoord[1].st);
	vec4 detail0 = texture2D(tex_detail0, gl_TexCoord[0].st);
	vec4 detail1 = texture2D(tex_detail1, gl_TexCoord[0].st);
	vec4 detail2 = texture2D(tex_detail2, gl_TexCoord[0].st);
	vec4 detail3 = texture2D(tex_detail3, gl_TexCoord[0].st);
	vec4 detail4 = texture2D(tex_detail4, gl_TexCoord[0].st);

	gl_FragColor = (splatting.r * detail0 +
									splatting.g * detail1 +
									splatting.b * detail2 +
									(1.0 - splatting.r - splatting.g - splatting.b) * detail3 +
									(1.0 - splatting.a) * detail4) * gl_Color;
}

}
*/
void glslEmu::frag_BFT_STK_1612_0x1e372102()
{
	uniform(sampler2D, tex_layout);
	uniform(sampler2D, tex_detail0);
	uniform(sampler2D, tex_detail1);
	uniform(sampler2D, tex_detail2);
	uniform(sampler2D, tex_detail3);
	uniform(sampler2D, tex_detail4);

	//void main()
	{
		vec4 splatting = texture2D(tex_layout, gl_TexCoord[1]);
		vec4 detail0 = texture2D(tex_detail0, gl_TexCoord[0]);
		vec4 detail1 = texture2D(tex_detail1, gl_TexCoord[0]);
		vec4 detail2 = texture2D(tex_detail2, gl_TexCoord[0]);
		vec4 detail3 = texture2D(tex_detail3, gl_TexCoord[0]);
		vec4 detail4 = texture2D(tex_detail4, gl_TexCoord[0]);

		gl_FragColor = (detail0 * splatting.r +
			detail1 * splatting.g +
			detail2 * splatting.b + 
			detail3 * (1.0f - splatting.r - splatting.g - splatting.b) +
			detail4 * (1.0f - splatting.a)) * gl_Color
			;
	}
}
#include "burning_shader_glsl_emu_undefine.h"


void glslEmu::call_fragmentShader()
{
	const sScanLineData& line = MaterialLink->shader_org->line;
	//const sScanConvertData& scan = MaterialLink->shader_org->scan;

	// depth test
	int depth_failed = 0;
	switch (MaterialLink->org.ZBuffer)
	{
	case ECFN_LESSEQUAL:
		depth_failed = line.w[0] < line.depth[line.xRun];
		break;
	default:
		break;
	}
	if (depth_failed)
		return;

	if (MaterialLink->depth_write) line.depth[line.xRun] = line.w[0];

	//void set_varying_in(const sScanLineData & line, const sScanConvertData & scan)

	// Current Fixed Function: Texture Coo are premultiplied with single Mipmap (PixelCoo). For Shader we need normalized
	// Current BL_Sampler: 
	//   - does perspective correction (* line.inversew[0]) 
	//   - wrapping
	//   - per pixel mipmap selection (needs to see the raw coord for dFdx,dFdy)

	gl_FragCoord.w = line.inversew[0];

	/* * line.inversew[0] to see glsl value */
	if (MaterialLink->FragmentShader.id == BFT_IRR_0342_0x11b0394b)
	{
		gl_TexCoord[0] = line.c[0][0];
	}
	else
	{
		gl_TexCoord[0] = line.t[0][0];
		gl_TexCoord[1] = line.t[1][0];
	}

	gl_Color = line.c[0][0] * line.inversew[0];

	varying_count = 0;
	glsl.varying_count = 0;

	_discard = 0;
	//call shader
	(this->*fragmentShader)();
	if (_discard) return;

	gl_FragColor.clampf01();

	sVec4 dst;

	//glBlendFunc
	switch (MaterialLink->Fallback_MaterialType) // shader->BaseMaterial
	{
	case EMT_TRANSPARENT_ADD_COLOR:
	case EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR:
	case EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR:
		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
		dst.setTextureSample(line.dst[line.xRun]);
		gl_FragColor.r += dst.r * (1.f - gl_FragColor.r);
		gl_FragColor.g += dst.g * (1.f - gl_FragColor.g);
		gl_FragColor.b += dst.b * (1.f - gl_FragColor.b);
		break;
	default:
		break;
	}

	//write 
	line.dst[line.xRun] = gl_FragColor.getRenderTargetSample();
}


void IBurningShader::glslEmu_drawScanLine()
{
	if (line.set(scan))
		return;

	SOFTWARE_DRIVER_2_CLIPCHECK;
	size_t ofs = (line.y * RenderTarget.color->getDimension().Width);
	line.dst = (tRenderTargetColorSample*)RenderTarget.color->getData() + ofs;
	line.depth = (fp24*)RenderTarget.depth->getData() + ofs;

	for (line.xRun = line.xStart; line.xRun <= line.xEnd; line.xRun += SOFTWARE_DRIVER_2_STEP_X)
	{
		line.inversew[0] = reciprocal_zero_no(line.w[0]);
		line.inversew[1] = reciprocal_zero_no((line.w[0] + line.w[1])); // dFdx
		glsl.call_fragmentShader();
		line.ipol_step(scan);

	}

}

void IBurningShader::glslEmu_drawFace(s4DVertexPair* face[4])
{
	if (scan.set_face(face, glsl.MaterialLink->CullFlag))
		return;

	// calculate slopes for the major edge
	scan.set_ipol(0, 0, 2, 0);

	// rasterize upper sub-triangle
	if (F32_GREATER_0(scan.invDeltaY[1]))
	{
		// calculate slopes for top edge
		scan.set_ipol(1, 0, 1, 1);
		scan.set_y(0, 1);

		// rasterize the edge scanlines
		for (line.y = scan.yStart; line.y <= scan.yEnd; line.y += SOFTWARE_DRIVER_2_STEP_Y)
		{
			// render a scanline
			if_interlace_scanline glslEmu_drawScanLine();
			scan.ipol_step();
		}

	}

	// rasterize lower sub-triangle
	if (F32_GREATER_0(scan.invDeltaY[2]))
	{
		// advance to middle point
		if (F32_GREATER_0(scan.invDeltaY[1])) scan.advance_midpoint();

		// calculate slopes for bottom edge
		scan.set_ipol(1, 1, 2, 2);
		scan.set_y(1, 2);

		// rasterize the edge scanlines
		for (line.y = scan.yStart; line.y <= scan.yEnd; line.y += SOFTWARE_DRIVER_2_STEP_Y)
		{
			// render a scanline
			if_interlace_scanline glslEmu_drawScanLine();
			scan.ipol_step();
		}
	}
}

#endif // defined(burning_glsl_emu)

void PushShaderData::push(IBurningShader* shader)
{
	CurrentShader = shader;
	if (shader) shader->pushShader(this, 1);
}
void PushShaderData::pop()
{
	if (CurrentShader) CurrentShader->pushShader(this, 0);
}

void IBurningShader::pushShader(PushShaderData* data, int save)
{
	if (save)
	{
		data->EdgeTestPass = EdgeTestPass;
	}
	else
	{
		EdgeTestPass = data->EdgeTestPass;
	}
}

IVideoDriver* IBurningShader::getVideoDriver()
{
	return Driver;
}


burning_namespace_end

#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_
