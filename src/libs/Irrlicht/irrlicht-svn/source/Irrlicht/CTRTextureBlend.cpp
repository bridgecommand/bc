// Copyright (C) 2002-2012 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#include "IBurningShader.h"

#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_

// compile flag for this file
#undef USE_ZBUFFER
#undef IPOL_Z
#undef CMP_Z
#undef WRITE_Z

#undef IPOL_W
#undef CMP_W
#undef WRITE_W

#undef SUBTEXEL
#undef INVERSE_W

#undef IPOL_C0
#undef IPOL_T0
#undef IPOL_T1

// define render case
#define SUBTEXEL
#define INVERSE_W

#define USE_ZBUFFER
#define IPOL_W
#define CMP_W
#define WRITE_W

#define IPOL_C0
#define IPOL_T0
//#define IPOL_T1

// apply global override
#ifndef SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT
	#undef INVERSE_W
#endif

#ifndef SOFTWARE_DRIVER_2_SUBTEXEL
	#undef SUBTEXEL
#endif

#if BURNING_MATERIAL_MAX_COLORS < 1
	#undef IPOL_C0
#endif

#if !defined ( SOFTWARE_DRIVER_2_USE_WBUFFER ) && defined ( USE_ZBUFFER )
	#ifndef SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT
		#undef IPOL_W
	#endif
	#define IPOL_Z

	#ifdef CMP_W
		#undef CMP_W
		#define CMP_Z
	#endif

	#ifdef WRITE_W
		#undef WRITE_W
		#define WRITE_Z
	#endif

#endif



namespace irr
{

namespace video
{

	class CTRTextureBlend : public IBurningShader
	{
	public:

		//! constructor
		CTRTextureBlend(CBurningVideoDriver* driver);

		//! draws an indexed triangle list
		virtual void drawTriangle(const s4DVertex* burning_restrict a, const s4DVertex* burning_restrict b, const s4DVertex* burning_restrict c) _IRR_OVERRIDE_;
		virtual void OnSetMaterial(const SBurningShaderMaterial& material) _IRR_OVERRIDE_;

#if defined(PATCH_SUPERTUX_8_0_1)

		virtual void setZCompareFunc(u32 func)
		{
			depth_func = (E_COMPARISON_FUNC)func;
		}
		virtual void setParam(u32 index, f32 value)
		{
			SBurningShaderMaterial material;
			material.org.ZBuffer = depth_func;
			material.org.MaterialTypeParam = value;
			OnSetMaterial(material);
	}

#endif

private:
	// fragment shader
	typedef void (CTRTextureBlend::*tFragmentShader) ();
	void fragment_dst_color_zero ();
	void fragment_dst_color_one ();
	void fragment_dst_color_src_alpha ();
	void fragment_dst_color_one_minus_dst_alpha ();
	void fragment_zero_one_minus_scr_color ();
	void fragment_src_color_src_alpha ();
	void fragment_one_one_minus_src_alpha ();
	void fragment_one_minus_dst_alpha_one();
	void fragment_src_alpha_one();
	void fragment_src_alpha_one_minus_src_alpha();

	tFragmentShader fragmentShader;

