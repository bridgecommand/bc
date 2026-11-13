// Copyright (C) 2002-2022 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_
#include "IBurningShader.h"

burning_namespace_start
#include "burning_shader_compile_start.h"

// define render case
#define SUBTEXEL
#define INVERSE_W

#define USE_ZBUFFER
#define IPOL_W
#define CMP_W
#define WRITE_W

#define IPOL_C0
#define IPOL_T0
#define IPOL_T1

#include "burning_shader_compile_verify.h"

class CTRGTextureLightMap2_M4 : public IBurningShader
{
public:

	//! constructor
	CTRGTextureLightMap2_M4(CBurningVideoDriver* driver);

	//! draws an indexed triangle list
	virtual void drawTriangle(const s4DVertex* burning_restrict a, const s4DVertex* burning_restrict b, const s4DVertex* burning_restrict c) IRR_OVERRIDE;
	virtual void OnSetMaterial_BL(const SBurningShaderMaterial& material) IRR_OVERRIDE;
	virtual void OnRender_BL(IMaterialRendererServices* service) IRR_OVERRIDE;

private:

	// fragment shader
	typedef void (CTRGTextureLightMap2_M4::* tFragmentShader) ();
	void fragment_linear();

	tFragmentShader fragmentShader;


#if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader)
	// Fallback Material is CTRTextureGouraud2 but we need two Textures
	void frag_BFT_STK_1612_0x1e372102();
	glslEmu::sampler2D tex_layout;
	glslEmu::sampler2D tex_detail0;
	glslEmu::sampler2D tex_detail1;
	glslEmu::sampler2D tex_detail2;
	glslEmu::sampler2D tex_detail3;
	glslEmu::sampler2D tex_detail4;
#endif

};

//! constructor
CTRGTextureLightMap2_M4::CTRGTextureLightMap2_M4(CBurningVideoDriver* driver)
	: IBurningShader(driver, EMT_LIGHTMAP_LIGHTING_M4)
{
#ifdef _DEBUG
	setDebugName("CTRGTextureLightMap2_M4");
#endif

	fragmentShader = &CTRGTextureLightMap2_M4::fragment_linear;

#if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader)
	//frag_BFT_STK_1612_0x1e372102
	tex_layout = 0;
	tex_detail0 = 0;
	tex_detail1 = 0;
	tex_detail2 = 0;
	tex_detail3 = 0;
	tex_detail4 = 0;
#endif
}

/*!
*/
void CTRGTextureLightMap2_M4::OnSetMaterial_BL(const SBurningShaderMaterial& material)
{
#if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader)
	if (material.FragmentShader.id == BFT_STK_1612_0x1e372102) fragmentShader = &CTRGTextureLightMap2_M4::frag_BFT_STK_1612_0x1e372102;
	else
#endif
	fragmentShader = &CTRGTextureLightMap2_M4::fragment_linear;
}

void CTRGTextureLightMap2_M4::OnRender_BL(IMaterialRendererServices* service)
{
#if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader)
	if (glsl.MaterialLink->FragmentShader.id == BFT_STK_1612_0x1e372102)
	{
		tex_layout = *bl_uniform_p(glslEmu::sampler2D, "tex_layout");
		tex_detail0 = *bl_uniform_p(glslEmu::sampler2D, "tex_detail0");
		tex_detail1 = *bl_uniform_p(glslEmu::sampler2D, "tex_detail1");
		tex_detail2 = *bl_uniform_p(glslEmu::sampler2D, "tex_detail2");
		tex_detail3 = *bl_uniform_p(glslEmu::sampler2D, "tex_detail3");
		tex_detail4 = *bl_uniform_p(glslEmu::sampler2D, "tex_detail4");
	}
#endif
}


