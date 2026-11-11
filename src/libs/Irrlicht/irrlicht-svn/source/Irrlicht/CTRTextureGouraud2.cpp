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
#define IPOL_C1
//#define IPOL_C2
#define IPOL_T0
//#define IPOL_T1
//#define IPOL_L0

#include "burning_shader_compile_verify.h"

class CTRTextureGouraud2 : public IBurningShader
{
public:

	//! constructor
	CTRTextureGouraud2(CBurningVideoDriver* driver);

	//! draws an indexed triangle list
	virtual void drawTriangle(const s4DVertex* burning_restrict a, const s4DVertex* burning_restrict b, const s4DVertex* burning_restrict c) IRR_OVERRIDE;
	virtual bool canWireFrame() IRR_OVERRIDE { return true; }

	virtual void OnSetMaterial_BL(const SBurningShaderMaterial& material) IRR_OVERRIDE;
	virtual void OnRender_BL(IMaterialRendererServices* service) IRR_OVERRIDE;

private:

	// fragment shader
	typedef void (CTRTextureGouraud2::* tFragmentShader) ();
	tFragmentShader fragmentShader;

	void fragment_linear();

	void frag_BFT_IRR_0225_0x3bf0d5a1();
	tFixPoint mEmissive_fix[4];

	void frag_BFT_IRR_0342_0x11b0394b();
	glslEmu::samplerCube cubeTex;
	f32 Roughness;

};

//! constructor
CTRTextureGouraud2::CTRTextureGouraud2(CBurningVideoDriver* driver)
	: IBurningShader(driver, EMT_SOLID)
{
#ifdef _DEBUG
	setDebugName("CTRTextureGouraud2");
#endif
	fragmentShader = &CTRTextureGouraud2::fragment_linear;

	//frag_BFT_IRR_0225_0x3bf0d5a1
	mEmissive_fix[0] = 0;
	mEmissive_fix[1] = 0;
	mEmissive_fix[2] = 0;
	mEmissive_fix[3] = 0;

	//frag_BFT_IRR_0342_0x11b0394b
	cubeTex = 0;
	Roughness = 0.f;
}

/*!
*/
void CTRTextureGouraud2::OnSetMaterial_BL(const SBurningShaderMaterial& material)
{
	if (material.FragmentShader.id == BFT_IRR_0225_0x3bf0d5a1) fragmentShader = &CTRTextureGouraud2::frag_BFT_IRR_0225_0x3bf0d5a1;
	else if (material.FragmentShader.id == BFT_IRR_0342_0x11b0394b) fragmentShader = &CTRTextureGouraud2::frag_BFT_IRR_0342_0x11b0394b;
	else fragmentShader = &CTRTextureGouraud2::fragment_linear;
}

void CTRTextureGouraud2::OnRender_BL(IMaterialRendererServices* service)
{
	if (glsl.MaterialLink->FragmentShader.id == BFT_IRR_0225_0x3bf0d5a1)
	{
		const f32* mEmissive = bl_uniform_p(f32, "mEmissive");
		mEmissive_fix[0] = tofix(mEmissive[0], FIX_POINT_F32_MUL * COLOR_MAX);
		mEmissive_fix[1] = tofix(mEmissive[1], FIX_POINT_F32_MUL * COLOR_MAX);
		mEmissive_fix[2] = tofix(mEmissive[2], FIX_POINT_F32_MUL * COLOR_MAX);
		mEmissive_fix[3] = tofix(mEmissive[3], FIX_POINT_F32_MUL * COLOR_MAX);
	}
	else if (glsl.MaterialLink->FragmentShader.id == BFT_IRR_0342_0x11b0394b)
	{
		cubeTex = *bl_uniform_p(glslEmu::samplerCube, "cubeTex");
		Roughness = *bl_uniform_p(f32, "Roughness");
	}
}


/*!
*/
void CTRTextureGouraud2::fragment_linear()
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

#ifdef IPOL_L0
	sVec3Pack_unpack slopeL[BURNING_MATERIAL_MAX_LIGHT_TANGENT];
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
#ifdef IPOL_C1
	slopeC[1] = (line.c[1][1] - line.c[1][0]) * invDeltaX;