	E_COMPARISON_FUNC depth_func;
};

//! constructor
CTRTextureBlend::CTRTextureBlend(CBurningVideoDriver* driver)
: IBurningShader(driver)
{
	#ifdef _DEBUG
	setDebugName("CTRTextureBlend");
	#endif

	depth_func = ECFN_LESSEQUAL;
	fragmentShader = &CTRTextureBlend::fragment_dst_color_zero;
}

/*!
*/
void CTRTextureBlend::OnSetMaterial(const SBurningShaderMaterial& material)
{
	int showname = 0;

	depth_func = (E_COMPARISON_FUNC)material.org.ZBuffer;
	AlphaRef = 0; // tofix(material.org.MaterialTypeParam, FIXPOINT_COLOR_MAX);

	E_BLEND_FACTOR srcFact,dstFact;
	E_MODULATE_FUNC modulate;
	u32 alphaSrc;
	unpack_textureBlendFunc ( srcFact, dstFact, modulate, alphaSrc, material.org.MaterialTypeParam);

	fragmentShader = 0;

	if ( srcFact == EBF_DST_COLOR && dstFact == EBF_ZERO )
	{
		fragmentShader = &CTRTextureBlend::fragment_dst_color_zero;
	}
	else if ( srcFact == EBF_DST_COLOR && dstFact == EBF_ONE )
	{
		fragmentShader = &CTRTextureBlend::fragment_dst_color_one;
	}
	else if ( srcFact == EBF_DST_COLOR && dstFact == EBF_SRC_ALPHA)
	{
		fragmentShader = &CTRTextureBlend::fragment_dst_color_src_alpha;
	}
	else if ( srcFact == EBF_DST_COLOR && dstFact == EBF_ONE_MINUS_DST_ALPHA)
	{
		fragmentShader = &CTRTextureBlend::fragment_dst_color_one_minus_dst_alpha;
	}
	else if ( srcFact == EBF_ZERO && dstFact == EBF_ONE_MINUS_SRC_COLOR )
	{
		fragmentShader = &CTRTextureBlend::fragment_zero_one_minus_scr_color;
	}
	else if ( srcFact == EBF_ONE && dstFact == EBF_ONE_MINUS_SRC_ALPHA)
	{
		fragmentShader = &CTRTextureBlend::fragment_one_one_minus_src_alpha;
	}
	else if ( srcFact == EBF_ONE_MINUS_DST_ALPHA && dstFact == EBF_ONE )
	{
		fragmentShader = &CTRTextureBlend::fragment_one_minus_dst_alpha_one;
	}
	else if ( srcFact == EBF_SRC_ALPHA && dstFact == EBF_ONE )
	{
		fragmentShader = &CTRTextureBlend::fragment_src_alpha_one;
	}
	else if (srcFact == EBF_SRC_ALPHA && dstFact == EBF_ONE_MINUS_SRC_ALPHA)
	{
		fragmentShader = &CTRTextureBlend::fragment_src_alpha_one_minus_src_alpha;
	}
	else if ( srcFact == EBF_SRC_COLOR && dstFact == EBF_SRC_ALPHA )
	{
		fragmentShader = &CTRTextureBlend::fragment_src_color_src_alpha;
	}
	else
	{
		showname = 1;
		fragmentShader = &CTRTextureBlend::fragment_dst_color_zero;
	}

	static const c8 *n[] =
	{
		"gl_zero",
		"gl_one",
		"gl_dst_color",
		"gl_one_minus_dst_color",
		"gl_src_color",
		"gl_one_minus_src_color",
		"gl_src_alpha",
		"gl_one_minus_src_alpha",
		"gl_dst_alpha",
		"gl_one_minus_dst_alpha",
		"gl_src_alpha_saturate"
	};

	static E_BLEND_FACTOR lsrcFact = EBF_ZERO;
	static E_BLEND_FACTOR ldstFact = EBF_ZERO;

	if ( showname && ( lsrcFact != srcFact || ldstFact != dstFact ) )
	{
		char buf[128];
		snprintf_irr ( buf, 128, "missing shader: %s %s",n[srcFact], n[dstFact] );
		os::Printer::log( buf, ELL_WARNING);

		lsrcFact = srcFact;
		ldstFact = dstFact;
	}

}


/*!
*/
void CTRTextureBlend::fragment_dst_color_src_alpha ()
{
	tVideoSample *dst;

#ifdef USE_ZBUFFER
	fp24 *z;
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
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left( line.x[0] );
	xEnd = fill_convention_right( line.x[1] );

	dx = xEnd - xStart;

	if ( dx < 0 )
		return;

	// slopes
	const f32 invDeltaX = reciprocal_zero2( line.x[1] - line.x[0] );

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
	subPixel = ( (f32) xStart ) - line.x[0];
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
	dst = (tVideoSample*)RenderTarget->getData() + ( line.y * RenderTarget->getDimension().Width ) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*) DepthBuffer->lock() + ( line.y * RenderTarget->getDimension().Width ) + xStart;
#endif


	f32 iw = 	FIX_POINT_F32_MUL;

	tFixPoint a0, r0, g0, b0;
	tFixPoint     r1, g1, b1;

	s32 i;

	switch (depth_func)
	{
	default:
	case ECFN_LESSEQUAL:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] >= z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( a0,r0,g0,b0,
							&IT[0],
							tofix(line.t[0][0].x, iw),
							tofix(line.t[0][0].y, iw)
						);

		color_to_fix ( r1, g1, b1, dst[i] );

		dst[i] = fix_to_sample( clampfix_maxcolor ( imulFix_tex2 ( r0, r1 ) ),
								clampfix_maxcolor ( imulFix_tex2 ( g0, g1 ) ),
								clampfix_maxcolor ( imulFix_tex2 ( b0, b1 ) )
							);
		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}
	break;

	case 2:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] == z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( a0,r0,g0,b0,
							&IT[0],
							tofix ( line.t[0][0].x,iw),
							tofix ( line.t[0][0].y,iw)
						);

		color_to_fix ( r1, g1, b1, dst[i] );

		dst[i] = fix_to_sample( clampfix_maxcolor ( imulFix_tex2 ( r0, r1 ) ),
								clampfix_maxcolor ( imulFix_tex2 ( g0, g1 ) ),
								clampfix_maxcolor ( imulFix_tex2 ( b0, b1 ) )
							);

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}break;
	} // zcompare

}

