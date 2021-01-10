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

//#define IPOL_C0
#define IPOL_T0
#define IPOL_T1

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

class CTRTextureLightMap2_M1 : public IBurningShader
{
public:

	//! constructor
	CTRTextureLightMap2_M1(CBurningVideoDriver* driver);

	//! draws an indexed triangle list
	virtual void drawTriangle(const s4DVertex* burning_restrict a, const s4DVertex* burning_restrict b, const s4DVertex* burning_restrict c) _IRR_OVERRIDE_;


private:
	void scanline_bilinear2 ();

};

//! constructor
CTRTextureLightMap2_M1::CTRTextureLightMap2_M1(CBurningVideoDriver* driver)
: IBurningShader(driver)
{
	#ifdef _DEBUG
	setDebugName("CTRTextureLightMap2_M1");
	#endif
}

/*!
*/
REALINLINE void CTRTextureLightMap2_M1::scanline_bilinear2 ()
{
	tVideoSample *dst;
	fp24 *z;

	s32 xStart;
	s32 xEnd;
	s32 dx;
	s32 i;


	// apply top-left fill-convention, left
	xStart = fill_convention_left( line.x[0] );
	xEnd = fill_convention_right( line.x[1] );

	dx = xEnd - xStart;
	if ( dx < 0 )
		return;

	// slopes
	const f32 invDeltaX = reciprocal_zero2( line.x[1] - line.x[0] );

	// search z-buffer for first not occulled pixel
	z = (fp24*) DepthBuffer->lock() + ( line.y * RenderTarget->getDimension().Width ) + xStart;

	// subTexel
	const f32 subPixel = ( (f32) xStart ) - line.x[0];

#ifdef IPOL_W
	const f32 b = (line.w[1] - line.w[0]) * invDeltaX;
	f32 a = line.w[0] + ( b * subPixel );

	i = 0;

	while ( a <= z[i] )
	{
		a += b;

		i += 1;
		if ( i > dx )
			return;
	}

	// lazy setup rest of scanline

	line.w[0] = a;
	line.w[1] = b;
#else
	const f32 b = (line.z[1] - line.z[0]) * invDeltaX;
	f32 a = line.z[0] + ( b * subPixel );

	i = 0;

	while ( a > z[i] )
	{
		a += b;

		i += 1;
		if ( i > dx )
			return;
	}

	// lazy setup rest of scanline

	line.z[0] = a;
	line.z[1] = b;
#endif
	SOFTWARE_DRIVER_2_CLIPCHECK;
	dst = (tVideoSample*)RenderTarget->getData() + ( line.y * RenderTarget->getDimension().Width ) + xStart;

	a = (f32) i + subPixel;

	line.t[0][1] = (line.t[0][1] - line.t[0][0]) * invDeltaX;
	line.t[1][1] = (line.t[1][1] - line.t[1][0]) * invDeltaX;

	line.t[0][0] += line.t[0][1] * a;
	line.t[1][0] += line.t[1][1] * a;


#ifdef BURNINGVIDEO_RENDERER_FAST
	u32 dIndex = ( line.y & 3 ) << 2;

	tFixPoint r0, g0, b0;
	tFixPoint r1, g1, b1;

#else
	//
	tFixPoint r0, g0, b0;
	tFixPoint r1, g1, b1;
#endif


	for ( ;i <= dx; i++ )
	{
#ifdef IPOL_W
		if ( line.w[0] >= z[i] )
		{
			z[i] = line.w[0];
#else
		if ( line.z[0] < z[i] )
		{
			z[i] = line.z[0];
#endif

#ifdef SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT
			f32 inversew = fix_inverse32 ( line.w[0] );
#else
			f32 inversew = FIX_POINT_F32_MUL;
#endif



#ifdef BURNINGVIDEO_RENDERER_FAST

			const tFixPointu d = dithermask [ dIndex | ( i ) & 3 ];

			getSample_texture ( r0, g0, b0, &IT[0], d + tofix ( line.t[0][0].x,inversew), d + tofix ( line.t[0][0].y,inversew) );
			getSample_texture ( r1, g1, b1, &IT[1], d + tofix ( line.t[1][0].x,inversew), d + tofix ( line.t[1][0].y,inversew) );
#else
			getSample_texture ( r0, g0, b0, &IT[0], tofix ( line.t[0][0].x,inversew), tofix ( line.t[0][0].y,inversew) );
			getSample_texture ( r1, g1, b1, &IT[1], tofix ( line.t[1][0].x,inversew), tofix ( line.t[1][0].y,inversew) );

#endif

			dst[i] = fix_to_sample( imulFix_tex1 ( r0, r1 ),
									imulFix_tex1 ( g0, g1 ),
									imulFix_tex1 ( b0, b1 )
								);
		}

#ifdef IPOL_W
		line.w[0] += line.w[1];
#else
		line.z[0] += line.z[1];
#endif
		line.t[0][0] += line.t[0][1];
		line.t[1][0] += line.t[1][1];
	}

}


void CTRTextureLightMap2_M1::drawTriangle(const s4DVertex* burning_restrict a, const s4DVertex* burning_restrict b, const s4DVertex* burning_restrict c)
{

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

	if ( F32_LOWER_EQUAL_0 ( scan.invDeltaY[0] )  )
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
	scan.slopeC[0] = (c->Color[0] - a->Color[0]) * scan.invDeltaY[0];
	scan.c[0] = a->Color[0];
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
	if ( F32_GREATER_0 ( scan.invDeltaY[1] ) )
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
		scan.slopeC[1] = (b->Color[0] - a->Color[0]) * scan.invDeltaY[1];
		scan.c[1] = a->Color[0];
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
		scan.c[0] += scan.slopeC[0] * subPixel;
		scan.c[1] += scan.slopeC[1] * subPixel;
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
			line.c[scan.left] = scan.c[0];
			line.c[scan.right] = scan.c[1];
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
			scanline_bilinear2 ();

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
			scan.c[0] += scan.slopeC[0];
			scan.c[1] += scan.slopeC[1];
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
	if ( F32_GREATER_0 ( scan.invDeltaY[2] ) )
	{
		// advance to middle point
		if ( F32_GREATER_0 ( scan.invDeltaY[1] )  )
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
			scan.c[0] = a->Color[0] + scan.slopeC[0] * temp[0];
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
		scan.slopeC[1] = (c->Color[0] - b->Color[0]) * scan.invDeltaY[2];
		scan.c[1] = b->Color[0];
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
		scan.c[0] += scan.slopeC[0] * subPixel;
		scan.c[1] += scan.slopeC[1] * subPixel;
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
			line.c[scan.left] = scan.c[0];
			line.c[scan.right] = scan.c[1];
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
			scanline_bilinear2 ();

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
			scan.c[0] += scan.slopeC[0];
			scan.c[1] += scan.slopeC[1];
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
IBurningShader* createTriangleRendererTextureLightMap2_M1(CBurningVideoDriver* driver)
{
	//ETR_TEXTURE_GOURAUD_LIGHTMAP_M1
	#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_
	return new CTRTextureLightMap2_M1(driver);
	#else
	return 0;
	#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_
}


} // end namespace video
} // end namespace irr