/*!
*/
void CTRGTextureLightMap2_M4::fragment_linear()
{
	tRenderTargetColorSample* dst;

#ifdef USE_ZBUFFER
	fp24* z;
#endif

	s32 xStart;
	s32 xEnd;
	s32 dx;


#ifdef SUBTEXEL
	f32 subPixel;
#endif

#ifdef IPOL_Z
	f32 slopeZ;
#endif
#ifdef IPOL_W
	fp24 slopeW;
#endif
#ifdef IPOL_C0
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS_USED];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES_USED];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left(line.x[0]);
	xEnd = fill_convention_right(line.x[1]);

	dx = xEnd - xStart;

	if (dx < 0)
		return;

	// slopes
	const f32 invDeltaX = fill_step_x(line.x[1] - line.x[0]);

#ifdef IPOL_Z
	slopeZ = (line.z[1] - line.z[0]) * invDeltaX;
#endif
#ifdef IPOL_W
	slopeW = (line.w[1] - line.w[0]) * invDeltaX;
#endif
#ifdef IPOL_C0
	slopeC[0] = (line.c[0][1] - line.c[0][0]) * invDeltaX;
#endif
#ifdef IPOL_T0
	slopeT[0] = (line.t[0][1] - line.t[0][0]) * invDeltaX;
#endif
#ifdef IPOL_T1
	slopeT[1] = (line.t[1][1] - line.t[1][0]) * invDeltaX;
#endif

#ifdef SUBTEXEL
	subPixel = ((f32)xStart) - line.x[0];
#ifdef IPOL_Z
	line.z[0] += slopeZ * subPixel;
#endif
#ifdef IPOL_W
	line.w[0] += slopeW * subPixel;
#endif
#ifdef IPOL_C0
	line.c[0][0] += slopeC[0] * subPixel;
#endif
#ifdef IPOL_T0
	line.t[0][0] += slopeT[0] * subPixel;
#endif
#ifdef IPOL_T1
	line.t[1][0] += slopeT[1] * subPixel;
#endif
#endif

	SOFTWARE_DRIVER_2_CLIPCHECK;
	dst = (tRenderTargetColorSample*)RenderTarget.color->getData() + (line.y * RenderTarget.color->getDimension().Width) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*)RenderTarget.depth->getData() + (line.y * RenderTarget.color->getDimension().Width) + xStart;
#endif


	f32 inversew = FIX_POINT_F32_MUL;

	tFixPoint tx0, tx1;
	tFixPoint ty0, ty1;

	tFixPoint r0, g0, b0;
	tFixPoint r1, g1, b1;
	tFixPoint r2, g2, b2;

#ifdef IPOL_C0
	tFixPoint r3, g3, b3;
#endif

	for (s32 i = 0; i <= dx; i += SOFTWARE_DRIVER_2_STEP_X)
	{
#ifdef CMP_Z
		if (line.z[0] < z[i])
#endif
#ifdef CMP_W
			if (line.w[0] >= z[i])
#endif
			{
#ifdef INVERSE_W
				inversew = fix_inverse32(line.w[0]);
#endif
				tx0 = tofix(line.t[0][0].x, inversew);
				ty0 = tofix(line.t[0][0].y, inversew);
				tx1 = tofix(line.t[1][0].x, inversew);
				ty1 = tofix(line.t[1][0].y, inversew);

#ifdef IPOL_C0
				r3 = tofix(line.c[0][0].y, inversew);
				g3 = tofix(line.c[0][0].z, inversew);
				b3 = tofix(line.c[0][0].w, inversew);
#endif

				getSample_texture(r0, g0, b0, &IT[0], tx0, ty0);
				getSample_texture(r1, g1, b1, &IT[1], tx1, ty1);

#ifdef IPOL_C0
				r2 = imulFix_simple(r0, r3);
				g2 = imulFix_simple(g0, g3);
				b2 = imulFix_simple(b0, b3);

				r2 = imulFix_tex4(r2, r1);
				g2 = imulFix_tex4(g2, g1);
				b2 = imulFix_tex4(b2, b1);

#else
				r2 = imulFix_tex4(r0, r1);
				g2 = imulFix_tex4(g0, g1);
				b2 = imulFix_tex4(b0, b1);
#endif


				dst[i] = fix_to_sample(r2, g2, b2);

#ifdef WRITE_Z
				z[i] = line.z[0];
#endif
#ifdef WRITE_W
				z[i] = line.w[0];
#endif
			}

#ifdef IPOL_Z
		line.z[0] += slopeZ;
#endif
#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_T1
		line.t[1][0] += slopeT[1];
#endif
	}

}