/*!
*/
void CTRTextureBlend::fragment_src_color_src_alpha ()
{
	tVideoSample *dst;

#ifdef USE_ZBUFFER
	fp24 *z;
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
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left( line.x[0] );
	xEnd = fill_convention_right( line.x[1] );

	dx = xEnd - xStart;
	if ( dx < 0 )
		return;

	// slopes
	const f32 invDeltaX = reciprocal_zero2( line.x[1] - line.x[0] );

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
	subPixel = ( (f32) xStart ) - line.x[0];
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
	dst = (tVideoSample*)RenderTarget->getData() + ( line.y * RenderTarget->getDimension().Width ) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*) DepthBuffer->lock() + ( line.y * RenderTarget->getDimension().Width ) + xStart;
#endif


	f32 iw = 	FIX_POINT_F32_MUL;

	tFixPoint a0, r0, g0, b0;
	tFixPoint     r1, g1, b1;

#ifdef IPOL_C0
	tFixPoint a2,r2, g2, b2;
#endif

	s32 i;

	switch (depth_func)
	{
	default:
	case ECFN_LESSEQUAL:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] >= z[i] )
#endif

		{
			//solves example 08. todo: depth_write. 
#ifdef WRITE_W
		//z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( a0, r0, g0, b0, &IT[0],	tofix ( line.t[0][0].x,iw),	tofix ( line.t[0][0].y,iw) );

#ifdef IPOL_C0
		vec4_to_fix(a2,r2, g2, b2, line.c[0][0], iw);
		//a0 = imulFix(a0, a2); why is vertex color enabled and not vertex_alpha?
		r0 = imulFix_simple(r0, r2);
		g0 = imulFix_simple(g0, g2);
		b0 = imulFix_simple(b0, b2);
#endif

		color_to_fix ( r1, g1, b1, dst[i] );
		dst[i] = fix_to_sample( clampfix_maxcolor ( imulFix_tex1( r0, r0 ) + imulFix_tex1( r1, a0 ) ),
								clampfix_maxcolor ( imulFix_tex1( g0, g0 ) + imulFix_tex1( g1, a0 ) ),
								clampfix_maxcolor ( imulFix_tex1( b0, b0 ) + imulFix_tex1( b1, a0 ) )
							);

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}
	break;

	case 2:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] == z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( a0,r0,g0,b0,
							&IT[0],
							tofix ( line.t[0][0].x,iw),
							tofix ( line.t[0][0].y,iw)
						);

		color_to_fix ( r1, g1, b1, dst[i] );

		dst[i] = fix_to_sample( clampfix_maxcolor ( imulFix_tex2 ( r0, r1 ) ),
								clampfix_maxcolor ( imulFix_tex2 ( g0, g1 ) ),
								clampfix_maxcolor ( imulFix_tex2 ( b0, b1 ) )
							);

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}break;
	
	} // zcompare

}

/*!
*/
void CTRTextureBlend::fragment_one_one_minus_src_alpha()
{
	tVideoSample *dst;

#ifdef USE_ZBUFFER
	fp24 *z;
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
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left( line.x[0] );
	xEnd = fill_convention_right( line.x[1] );

	dx = xEnd - xStart;

	if ( dx < 0 )
		return;

	// slopes
	const f32 invDeltaX = reciprocal_zero2 ( line.x[1] - line.x[0] );

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
	subPixel = ( (f32) xStart ) - line.x[0];
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
	dst = (tVideoSample*)RenderTarget->getData() + ( line.y * RenderTarget->getDimension().Width ) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*) DepthBuffer->lock() + ( line.y * RenderTarget->getDimension().Width ) + xStart;
#endif


	f32 iw = FIX_POINT_F32_MUL;

	tFixPoint a0,r0, g0, b0;
	tFixPoint	 r1, g1, b1;
#ifdef IPOL_C0
	tFixPoint	 r2, g2, b2;
#endif
	s32 i;

	switch (depth_func)
	{
	default:
	case ECFN_LESSEQUAL:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] >= z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( a0, r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		a0 = FIX_POINT_ONE - a0;

		color_to_fix1 ( r1, g1, b1, dst[i] );
#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		dst[i] = fix_to_sample( imulFix ( r0 + imulFix_simple( r1, a0 ), r2 ),
								imulFix ( g0 + imulFix_simple( g1, a0 ), g2 ),
								imulFix ( b0 + imulFix_simple( b1, a0 ), b2 )
							);