#endif
#ifdef IPOL_C2
	slopeC[2] = (line.c[2][1] - line.c[2][0]) * invDeltaX;
#endif
#ifdef IPOL_T0
	slopeT[0] = (line.t[0][1] - line.t[0][0]) * invDeltaX;
#endif
#ifdef IPOL_T1
	slopeT[1] = (line.t[1][1] - line.t[1][0]) * invDeltaX;
#endif
#ifdef IPOL_L0
	slopeL[0] = (line.l[0][1] - line.l[0][0]) * invDeltaX;
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
#ifdef IPOL_C1
	line.c[1][0] += slopeC[1] * subPixel;
#endif
#ifdef IPOL_C2
	line.c[2][0] += slopeC[2] * subPixel;
#endif
#ifdef IPOL_T0
	line.t[0][0] += slopeT[0] * subPixel;
#endif
#ifdef IPOL_T1
	line.t[1][0] += slopeT[1] * subPixel;
#endif
#ifdef IPOL_L0
	line.l[0][0] += slopeL[0] * subPixel;
#endif
#endif

	SOFTWARE_DRIVER_2_CLIPCHECK;
	dst = (tRenderTargetColorSample*)RenderTarget.color->getData() + (line.y * RenderTarget.color->getDimension().Width) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*)RenderTarget.depth->getData() + (line.y * RenderTarget.color->getDimension().Width) + xStart;
#endif


	f32 inversew = INVERSE_W_RANGE;

	tFixPoint tx0;
	tFixPoint ty0;

#ifdef IPOL_C0
	tFixPoint r0, g0, b0;
	tFixPoint r1, g1, b1;
#endif

#ifdef IPOL_C1
	tFixPoint aFog = FIX_POINT_ONE;
#endif


#ifdef IPOL_C2
	tFixPoint r3, g3, b3;
#endif

#if defined(BURNINGVIDEO_RENDERER_FAST) && COLOR_MAX==0xff
	u32 dIndex = (line.y & 3) << 2;