void CTRGTextureLightMap2_M4::drawTriangle(const s4DVertex* burning_restrict a, const s4DVertex* burning_restrict b, const s4DVertex* burning_restrict c)
{
	// sort on height, y
	if (F32_A_GREATER_B(a->Pos.y, b->Pos.y)) swapVertexPointer(a, b);
	if (F32_A_GREATER_B(b->Pos.y, c->Pos.y)) swapVertexPointer(b, c);
	if (F32_A_GREATER_B(a->Pos.y, b->Pos.y)) swapVertexPointer(a, b);

	const f32 ca = c->Pos.y - a->Pos.y;
	const f32 ba = b->Pos.y - a->Pos.y;
	const f32 cb = c->Pos.y - b->Pos.y;
	// calculate delta y of the edges
	scan.invDeltaY[0] = fill_step_y(ca);
	scan.invDeltaY[1] = fill_step_y(ba);
	scan.invDeltaY[2] = fill_step_y(cb);

	if (F32_LOWER_0(scan.invDeltaY[0]))
		return;

	// find if the major edge is left or right aligned
	f32 temp[4];

	temp[0] = a->Pos.x - c->Pos.x;
	temp[1] = -ca;
	temp[2] = b->Pos.x - a->Pos.x;
	temp[3] = ba;

	scan.left = (temp[0] * temp[3] - temp[1] * temp[2]) < 0.f ? 1 : 0;
	scan.right = 1 - scan.left;

	// calculate slopes for the major edge
	scan.slopeX[0] = (c->Pos.x - a->Pos.x) * scan.invDeltaY[0];
	scan.x[0] = a->Pos.x;

#ifdef IPOL_Z
	scan.slopeZ[0] = (c->Pos.z - a->Pos.z) * scan.invDeltaY[0];
	scan.z[0] = a->Pos.z;
#endif

#ifdef IPOL_W
	scan.slopeW[0] = (c->Pos.w - a->Pos.w) * scan.invDeltaY[0];
	scan.w[0] = a->Pos.w;
#endif

#ifdef IPOL_C0
	scan.slopeC[0][0] = (c->Color[0] - a->Color[0]) * scan.invDeltaY[0];
	scan.c[0][0] = a->Color[0];
#endif

#ifdef IPOL_T0
	scan.slopeT[0][0] = (c->Tex[0] - a->Tex[0]) * scan.invDeltaY[0];
	scan.t[0][0] = a->Tex[0];
#endif

#ifdef IPOL_T1
	scan.slopeT[1][0] = (c->Tex[1] - a->Tex[1]) * scan.invDeltaY[0];
	scan.t[1][0] = a->Tex[1];
#endif

	// top left fill convention y run
	s32 yStart;
	s32 yEnd;

#ifdef SUBTEXEL
	f32 subPixel;
#endif


	// rasterize upper sub-triangle
	//if ( (f32) 0.0 != scan.invDeltaY[1]  )
	if (F32_GREATER_0(scan.invDeltaY[1]))
	{
		// calculate slopes for top edge
		scan.slopeX[1] = (b->Pos.x - a->Pos.x) * scan.invDeltaY[1];
		scan.x[1] = a->Pos.x;

#ifdef IPOL_Z
		scan.slopeZ[1] = (b->Pos.z - a->Pos.z) * scan.invDeltaY[1];
		scan.z[1] = a->Pos.z;
#endif

#ifdef IPOL_W
		scan.slopeW[1] = (b->Pos.w - a->Pos.w) * scan.invDeltaY[1];
		scan.w[1] = a->Pos.w;
#endif

#ifdef IPOL_C0
		scan.slopeC[0][1] = (b->Color[0] - a->Color[0]) * scan.invDeltaY[1];
		scan.c[0][1] = a->Color[0];
#endif

#ifdef IPOL_T0
		scan.slopeT[0][1] = (b->Tex[0] - a->Tex[0]) * scan.invDeltaY[1];
		scan.t[0][1] = a->Tex[0];
#endif

#ifdef IPOL_T1
		scan.slopeT[1][1] = (b->Tex[1] - a->Tex[1]) * scan.invDeltaY[1];
		scan.t[1][1] = a->Tex[1];
#endif

		// apply top-left fill convention, top part
		yStart = fill_convention_top(a->Pos.y);
		yEnd = fill_convention_down(b->Pos.y);

#ifdef SUBTEXEL
		subPixel = ((f32)yStart) - a->Pos.y;

		// correct to pixel center
		scan.x[0] += scan.slopeX[0] * subPixel;
		scan.x[1] += scan.slopeX[1] * subPixel;

#ifdef IPOL_Z
		scan.z[0] += scan.slopeZ[0] * subPixel;
		scan.z[1] += scan.slopeZ[1] * subPixel;
#endif

#ifdef IPOL_W
		scan.w[0] += scan.slopeW[0] * subPixel;
		scan.w[1] += scan.slopeW[1] * subPixel;
#endif

#ifdef IPOL_C0
		scan.c[0][0] += scan.slopeC[0][0] * subPixel;
		scan.c[0][1] += scan.slopeC[0][1] * subPixel;
#endif

#ifdef IPOL_T0
		scan.t[0][0] += scan.slopeT[0][0] * subPixel;
		scan.t[0][1] += scan.slopeT[0][1] * subPixel;
#endif

#ifdef IPOL_T1
		scan.t[1][0] += scan.slopeT[1][0] * subPixel;
		scan.t[1][1] += scan.slopeT[1][1] * subPixel;
#endif

#endif

		// rasterize the edge scanlines
		for (line.y = yStart; line.y <= yEnd; line.y += SOFTWARE_DRIVER_2_STEP_Y)
		{
			line.x[scan.left] = scan.x[0];
			line.x[scan.right] = scan.x[1];

#ifdef IPOL_Z
			line.z[scan.left] = scan.z[0];
			line.z[scan.right] = scan.z[1];
#endif

#ifdef IPOL_W
			line.w[scan.left] = scan.w[0];
			line.w[scan.right] = scan.w[1];
#endif

#ifdef IPOL_C0
			line.c[0][scan.left] = scan.c[0][0];
			line.c[0][scan.right] = scan.c[0][1];
#endif

#ifdef IPOL_T0
			line.t[0][scan.left] = scan.t[0][0];
			line.t[0][scan.right] = scan.t[0][1];
#endif

#ifdef IPOL_T1
			line.t[1][scan.left] = scan.t[1][0];
			line.t[1][scan.right] = scan.t[1][1];
#endif

			// render a scanline
			if_interlace_scanline
			(this->*fragmentShader) ();

			scan.x[0] += scan.slopeX[0];
			scan.x[1] += scan.slopeX[1];

#ifdef IPOL_Z
			scan.z[0] += scan.slopeZ[0];
			scan.z[1] += scan.slopeZ[1];
#endif

#ifdef IPOL_W
			scan.w[0] += scan.slopeW[0];
			scan.w[1] += scan.slopeW[1];
#endif

#ifdef IPOL_C0
			scan.c[0][0] += scan.slopeC[0][0];
			scan.c[0][1] += scan.slopeC[0][1];
#endif

#ifdef IPOL_T0
			scan.t[0][0] += scan.slopeT[0][0];
			scan.t[0][1] += scan.slopeT[0][1];
#endif

#ifdef IPOL_T1
			scan.t[1][0] += scan.slopeT[1][0];
			scan.t[1][1] += scan.slopeT[1][1];
#endif

		}
	}

	// rasterize lower sub-triangle
	//if ( (f32) 0.0 != scan.invDeltaY[2] )
	if (F32_GREATER_0(scan.invDeltaY[2]))
	{
		// advance to middle point
		//if( (f32) 0.0 != scan.invDeltaY[1] )
		if (F32_GREATER_0(scan.invDeltaY[1]))
		{
			temp[0] = b->Pos.y - a->Pos.y;	// dy

			scan.x[0] = a->Pos.x + scan.slopeX[0] * temp[0];
#ifdef IPOL_Z
			scan.z[0] = a->Pos.z + scan.slopeZ[0] * temp[0];
#endif
#ifdef IPOL_W
			scan.w[0] = a->Pos.w + scan.slopeW[0] * temp[0];
#endif
#ifdef IPOL_C0
			scan.c[0][0] = a->Color[0] + scan.slopeC[0][0] * temp[0];
#endif
#ifdef IPOL_T0
			scan.t[0][0] = a->Tex[0] + scan.slopeT[0][0] * temp[0];
#endif
#ifdef IPOL_T1
			scan.t[1][0] = a->Tex[1] + scan.slopeT[1][0] * temp[0];
#endif

		}

		// calculate slopes for bottom edge
		scan.slopeX[1] = (c->Pos.x - b->Pos.x) * scan.invDeltaY[2];
		scan.x[1] = b->Pos.x;

#ifdef IPOL_Z
		scan.slopeZ[1] = (c->Pos.z - b->Pos.z) * scan.invDeltaY[2];
		scan.z[1] = b->Pos.z;
#endif

#ifdef IPOL_W
		scan.slopeW[1] = (c->Pos.w - b->Pos.w) * scan.invDeltaY[2];
		scan.w[1] = b->Pos.w;
#endif

#ifdef IPOL_C0
		scan.slopeC[0][1] = (c->Color[0] - b->Color[0]) * scan.invDeltaY[2];
		scan.c[0][1] = b->Color[0];
#endif

#ifdef IPOL_T0
		scan.slopeT[0][1] = (c->Tex[0] - b->Tex[0]) * scan.invDeltaY[2];
		scan.t[0][1] = b->Tex[0];
#endif

#ifdef IPOL_T1
		scan.slopeT[1][1] = (c->Tex[1] - b->Tex[1]) * scan.invDeltaY[2];
		scan.t[1][1] = b->Tex[1];
#endif

		// apply top-left fill convention, top part
		yStart = fill_convention_top(b->Pos.y);
		yEnd = fill_convention_down(c->Pos.y);

#ifdef SUBTEXEL

		subPixel = ((f32)yStart) - b->Pos.y;

		// correct to pixel center
		scan.x[0] += scan.slopeX[0] * subPixel;
		scan.x[1] += scan.slopeX[1] * subPixel;

#ifdef IPOL_Z
		scan.z[0] += scan.slopeZ[0] * subPixel;
		scan.z[1] += scan.slopeZ[1] * subPixel;
#endif

#ifdef IPOL_W
		scan.w[0] += scan.slopeW[0] * subPixel;
		scan.w[1] += scan.slopeW[1] * subPixel;
#endif

#ifdef IPOL_C0
		scan.c[0][0] += scan.slopeC[0][0] * subPixel;
		scan.c[0][1] += scan.slopeC[0][1] * subPixel;
#endif

#ifdef IPOL_T0
		scan.t[0][0] += scan.slopeT[0][0] * subPixel;
		scan.t[0][1] += scan.slopeT[0][1] * subPixel;
#endif

#ifdef IPOL_T1
		scan.t[1][0] += scan.slopeT[1][0] * subPixel;
		scan.t[1][1] += scan.slopeT[1][1] * subPixel;
#endif

#endif

		// rasterize the edge scanlines
		for (line.y = yStart; line.y <= yEnd; line.y += SOFTWARE_DRIVER_2_STEP_Y)
		{
			line.x[scan.left] = scan.x[0];
			line.x[scan.right] = scan.x[1];

#ifdef IPOL_Z
			line.z[scan.left] = scan.z[0];
			line.z[scan.right] = scan.z[1];
#endif

#ifdef IPOL_W
			line.w[scan.left] = scan.w[0];
			line.w[scan.right] = scan.w[1];
#endif

#ifdef IPOL_C0
			line.c[0][scan.left] = scan.c[0][0];
			line.c[0][scan.right] = scan.c[0][1];
#endif

#ifdef IPOL_T0
			line.t[0][scan.left] = scan.t[0][0];
			line.t[0][scan.right] = scan.t[0][1];
#endif

#ifdef IPOL_T1
			line.t[1][scan.left] = scan.t[1][0];
			line.t[1][scan.right] = scan.t[1][1];
#endif

			// render a scanline
			if_interlace_scanline
			(this->*fragmentShader) ();

			scan.x[0] += scan.slopeX[0];
			scan.x[1] += scan.slopeX[1];

#ifdef IPOL_Z
			scan.z[0] += scan.slopeZ[0];
			scan.z[1] += scan.slopeZ[1];
#endif

#ifdef IPOL_W
			scan.w[0] += scan.slopeW[0];
			scan.w[1] += scan.slopeW[1];
#endif

#ifdef IPOL_C0
			scan.c[0][0] += scan.slopeC[0][0];
			scan.c[0][1] += scan.slopeC[0][1];
#endif

#ifdef IPOL_T0
			scan.t[0][0] += scan.slopeT[0][0];
			scan.t[0][1] += scan.slopeT[0][1];
#endif

#ifdef IPOL_T1
			scan.t[1][0] += scan.slopeT[1][0];
			scan.t[1][1] += scan.slopeT[1][1];
#endif

		}
	}

}