#else
		dst[i] = fix_to_sample( r0 + imulFix_simple( r1, a0 ),
								g0 + imulFix_simple( g1, a0 ),
								b0 + imulFix_simple( b1, a0 )
							);

#endif

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}
	break;

	case 2:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] == z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif
		getSample_texture ( a0, r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		a0 = FIX_POINT_ONE - a0;

		color_to_fix1 ( r1, g1, b1, dst[i] );
#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		dst[i] = fix_to_sample( imulFix ( r0 + imulFix_simple( r1, a0 ), r2 ),
								imulFix ( g0 + imulFix_simple( g1, a0 ), g2 ),
								imulFix ( b0 + imulFix_simple( b1, a0 ), b2 )
							);
#else
		dst[i] = fix_to_sample( r0 + imulFix_simple( r1, a0 ),
								g0 + imulFix_simple( g1, a0 ),
								b0 + imulFix_simple( b1, a0 )
							);

#endif

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}break;

	} // zcompare

}

/*!
*/
void CTRTextureBlend::fragment_one_minus_dst_alpha_one ()
{
	tVideoSample *dst;

#ifdef USE_ZBUFFER
	fp24 *z;
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
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left( line.x[0] );
	xEnd = fill_convention_right( line.x[1] );

	dx = xEnd - xStart;

	if ( dx < 0 )
		return;

	// slopes
	const f32 invDeltaX = reciprocal_zero2( line.x[1] - line.x[0] );

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
	subPixel = ( (f32) xStart ) - line.x[0];
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
	dst = (tVideoSample*)RenderTarget->getData() + ( line.y * RenderTarget->getDimension().Width ) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*) DepthBuffer->lock() + ( line.y * RenderTarget->getDimension().Width ) + xStart;
#endif


	f32 iw = FIX_POINT_F32_MUL;

	tFixPoint r0, g0, b0;
	tFixPoint a1, r1, g1, b1;
#ifdef IPOL_C0
	tFixPoint r2, g2, b2;
#endif
	s32 i;

	switch (depth_func)
	{
	default:
	case ECFN_LESSEQUAL:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] >= z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		color_to_fix1 ( a1, r1, g1, b1, dst[i] );
#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		a1 = FIX_POINT_ONE - a1;
		dst[i] = fix_to_sample( imulFix (imulFix_simple( r0, a1 ) + r1, r2 ),
								imulFix (imulFix_simple( g0, a1 ) + g1, g2 ),
								imulFix (imulFix_simple( b0, a1 ) + b1, b2 )
							);
#else
		dst[i] = fix_to_sample(imulFix_simple( r0, a1) + r0,
			imulFix_simple( g0, a1) + g0,
			imulFix_simple( b0, a1) + b0
							);

#endif

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}
	break;

	case 2:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] == z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif
		getSample_texture ( r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		color_to_fix1 ( a1, r1, g1, b1, dst[i] );

#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		a1 = FIX_POINT_ONE - a1;
		dst[i] = fix_to_sample( imulFix (imulFix_simple( r0, a1 ) + r1, r2 ),
								imulFix (imulFix_simple( g0, a1 ) + g1, g2 ),
								imulFix (imulFix_simple( b0, a1 ) + b1, b2 )
							);
#else
		dst[i] = fix_to_sample(imulFix_simple( r0, a1) + r0,
			imulFix_simple( g0, a1) + g0,
			imulFix_simple( b0, a1) + b0
							);

#endif

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}break;

	} // zcompare

}

/*!
*/
void CTRTextureBlend::fragment_src_alpha_one ()
{
	tVideoSample *dst;

#ifdef USE_ZBUFFER
	fp24 *z;
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
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left( line.x[0] );
	xEnd = fill_convention_right( line.x[1] );

	dx = xEnd - xStart;

	if ( dx < 0 )
		return;

	// slopes
	const f32 invDeltaX = reciprocal_zero2( line.x[1] - line.x[0] );

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
	subPixel = ( (f32) xStart ) - line.x[0];
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
	dst = (tVideoSample*)RenderTarget->getData() + ( line.y * RenderTarget->getDimension().Width ) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*) DepthBuffer->lock() + ( line.y * RenderTarget->getDimension().Width ) + xStart;
#endif


	f32 iw = FIX_POINT_F32_MUL;

	tFixPoint a0, r0, g0, b0;
	tFixPoint r1, g1, b1;
#ifdef IPOL_C0
	tFixPoint r2, g2, b2;
#endif
	s32 i;

	switch (depth_func)
	{
	default:
	case ECFN_LESSEQUAL:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] >= z[i] )