#endif

	for (s32 i = 0; i <= dx; i += SOFTWARE_DRIVER_2_STEP_X)
	{
		//if test active only first pixel
		if ((0 == EdgeTestPass) & (i > line.x_edgetest)) break;

#ifdef CMP_Z
		if (line.z[0] < z[i])
#endif
#ifdef CMP_W
		if (line.w[0] >= z[i])
#endif
		{
#ifdef WRITE_Z
			z[i] = line.z[0];
#endif
#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
			inversew = fix_inverse32(line.w[0]);
#endif

#ifdef IPOL_C1
			//complete inside fog
			if (TL_Flag & TL_FOG)
			{
				aFog = tofix(line.c[1][0].a, inversew);
				if (aFog <= 0)
				{
					dst[i] = fog_color_sample;
					continue;
				}
			}
#endif
			tx0 = tofix(line.t[0][0].x, inversew);
			ty0 = tofix(line.t[0][0].y, inversew);

#ifdef IPOL_C0

			getSample_texture(r0, g0, b0, IT + 0, tx0, ty0);
			vec4_to_fix(r1, g1, b1, line.c[0][0], inversew);

			r0 = imulFix_simple(r0, r1);
			g0 = imulFix_simple(g0, g1);
			b0 = imulFix_simple(b0, b1);

#ifdef IPOL_C1

			//specular highlight
			if (TL_Flag & TL_SPECULAR)
			{
				vec4_to_fix(r1, g1, b1, line.c[1][0], inversew * COLOR_MAX);
				r0 = clampfix_maxcolor(r1 + r0);
				g0 = clampfix_maxcolor(g1 + g0);
				b0 = clampfix_maxcolor(b1 + b0);
			}
			//mix with distance
			if (aFog < FIX_POINT_ONE) //TL_Flag & TL_FOG)
			{
				r0 = fog_color[1] + imulFix(aFog, r0 - fog_color[1]);
				g0 = fog_color[2] + imulFix(aFog, g0 - fog_color[2]);
				b0 = fog_color[3] + imulFix(aFog, b0 - fog_color[3]);
			}
			dst[i] = fix_to_sample_nearest(r0, g0, b0);

#else
			dst[i] = fix_to_sample(
				imulFix_simple(r0, r1),
				imulFix_simple(g0, g1),
				imulFix_simple(b0, b1)
			);
#endif

#else

#if defined(BURNINGVIDEO_RENDERER_FAST) && COLOR_MAX==0xff
			const tFixPointu d = dithermask[dIndex | (i) & 3];
			dst[i] = texelFetch(&IT[0], d + tx0, d + ty0);
#else
			getSample_texture(r0, g0, b0, &IT[0], tx0, ty0);
			dst[i] = fix_to_sample(r0, g0, b0);
#endif

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

}

void CTRTextureGouraud2::drawTriangle(const s4DVertex* burning_restrict a, const s4DVertex* burning_restrict b, const s4DVertex* burning_restrict c)
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

	if (F32_LOWER_EQUAL_0(scan.invDeltaY[0]))
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

#ifdef IPOL_C1
	scan.slopeC[1][0] = (c->Color[1] - a->Color[1]) * scan.invDeltaY[0];
	scan.c[1][0] = a->Color[1];
#endif

#ifdef IPOL_C2
	scan.slopeC[2][0] = (c->Color[2] - a->Color[2]) * scan.invDeltaY[0];
	scan.c[2][0] = a->Color[2];
#endif

#ifdef IPOL_T0
	scan.slopeT[0][0] = (c->Tex[0] - a->Tex[0]) * scan.invDeltaY[0];
	scan.t[0][0] = a->Tex[0];
#endif

#ifdef IPOL_T1
	scan.slopeT[1][0] = (c->Tex[1] - a->Tex[1]) * scan.invDeltaY[0];
	scan.t[1][0] = a->Tex[1];
#endif

#ifdef IPOL_L0
	scan.slopeL[0][0] = (c->LightTangent[0] - a->LightTangent[0]) * scan.invDeltaY[0];
	scan.l[0][0] = a->LightTangent[0];
#endif

	// top left fill convention y run
	s32 yStart;
	s32 yEnd;

#ifdef SUBTEXEL
	f32 subPixel;
#endif

	// rasterize upper sub-triangle
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

#ifdef IPOL_C1
		scan.slopeC[1][1] = (b->Color[1] - a->Color[1]) * scan.invDeltaY[1];
		scan.c[1][1] = a->Color[1];
#endif

#ifdef IPOL_C2
		scan.slopeC[2][1] = (b->Color[2] - a->Color[2]) * scan.invDeltaY[1];
		scan.c[2][1] = a->Color[2];
#endif

#ifdef IPOL_T0
		scan.slopeT[0][1] = (b->Tex[0] - a->Tex[0]) * scan.invDeltaY[1];
		scan.t[0][1] = a->Tex[0];
#endif

#ifdef IPOL_T1
		scan.slopeT[1][1] = (b->Tex[1] - a->Tex[1]) * scan.invDeltaY[1];
		scan.t[1][1] = a->Tex[1];
#endif

#ifdef IPOL_L0
		scan.slopeL[0][1] = (b->LightTangent[0] - a->LightTangent[0]) * scan.invDeltaY[1];
		scan.l[0][1] = a->LightTangent[0];
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

#ifdef IPOL_C1
		scan.c[1][0] += scan.slopeC[1][0] * subPixel;
		scan.c[1][1] += scan.slopeC[1][1] * subPixel;
#endif

#ifdef IPOL_C2
		scan.c[2][0] += scan.slopeC[2][0] * subPixel;
		scan.c[2][1] += scan.slopeC[2][1] * subPixel;
#endif

#ifdef IPOL_T0
		scan.t[0][0] += scan.slopeT[0][0] * subPixel;
		scan.t[0][1] += scan.slopeT[0][1] * subPixel;
#endif

#ifdef IPOL_T1
		scan.t[1][0] += scan.slopeT[1][0] * subPixel;
		scan.t[1][1] += scan.slopeT[1][1] * subPixel;
#endif

#ifdef IPOL_L0
		scan.l[0][0] += scan.slopeL[0][0] * subPixel;
		scan.l[0][1] += scan.slopeL[0][1] * subPixel;
#endif

#endif

		// rasterize the edge scanlines
		line.x_edgetest = fill_convention_edge(scan.slopeX[scan.left]);

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

#ifdef IPOL_C1
			line.c[1][scan.left] = scan.c[1][0];
			line.c[1][scan.right] = scan.c[1][1];
#endif

#ifdef IPOL_C2
			line.c[2][scan.left] = scan.c[2][0];
			line.c[2][scan.right] = scan.c[2][1];
#endif

#ifdef IPOL_T0
			line.t[0][scan.left] = scan.t[0][0];
			line.t[0][scan.right] = scan.t[0][1];
#endif

#ifdef IPOL_T1
			line.t[1][scan.left] = scan.t[1][0];
			line.t[1][scan.right] = scan.t[1][1];
#endif

#ifdef IPOL_L0
			line.l[0][scan.left] = scan.l[0][0];
			line.l[0][scan.right] = scan.l[0][1];
#endif

			// render a scanline
			if_interlace_scanline
			(this->*fragmentShader) ();
			if (EdgeTestPass & edge_test_first_line) break;


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

#ifdef IPOL_C1
			scan.c[1][0] += scan.slopeC[1][0];
			scan.c[1][1] += scan.slopeC[1][1];
#endif

#ifdef IPOL_C2
			scan.c[2][0] += scan.slopeC[2][0];
			scan.c[2][1] += scan.slopeC[2][1];
#endif

#ifdef IPOL_T0
			scan.t[0][0] += scan.slopeT[0][0];
			scan.t[0][1] += scan.slopeT[0][1];
#endif

#ifdef IPOL_T1
			scan.t[1][0] += scan.slopeT[1][0];
			scan.t[1][1] += scan.slopeT[1][1];
#endif

#ifdef IPOL_L0
			scan.l[0][0] += scan.slopeL[0][0];
			scan.l[0][1] += scan.slopeL[0][1];
#endif

		}
	}

	// rasterize lower sub-triangle
	if (F32_GREATER_0(scan.invDeltaY[2]))
	{
		// advance to middle point
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
#ifdef IPOL_C1
			scan.c[1][0] = a->Color[1] + scan.slopeC[1][0] * temp[0];
#endif
#ifdef IPOL_C2
			scan.c[2][0] = a->Color[2] + scan.slopeC[2][0] * temp[0];
#endif
#ifdef IPOL_T0
			scan.t[0][0] = a->Tex[0] + scan.slopeT[0][0] * temp[0];
#endif
#ifdef IPOL_T1
			scan.t[1][0] = a->Tex[1] + scan.slopeT[1][0] * temp[0];
#endif

#ifdef IPOL_L0
			scan.l[0][0] = sVec3Pack_unpack(a->LightTangent[0]) + scan.slopeL[0][0] * temp[0];
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

#ifdef IPOL_C1
		scan.slopeC[1][1] = (c->Color[1] - b->Color[1]) * scan.invDeltaY[2];
		scan.c[1][1] = b->Color[1];
#endif

#ifdef IPOL_C2
		scan.slopeC[2][1] = (c->Color[2] - b->Color[2]) * scan.invDeltaY[2];
		scan.c[2][1] = b->Color[2];
#endif

#ifdef IPOL_T0
		scan.slopeT[0][1] = (c->Tex[0] - b->Tex[0]) * scan.invDeltaY[2];
		scan.t[0][1] = b->Tex[0];
#endif

#ifdef IPOL_T1
		scan.slopeT[1][1] = (c->Tex[1] - b->Tex[1]) * scan.invDeltaY[2];
		scan.t[1][1] = b->Tex[1];
#endif

#ifdef IPOL_L0
		scan.slopeL[0][1] = (c->LightTangent[0] - b->LightTangent[0]) * scan.invDeltaY[2];
		scan.l[0][1] = b->LightTangent[0];
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

#ifdef IPOL_C1
		scan.c[1][0] += scan.slopeC[1][0] * subPixel;
		scan.c[1][1] += scan.slopeC[1][1] * subPixel;
#endif

#ifdef IPOL_C2
		scan.c[2][0] += scan.slopeC[2][0] * subPixel;
		scan.c[2][1] += scan.slopeC[2][1] * subPixel;
#endif

#ifdef IPOL_T0
		scan.t[0][0] += scan.slopeT[0][0] * subPixel;
		scan.t[0][1] += scan.slopeT[0][1] * subPixel;
#endif

#ifdef IPOL_T1
		scan.t[1][0] += scan.slopeT[1][0] * subPixel;
		scan.t[1][1] += scan.slopeT[1][1] * subPixel;
#endif

#ifdef IPOL_L0
		scan.l[0][0] += scan.slopeL[0][0] * subPixel;
		scan.l[0][1] += scan.slopeL[0][1] * subPixel;
#endif

#endif
		// rasterize the edge scanlines
		line.x_edgetest = fill_convention_edge(scan.slopeX[scan.left]);

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

#ifdef IPOL_C1
			line.c[1][scan.left] = scan.c[1][0];
			line.c[1][scan.right] = scan.c[1][1];
#endif

#ifdef IPOL_C2
			line.c[2][scan.left] = scan.c[2][0];
			line.c[2][scan.right] = scan.c[2][1];
#endif

#ifdef IPOL_T0
			line.t[0][scan.left] = scan.t[0][0];
			line.t[0][scan.right] = scan.t[0][1];
#endif

#ifdef IPOL_T1
			line.t[1][scan.left] = scan.t[1][0];
			line.t[1][scan.right] = scan.t[1][1];
#endif

#ifdef IPOL_L0
			line.l[0][scan.left] = scan.l[0][0];
			line.l[0][scan.right] = scan.l[0][1];
#endif

			// render a scanline
			if_interlace_scanline
			(this->*fragmentShader) ();
			if (EdgeTestPass & edge_test_first_line) break;


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

#ifdef IPOL_C1
			scan.c[1][0] += scan.slopeC[1][0];
			scan.c[1][1] += scan.slopeC[1][1];
#endif

#ifdef IPOL_C2
			scan.c[2][0] += scan.slopeC[2][0];
			scan.c[2][1] += scan.slopeC[2][1];
#endif

#ifdef IPOL_T0
			scan.t[0][0] += scan.slopeT[0][0];
			scan.t[0][1] += scan.slopeT[0][1];
#endif

#ifdef IPOL_T1
			scan.t[1][0] += scan.slopeT[1][0];
			scan.t[1][1] += scan.slopeT[1][1];
#endif

#ifdef IPOL_L0
			scan.l[0][0] += scan.slopeL[0][0];
			scan.l[0][1] += scan.slopeL[0][1];
#endif

		}
	}

}