#if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader)
/*!
// splatting2.frag

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

	gl_FragColor = (
		splatting.r * detail0 +
		splatting.g * detail1 +
		splatting.b * detail2 +
		(1.0 - splatting.r - splatting.g - splatting.b) * detail3 +
		(1.0 - splatting.a) * detail4
	) * gl_Color;
}
*/

#define IPOL_C0
#define IPOL_T0
#define IPOL_T1
#include "burning_shader_compile_verify.h"

void CTRGTextureLightMap2_M4::frag_BFT_STK_1612_0x1e372102()
{
	tRenderTargetColorSample* dst;

#ifdef USE_ZBUFFER
	fp24* z;
#endif

	s32 xStart;
	s32 xEnd;
	s32 dx;


#ifdef SUBTEXEL
	f32 subPixel;
#endif

#ifdef IPOL_Z
	f32 slopeZ;
#endif
#ifdef IPOL_W
	fp24 slopeW;
#endif
#ifdef IPOL_C0
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS_USED];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES_USED];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left(line.x[0]);
	xEnd = fill_convention_right(line.x[1]);

	dx = xEnd - xStart;
	if (dx < 0)
		return;

	// slopes
	const f32 invDeltaX = fill_step_x(line.x[1] - line.x[0]);

#ifdef IPOL_Z
	slopeZ = (line.z[1] - line.z[0]) * invDeltaX;