#endif

		{


#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( a0, r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		if ( a0 > 0 )
		{
			fix_color_norm(a0);

		color_to_fix ( r1, g1, b1, dst[i] );

#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		dst[i] = fix4_to_sample( a0,
								 clampfix_maxcolor (imulFix_simple(r0,a0) + r1),
								 clampfix_maxcolor (imulFix_simple(g0,a0) + g1),
								 clampfix_maxcolor (imulFix_simple(b0,a0) + b1)
								);

/*
		fix_color_norm(a0);
		dst[i] = fix4_to_sample ( a0,
								imulFix ( imulFix_simple ( r0, a0 ) + r1, r2 ),
								imulFix ( imulFix_simple ( g0, a0 ) + g1, g2 ),
								imulFix ( imulFix_simple ( b0, a0 ) + b1, b2 )
							);
*/
#else
		dst[i] = fix4_to_sample( a0,
								 clampfix_maxcolor (imulFix_simple(r0,a0 ) + r1 ),
								 clampfix_maxcolor (imulFix_simple(g0,a0 ) + g1 ),
								 clampfix_maxcolor (imulFix_simple(b0,a0 ) + b1 )
								);

#endif

#ifdef WRITE_W
			//z[i] = line.w[0];
#endif
		}

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}
	break;

	case 2:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] == z[i] )
#endif
		{

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( a0, r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		if ( a0 > 0 )
		{
			fix_color_norm(a0);

		color_to_fix ( r1, g1, b1, dst[i] );

#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		dst[i] = fix4_to_sample( a0,
								 clampfix_maxcolor ( imulFix (imulFix_simple(r0,a0 ) + r1, r2 ) ),
								 clampfix_maxcolor ( imulFix (imulFix_simple(g0,a0 ) + g1, g2 ) ),
								 clampfix_maxcolor ( imulFix (imulFix_simple(b0,a0 ) + b1, b2 ) )
								);

/*
		fix_color_norm(a0);
		dst[i] = fix4_to_sample ( a0,
								imulFix ( imulFix_simple ( r0, a0 ) + r1, r2 ),
								imulFix ( imulFix_simple ( g0, a0 ) + g1, g2 ),
								imulFix ( imulFix_simple ( b0, a0 ) + b1, b2 )
							);
*/
#else
		dst[i] = fix4_to_sample( a0,
								 clampfix_maxcolor (imulFix_simple(r0,a0 ) + r1 ),
								 clampfix_maxcolor (imulFix_simple(g0,a0 ) + g1 ),
								 clampfix_maxcolor (imulFix_simple(b0,a0 ) + b1 )
								);

#endif

#ifdef WRITE_W
			z[i] = line.w[0];
#endif
		}
		}
#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}break;

	} // zcompare

}


/*!
*/
void CTRTextureBlend::fragment_src_alpha_one_minus_src_alpha()
{
	tVideoSample *dst;

#ifdef USE_ZBUFFER
	fp24 *z;
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
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left(line.x[0]);
	xEnd = fill_convention_right(line.x[1]);

	dx = xEnd - xStart;
	if (dx < 0)
		return;

	// slopes
	const f32 invDeltaX = reciprocal_zero2(line.x[1] - line.x[0]);

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
	dst = (tVideoSample*)RenderTarget->getData() + (line.y * RenderTarget->getDimension().Width) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*)DepthBuffer->lock() + (line.y * RenderTarget->getDimension().Width) + xStart;
#endif


	f32 iw = FIX_POINT_F32_MUL;

	tFixPoint a0, r0, g0, b0;
	tFixPoint     r1, g1, b1;
	tFixPoint a2, r2, g2, b2;

	s32 i;

	switch (depth_func)
	{
	default:
	case ECFN_LESSEQUAL:
		for (i = 0; i <= dx; ++i)
		{
#ifdef CMP_W
			if (line.w[0] >= z[i])
#endif

			{
#ifdef WRITE_W
		//z[i] = line.w[0];
#endif

#ifdef INVERSE_W
				iw = fix_inverse32(line.w[0]);
#endif

				getSample_texture(a0, r0, g0, b0, &IT[0], tofix(line.t[0][0].x, iw), tofix(line.t[0][0].y, iw));
				if (a0 > AlphaRef)
				{
#ifdef IPOL_C0
					vec4_to_fix(a2, r2, g2, b2, line.c[0][0], iw);
					//a0 = imulFix(a0, a2); why is vertex color enabled and not vertex_alpha?
					r0 = imulFix_simple(r0, r2);
					g0 = imulFix_simple(g0, g2);
					b0 = imulFix_simple(b0, b2);
#endif

					color_to_fix(r1, g1, b1, dst[i]);

					fix_color_norm(a0);

					r2 = r1 + imulFix(a0, r0 - r1);
					g2 = g1 + imulFix(a0, g0 - g1);
					b2 = b1 + imulFix(a0, b0 - b1);
					dst[i] = fix4_to_sample(a0, r2, g2, b2);
				}

			}

#ifdef IPOL_W
			line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
			line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
			line.c[0][0] += slopeC[0];
#endif
		}
		break;

	} // zcompare

}