/*!
uniform sampler2D myTexture;
uniform vec4 mEmissive;

void main (void)
{
	vec4 col = texture2D(myTexture, vec2(gl_TexCoord[0]));
	col *= gl_Color;
	gl_FragColor = col * 4.0;
	gl_FragColor += mEmissive;
}
*/
#undef IPOL_C1
#include "burning_shader_compile_verify.h"

void CTRTextureGouraud2::frag_BFT_IRR_0225_0x3bf0d5a1()
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

#endif

	SOFTWARE_DRIVER_2_CLIPCHECK;
	dst = (tRenderTargetColorSample*)RenderTarget.color->getData() + (line.y * RenderTarget.color->getDimension().Width) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*)RenderTarget.depth->getData() + (line.y * RenderTarget.color->getDimension().Width) + xStart;
#endif

	f32 inversew = INVERSE_W_RANGE;

#include "burning_shader_glsl_emu_define.h"

#define burning_shader_fixpoint

#ifdef burning_shader_fixpoint
	tFixPoint r0, g0, b0;
	tFixPoint r2, g2, b2;

#else
	uniform(sampler2D, myTexture);
#endif

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

#ifdef burning_shader_fixpoint

#ifdef INVERSE_W
			inversew = fix_inverse32(line.w[0]);
#endif
			getSample_texture(r0, g0, b0, IT + 0, tofix(line.t[0][0].x, inversew), tofix(line.t[0][0].y, inversew));

			vec4_to_fix(r2, g2, b2, line.c[0][0], inversew);
			r0 = clampfix_maxcolor(imulFix4(r0, r2) + mEmissive_fix[0]);
			g0 = clampfix_maxcolor(imulFix4(g0, g2) + mEmissive_fix[1]);
			b0 = clampfix_maxcolor(imulFix4(b0, b2) + mEmissive_fix[2]);
			gl_FragColor = fix_to_sample_nearest(r0, g0, b0);