#endif
#ifdef IPOL_W
	slopeW = (line.w[1] - line.w[0]) * invDeltaX;
#endif
#ifdef IPOL_C0
	slopeC[0] = (line.c[0][1] - line.c[0][0]) * invDeltaX;
#endif
#ifdef IPOL_T0
	slopeT[0] = (line.t[0][1] - line.t[0][0]) * invDeltaX;
#endif
#ifdef IPOL_T1
	slopeT[1] = (line.t[1][1] - line.t[1][0]) * invDeltaX;
#endif

#ifdef SUBTEXEL
	subPixel = ((f32)xStart) - line.x[0];
#ifdef IPOL_Z
	line.z[0] += slopeZ * subPixel;
#endif
#ifdef IPOL_W
	line.w[0] += slopeW * subPixel;
#endif
#ifdef IPOL_C0
	line.c[0][0] += slopeC[0] * subPixel;
#endif
#ifdef IPOL_T0
	line.t[0][0] += slopeT[0] * subPixel;
#endif
#ifdef IPOL_T1
	line.t[1][0] += slopeT[1] * subPixel;
#endif
#endif

	SOFTWARE_DRIVER_2_CLIPCHECK;
	dst = (tRenderTargetColorSample*)RenderTarget.color->getData() + (line.y * RenderTarget.color->getDimension().Width) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*)RenderTarget.depth->getData() + (line.y * RenderTarget.color->getDimension().Width) + xStart;