/*!
*/
void CTRTextureBlend::fragment_dst_color_one_minus_dst_alpha ()
{
	tVideoSample *dst;

#ifdef USE_ZBUFFER
	fp24 *z;
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
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left( line.x[0] );
	xEnd = fill_convention_right( line.x[1] );

	dx = xEnd - xStart;

	if ( dx < 0 )
		return;

	// slopes
	const f32 invDeltaX = reciprocal_zero2( line.x[1] - line.x[0] );

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
	subPixel = ( (f32) xStart ) - line.x[0];
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
	dst = (tVideoSample*)RenderTarget->getData() + ( line.y * RenderTarget->getDimension().Width ) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*) DepthBuffer->lock() + ( line.y * RenderTarget->getDimension().Width ) + xStart;
#endif


	f32 iw = FIX_POINT_F32_MUL;

	tFixPoint r0, g0, b0;
	tFixPoint a1, r1, g1, b1;
#ifdef IPOL_C0
	tFixPoint r2, g2, b2;
#endif
	s32 i;

	switch (depth_func)
	{
	default:
	case ECFN_LESSEQUAL:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] >= z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		color_to_fix1 ( a1, r1, g1, b1, dst[i] );
#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		a1 = FIX_POINT_ONE - a1;
		dst[i] = fix_to_sample( imulFix ( imulFix ( r1, r0 + a1 ), r2 ),
								imulFix ( imulFix ( g1, g0 + a1 ), g2 ),
								imulFix ( imulFix ( b1, b0 + a1 ), b2 )
							);
#else
		dst[i] = fix_to_sample( imulFix ( r1, r0 + a1 ),
								imulFix ( g1, g0 + a1 ),
								imulFix ( b1, b0 + a1 )
							);

#endif

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}
	break;

	case 2:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] == z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif
		getSample_texture ( r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		color_to_fix1 ( a1, r1, g1, b1, dst[i] );

#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		a1 = FIX_POINT_ONE - a1;
		dst[i] = fix_to_sample( imulFix ( imulFix ( r1, r0 + a1 ), r2 ),
								imulFix ( imulFix ( g1, g0 + a1 ), g2 ),
								imulFix ( imulFix ( b1, b0 + a1 ), b2 )
							);
#else
		dst[i] = fix_to_sample( imulFix ( r1, r0 + a1 ),
								imulFix ( g1, g0 + a1 ),
								imulFix ( b1, b0 + a1 )
							);

#endif

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}break;
	} // zcompare

}

/*!
*/
void CTRTextureBlend::fragment_dst_color_zero ()
{
	tVideoSample *dst;

#ifdef USE_ZBUFFER
	fp24 *z;
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
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left( line.x[0] );
	xEnd = fill_convention_right( line.x[1] );

	dx = xEnd - xStart;

	if ( dx < 0 )
		return;

	// slopes
	const f32 invDeltaX = reciprocal_zero2( line.x[1] - line.x[0] );

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
	subPixel = ( (f32) xStart ) - line.x[0];
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
	dst = (tVideoSample*)RenderTarget->getData() + ( line.y * RenderTarget->getDimension().Width ) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*) DepthBuffer->lock() + ( line.y * RenderTarget->getDimension().Width ) + xStart;
#endif


	f32 iw = FIX_POINT_F32_MUL;

	tFixPoint r0, g0, b0;
	tFixPoint r1, g1, b1;
#ifdef IPOL_C0
	tFixPoint r2, g2, b2;
#endif
	s32 i;

	switch (depth_func)
	{
	default:
	case ECFN_LESSEQUAL:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] >= z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		color_to_fix1 ( r1, g1, b1, dst[i] );

#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		dst[i] = fix_to_sample( imulFix (imulFix_simple( r0, r1 ), r2 ),
								imulFix (imulFix_simple( g0, g1 ), g2 ),
								imulFix (imulFix_simple( b0, b1 ), b2 ) );
#else
		dst[i] = fix_to_sample(imulFix_simple( r0, r1 ),
			imulFix_simple( g0, g1 ),
			imulFix_simple( b0, b1 )
							);