#else
			//inversew = fix_inverse32(line.w[0]);
			inversew = reciprocal_zero_no(line.w[0]);

			gl_TexCoord[0] = line.t[0][0] * inversew;
			gl_Color = line.c[0][0] * inversew;

			vec4 col = texture2D(myTexture, vec2(gl_TexCoord[0]));
			col *= gl_Color;
			gl_FragColor = col * 4.f;
#endif
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


void cubeProject(
	E_CUBE_SURFACE& face,
	sVec2& uv,
	const sVec4& in,
	const f32 inversew
)
{
	sVec4 d;
	d.x = in.x * inversew;
	d.y = in.y * inversew;
	d.z = in.z * inversew;
	d.w = 0.f;
	//d.normalize_dir_xyz();

	//dominant axis
	sVec4 a(fabsf(d.x), fabsf(d.y), fabsf(d.z));

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

	// [-1,1] -> [0,1]
	uv.x = uv.x * 0.5f + 0.5f;
	uv.y = uv.y * 0.5f + 0.5f;

	//BL_textureWrap(uv, texObj);
	if (uv.x < 0.f) uv.x = 0;
	else if (uv.x > 1.f) uv.x = 1.f;

	if (uv.y < 0.f) uv.y = 0;
	else if (uv.y > 1.f) uv.y = 1.f;

}