#endif

#define burning_shader_fragment_fix
#include "burning_shader_glsl_emu_define.h"

	/*
		//0 do not see explicit set. but is mostly correct for single texture
		const sampler2D tex_layout = 1;		//splat.png
		const sampler2D tex_detail0 = 2;	//dirt.jpg, m_splatting_texture_1
		const sampler2D tex_detail1 = 3;	// grass2.jpg,m_splatting_texture_2
		const sampler2D tex_detail2 = 4;	// snow.png,m_splatting_texture_3
		const sampler2D tex_detail3 = 5;	// rock_grey.jpg m_splatting_texture_4
		const sampler2D tex_detail4 = 0;	// unset default to 0
	*/
	f32 inversew = FIX_POINT_F32_MUL;

	sVec4Fix splatting;
	sVec4Fix tex;
	sVec4Fix frag;

	for (s32 _sx = 0; _sx <= dx; _sx += SOFTWARE_DRIVER_2_STEP_X)
	{

#ifdef CMP_Z
		if (line.z[0] > z[_sx]) discard;
#endif
#ifdef CMP_W
		if (line.w[0] < z[_sx]) discard;
#endif
		{
#ifdef WRITE_Z
			z[_sx] = line.z[0];
#endif
#ifdef WRITE_W
			z[_sx] = line.w[0];
#endif

#ifdef INVERSE_W
			inversew = fix_inverse32(line.w[0]);
#endif
			gl_TexCoord[0] = tofix(line.t[0][0].x, inversew);
			gl_TexCoord[1] = tofix(line.t[0][0].y, inversew);
			gl_TexCoord[2] = tofix(line.t[1][0].x, inversew);
			gl_TexCoord[3] = tofix(line.t[1][0].y, inversew);

			gl_Color.r = tofix(line.c[0][0].r, inversew);
			gl_Color.g = tofix(line.c[0][0].g, inversew);
			gl_Color.b = tofix(line.c[0][0].b, inversew);

			//vec4 splatting = texture2D(tex_layout, gl_TexCoord[1].st);
			getSample_texture(splatting.a, splatting.r, splatting.g, splatting.b,
				IT + tex_layout, gl_TexCoord[2], gl_TexCoord[3]);

			//gl_FragColor = (
			frag.r = 0; frag.g = 0; frag.b = 0;

			// splatting.r * detail0
			if (splatting.r)
			{
				getSample_texture(tex.r, tex.g, tex.b, IT + tex_detail0, gl_TexCoord[0], gl_TexCoord[1]);
				frag.add_mul_rgb(tex, splatting.r);
			}

			// + splatting.g * detail1
			if (splatting.g)
			{
				getSample_texture(tex.r, tex.g, tex.b, IT + tex_detail1, gl_TexCoord[0], gl_TexCoord[1]);
				frag.add_mul_rgb(tex, splatting.g);
			}

			// + splatting.b * detail2
			if (splatting.b)
			{
				getSample_texture(tex.r, tex.g, tex.b, IT + tex_detail2, gl_TexCoord[0], gl_TexCoord[1]);
				frag.add_mul_rgb(tex, splatting.b);
			}

			// + (1.0 - splatting.r - splatting.g - splatting.b)* detail3
			tFixPoint scale = FIX_POINT_COLOR_MAX - splatting.r - splatting.g - splatting.b;
			if (scale)
			{
				getSample_texture(tex.r, tex.g, tex.b, IT + tex_detail3, gl_TexCoord[0], gl_TexCoord[1]);
				frag.add_mul_rgb(tex, scale);
			}

			// + (1.0 - splatting.a) * detail4
			scale = FIX_POINT_COLOR_MAX - splatting.a;
			if (scale)
			{
				getSample_texture(tex.r, tex.g, tex.b, IT + tex_detail4, gl_TexCoord[0], gl_TexCoord[1]);
				frag.add_mul_rgb(tex, scale);
			}

			//) * gl_Color
			frag.r = imulFix_simple(frag.r, gl_Color.r);
			frag.g = imulFix_simple(frag.g, gl_Color.g);
			frag.b = imulFix_simple(frag.b, gl_Color.b);

			gl_FragColor = fix_to_sample(frag.r, frag.g, frag.b);


			/*
				tFixPoint m = FIX_POINT_ONE - splatting.a;
				r0 += imulFix4(r1, m);
				g0 += imulFix4(g1, m);
				b0 += imulFix4(b1, m);


				r0 = clampfix_mincolor(imulFix_simple(r0, gl_Color.r));
				g0 = clampfix_mincolor(imulFix_simple(g0, gl_Color.g));
				b0 = clampfix_mincolor(imulFix_simple(b0, gl_Color.b));

				gl_FragColor = fix_to_sample(r0, g0, b0);
			*/

#if 0
			gl_Color.r = tofix(line.c[0][0].r, inversew);
			gl_Color.g = tofix(line.c[0][0].g, inversew);
			gl_Color.b = tofix(line.c[0][0].b, inversew);
			gl_Color.a = tofix(line.c[0][0].b, inversew);

			//getSample_texture(a1, r1, g1, b1, IT + tex_layout, tx1, ty1);
			//texture2DFix(splatting, tex_layout, gl_TexCoord + 2);
			getSample_texture(splatting.a, splatting.r, splatting.g, splatting.b,
				IT + tex_layout, gl_TexCoord[2], gl_TexCoord[3]);

			getSample_texture(r0, g0, b0, IT + tex_detail0, gl_TexCoord[0], gl_TexCoord[1]);
			r2 = imulFix_simple(r0, splatting.r);
			g2 = imulFix_simple(g0, splatting.r);
			b2 = imulFix_simple(b0, splatting.r);

			getSample_texture(r0, g0, b0, IT + tex_detail1, gl_TexCoord[0], gl_TexCoord[1]);
			gl_FragColor = fix_to_sample(r0, g0, b0);
#endif

			/*
			r2 += imulFix_simple(r0, splatting.g);
			g2 += imulFix_simple(g0, splatting.g);
			b2 += imulFix_simple(b0, splatting.g);

			getSample_texture(r0, g0, b0, IT + tex_detail2, gl_TexCoord[0], gl_TexCoord[1]);
			r2 += imulFix_simple(r0, splatting.b);
			g2 += imulFix_simple(g0, splatting.b);
			b2 += imulFix_simple(b0, splatting.b);

			tFixPoint m = FIX_POINT_ONE - splatting.r - splatting.g - splatting.b;
			getSample_texture(r0, g0, b0, IT + tex_detail3, gl_TexCoord[0], gl_TexCoord[1]);
			r2 += imulFix_simple(r0, m);
			g2 += imulFix_simple(g0, m);
			b2 += imulFix_simple(b0, m);

			m = FIX_POINT_ONE - splatting.a;
			getSample_texture(r0, g0, b0, IT + tex_detail4, gl_TexCoord[0], gl_TexCoord[1]);
			r2 += imulFix_simple(r0, m);
			g2 += imulFix_simple(g0, m);
			b2 += imulFix_simple(b0, m);

			r2 = imulFix_simple(r2, gl_Color.r);
			g2 = imulFix_simple(g2, gl_Color.g);
			b2 = imulFix_simple(b2, gl_Color.b);

			dst[i] = fix_to_sample(clampfix_mincolor(r2), clampfix_mincolor(g2), clampfix_mincolor(b2));
			*/
		}
	BL_shader_return:
#ifdef IPOL_Z
		line.z[0] += slopeZ;
#endif
#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
#ifdef IPOL_C1
		line.c[1][0] += slopeC[1];
#endif
#ifdef IPOL_C2
		line.c[2][0] += slopeC[2];
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_T1
		line.t[1][0] += slopeT[1];
#endif
#ifdef IPOL_L0
		line.l[0][0] += slopeL[0];
#endif
	}
#include "burning_shader_glsl_emu_undefine.h"
}
#endif // #if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader)


//! creates a flat triangle renderer
IBurningShader* createTriangleRendererGTextureLightMap2_M4(CBurningVideoDriver* driver)
{
	return new CTRGTextureLightMap2_M4(driver);
}

burning_namespace_end

#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_