#endif

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}
	break;

	case 2:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] == z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif
		getSample_texture ( r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		color_to_fix1 ( r1, g1, b1, dst[i] );

#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		dst[i] = fix_to_sample( imulFix (imulFix_simple( r0, r1 ), r2 ),
								imulFix (imulFix_simple( g0, g1 ), g2 ),
								imulFix (imulFix_simple( b0, b1 ), b2 )
							);
#else
		dst[i] = fix_to_sample(imulFix_simple( r0, r1 ),
			imulFix_simple( g0, g1 ),
			imulFix_simple( b0, b1 )
							);

#endif

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}break;
	} // zcompare

}

/*!
*/
void CTRTextureBlend::fragment_dst_color_one ()
{
	tVideoSample *dst;

#ifdef USE_ZBUFFER
	fp24 *z;
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
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left( line.x[0] );
	xEnd = fill_convention_right( line.x[1] );

	dx = xEnd - xStart;

	if ( dx < 0 )
		return;

	// slopes
	const f32 invDeltaX = reciprocal_zero2( line.x[1] - line.x[0] );

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
	subPixel = ( (f32) xStart ) - line.x[0];
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
	dst = (tVideoSample*)RenderTarget->getData() + ( line.y * RenderTarget->getDimension().Width ) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*) DepthBuffer->lock() + ( line.y * RenderTarget->getDimension().Width ) + xStart;
#endif


	f32 iw = FIX_POINT_F32_MUL;

	tFixPoint r0, g0, b0;
	tFixPoint r1, g1, b1;
#ifdef IPOL_C0
	tFixPoint r2, g2, b2;
#endif
	s32 i;

	switch (depth_func)
	{
	default:
	case ECFN_LESSEQUAL:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] >= z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		color_to_fix ( r1, g1, b1, dst[i] );
#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		dst[i] = fix_to_sample( clampfix_maxcolor ( imulFix_tex1 ( r0, r1 ) + r1 ),
								clampfix_maxcolor ( imulFix_tex1 ( g0, g1 ) + g1 ),
								clampfix_maxcolor ( imulFix_tex1 ( b0, b1 ) + b1 )
							);

#else
		dst[i] = fix_to_sample( clampfix_maxcolor ( imulFix_tex1 ( r0, r1 ) + r1 ),
								clampfix_maxcolor ( imulFix_tex1 ( g0, g1 ) + g1 ),
								clampfix_maxcolor ( imulFix_tex1 ( b0, b1 ) + b1 )
							);

#endif

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}
	break;

	case 2:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] == z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif
		getSample_texture ( r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		color_to_fix ( r1, g1, b1, dst[i] );

#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		dst[i] = fix_to_sample( clampfix_maxcolor ( imulFix_tex1 ( r0, r1 ) + r1 ),
								clampfix_maxcolor ( imulFix_tex1 ( g0, g1 ) + g1 ),
								clampfix_maxcolor ( imulFix_tex1 ( b0, b1 ) + b1 )
							);

#else
		dst[i] = fix_to_sample( clampfix_maxcolor ( imulFix_tex1 ( r0, r1 ) + r1 ),
								clampfix_maxcolor ( imulFix_tex1 ( g0, g1 ) + g1 ),
								clampfix_maxcolor ( imulFix_tex1 ( b0, b1 ) + b1 )
							);

#endif


		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}break;
	} // zcompare

}

/*!
*/
void CTRTextureBlend::fragment_zero_one_minus_scr_color ()
{
	tVideoSample *dst;

#ifdef USE_ZBUFFER
	fp24 *z;
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
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS];
#endif
#ifdef IPOL_T0
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES];
#endif

	// apply top-left fill-convention, left
	xStart = fill_convention_left( line.x[0] );
	xEnd = fill_convention_right( line.x[1] );

	dx = xEnd - xStart;

	if ( dx < 0 )
		return;

	// slopes
	const f32 invDeltaX = reciprocal_zero2( line.x[1] - line.x[0] );

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
	subPixel = ( (f32) xStart ) - line.x[0];
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
	dst = (tVideoSample*)RenderTarget->getData() + ( line.y * RenderTarget->getDimension().Width ) + xStart;

#ifdef USE_ZBUFFER
	z = (fp24*) DepthBuffer->lock() + ( line.y * RenderTarget->getDimension().Width ) + xStart;
#endif


	f32 iw = FIX_POINT_F32_MUL;

	tFixPoint r0, g0, b0;
	tFixPoint r1, g1, b1;
#ifdef IPOL_C0
	tFixPoint r2, g2, b2;
#endif
	s32 i;

	switch (depth_func)
	{
	default:
	case ECFN_LESSEQUAL:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] >= z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif

		getSample_texture ( r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		color_to_fix1 ( r1, g1, b1, dst[i] );
#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		dst[i] = fix_to_sample(imulFix( FIX_POINT_ONE - r0, r1 ),
			imulFix( FIX_POINT_ONE - g0, g1 ),
			imulFix( FIX_POINT_ONE - b0, b1 )
							);