/*
// cubeMapReflection.frag
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
#undef IPOL_T0
#undef IPOL_C1
#include "burning_shader_compile_verify.h"

void CTRTextureGouraud2::frag_BFT_IRR_0342_0x11b0394b()
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
#ifdef IPOL_C1
	slopeC[1] = (line.c[1][1] - line.c[1][0]) * invDeltaX;
#endif

#ifdef IPOL_T0
	slopeT[0] = (line.t[0][1] - line.t[0][0]) * invDeltaX;
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
#ifdef IPOL_C1
	line.c[1][0] += slopeC[1] * subPixel;
#endif
#ifdef IPOL_T0
	line.t[0][0] += slopeT[0] * subPixel;
#endif

#endif

	SOFTWARE_DRIVER_2_CLIPCHECK;
	dst = (tRenderTargetColorSample*)RenderTarget.color->getData() + (line.y * RenderTarget.color->getDimension().Width) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*)RenderTarget.depth->getData() + (line.y * RenderTarget.color->getDimension().Width) + xStart;
#endif

	f32 inversew = INVERSE_W_RANGE;

#define burning_shader_fragment_fix
#include "burning_shader_glsl_emu_define.h"
irr_unreferenced_parameter(gl_Color);


	const CSoftwareTexture2_Bound* cubeTex_b = IT[cubeTex].Texture->getTexBound_index();
	E_CUBE_SURFACE face;
	sVec2 uv;

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
			inversew = reciprocal_zero_no(line.w[0]);
#endif
			// cubeProject
			cubeProject(face, uv, line.c[0][0], inversew);
			gl_TexCoord[0] = tofix(uv.x * cubeTex_b[face].mat[0], FIX_POINT_F32_MUL);
			gl_TexCoord[1] = tofix(uv.y * cubeTex_b[face].mat[2], FIX_POINT_F32_MUL);

			IT[cubeTex].setFace(face);
			gl_FragColor = texelFetch(IT + cubeTex, gl_TexCoord[0], gl_TexCoord[1]);

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




//! creates a flat triangle renderer
IBurningShader* createTriangleRendererTextureGouraud2(CBurningVideoDriver* driver)
{
	// ETR_TEXTURE_GOURAUD
	return new CTRTextureGouraud2(driver);
}

burning_namespace_end

#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_