#else
		dst[i] = fix_to_sample(imulFix( FIX_POINT_ONE - r0, r1 ),
			imulFix( FIX_POINT_ONE - g0, g1 ),
			imulFix( FIX_POINT_ONE - b0, b1 )
							);

#endif

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}
	break;

	case 2:
	for ( i = 0; i <= dx; ++i )
	{
#ifdef CMP_W
		if ( line.w[0] == z[i] )
#endif

		{

#ifdef WRITE_W
			z[i] = line.w[0];
#endif

#ifdef INVERSE_W
		iw = fix_inverse32 ( line.w[0] );
#endif
		getSample_texture ( r0, g0, b0, IT + 0, tofix ( line.t[0][0].x,iw),tofix ( line.t[0][0].y,iw) );
		color_to_fix1 ( r1, g1, b1, dst[i] );
#ifdef IPOL_C0
		vec4_to_fix( r2, g2, b2, line.c[0][0],iw );

		dst[i] = fix_to_sample(imulFix( FIX_POINT_ONE - r0, r1 ),
			imulFix( FIX_POINT_ONE - g0, g1 ),
			imulFix( FIX_POINT_ONE - b0, b1 )
							);

#else
		dst[i] = fix_to_sample(imulFix( FIX_POINT_ONE - r0, r1 ),
			imulFix( FIX_POINT_ONE - g0, g1 ),
			imulFix( FIX_POINT_ONE - b0, b1 )
							);

#endif

		}

#ifdef IPOL_W
		line.w[0] += slopeW;
#endif
#ifdef IPOL_T0
		line.t[0][0] += slopeT[0];
#endif
#ifdef IPOL_C0
		line.c[0][0] += slopeC[0];
#endif
	}break;
	} // zcompare

}



void CTRTextureBlend::drawTriangle(const s4DVertex* burning_restrict a, const s4DVertex* burning_restrict b, const s4DVertex* burning_restrict c)
{
	if ( 0 == fragmentShader )
		return;

	// sort on height, y
	if ( F32_A_GREATER_B ( a->Pos.y , b->Pos.y ) ) swapVertexPointer(&a, &b);
	if ( F32_A_GREATER_B ( b->Pos.y , c->Pos.y ) ) swapVertexPointer(&b, &c);
	if ( F32_A_GREATER_B ( a->Pos.y , b->Pos.y ) ) swapVertexPointer(&a, &b);

	const f32 ca = c->Pos.y - a->Pos.y;
	const f32 ba = b->Pos.y - a->Pos.y;
	const f32 cb = c->Pos.y - b->Pos.y;
	// calculate delta y of the edges
	scan.invDeltaY[0] = reciprocal_zero( ca );
	scan.invDeltaY[1] = reciprocal_zero( ba );
	scan.invDeltaY[2] = reciprocal_zero( cb );

	if ( F32_LOWER_EQUAL_0 ( scan.invDeltaY[0] ) )
		return;

	// find if the major edge is left or right aligned
	f32 temp[4];

	temp[0] = a->Pos.x - c->Pos.x;
	temp[1] = -ca;
	temp[2] = b->Pos.x - a->Pos.x;
	temp[3] = ba;

	scan.left = ( temp[0] * temp[3] - temp[1] * temp[2] ) > 0.f ? 0 : 1;
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
	if ( (f32) 0.0 != scan.invDeltaY[1]  )
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
		yStart = fill_convention_left( a->Pos.y );
		yEnd = fill_convention_right( b->Pos.y );

#ifdef SUBTEXEL
		subPixel = ( (f32) yStart ) - a->Pos.y;

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
		for( line.y = yStart; line.y <= yEnd; ++line.y)
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
	if ( (f32) 0.0 != scan.invDeltaY[2] )
	{
		// advance to middle point
		if( (f32) 0.0 != scan.invDeltaY[1] )
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
		yStart = fill_convention_left( b->Pos.y );
		yEnd = fill_convention_right( c->Pos.y );

#ifdef SUBTEXEL

		subPixel = ( (f32) yStart ) - b->Pos.y;

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
		for( line.y = yStart; line.y <= yEnd; ++line.y)
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



} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_

namespace irr
{
namespace video
{

//! creates a flat triangle renderer
IBurningShader* createTRTextureBlend(CBurningVideoDriver* driver)
{
	// EMT_ONETEXTURE_BLEND
	#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_
	return new CTRTextureBlend(driver);
	#else
	return 0;
	#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_
}


} // end namespace video
} // end namespace irr


