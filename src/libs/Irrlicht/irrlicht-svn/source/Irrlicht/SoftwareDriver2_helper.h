// Copyright (C) 2002-2022 Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

/*
History:
- changed behavior for log2 textures ( replaced multiplies by shift )
*/

#ifndef S_VIDEO_2_SOFTWARE_HELPER_H_INCLUDED
#define S_VIDEO_2_SOFTWARE_HELPER_H_INCLUDED

#include "SoftwareDriver2_compile_config.h"
#include "CSoftwareTexture2.h"
#include "SMaterial.h"

burning_namespace_start

// supporting different packed pixel needs many defines...

#if defined(SOFTWARE_DRIVER_2_32BIT)
// Bit-width storage type for one render target/framebuffer sample
// The actual channel layout is platform-dependent; masks and shifts are used when accessing channels.
typedef u32	tRenderTargetColorSample;
typedef u8	tStencilSample;

#define	MASK_A	0xFF000000
#define	MASK_R	0x00FF0000
#define	MASK_G	0x0000FF00
#define	MASK_B	0x000000FF

#define	SHIFT_A	(unsigned)24
#define	SHIFT_R	(unsigned)16
#define	SHIFT_G	(unsigned)8
#define	SHIFT_B	(unsigned)0

#define	COLOR_MAX					0xFF
#define	COLOR_MAX_LOG2				8
#define	COLOR_MAX_SAMPLE    		0xFFFFFFFF

//log2 of pixel-bytewidth step multiplier
#define BV_TEXTURE_LOG2_STEP	(unsigned)2
#else
typedef u16	tRenderTargetColorSample;
typedef u8	tStencilSample;

#define	MASK_A	0x8000
#define	MASK_R	0x7C00
#define	MASK_G	0x03E0
#define	MASK_B	0x001F

#define	SHIFT_A	(unsigned)15
#define	SHIFT_R	(unsigned)10
#define	SHIFT_G	(unsigned)5
#define	SHIFT_B	(unsigned)0

#define	COLOR_MAX					0x1F
#define	COLOR_MAX_LOG2				5
#define	COLOR_MAX_SAMPLE			0xFFFF
#define BV_TEXTURE_LOG2_STEP	(unsigned)1

#endif

// Bit-width storage type for one texture sample.
// Textures are stored in the same width as the render target
typedef tRenderTargetColorSample tTextureSample;


// ----------------------- Generic ----------------------------------
//! align_next - align to next upper 2^n
#define align_next(num,to) (((num) + (to-1)) & (~(to-1)))

//! a more useful memset for pixel. dest must be aligned at least to 4 byte
// (standard memset only works with 8-bit values)
static inline void memset32(void* dest, const u32 value, size_t bytesize)
{
	u32* d = (u32*)dest;

	size_t i;

	// loops unrolled to reduce the number of increments by factor ~8.
	i = bytesize >> (2 + 3);
	while (i)
	{
		d[0] = value;
		d[1] = value;
		d[2] = value;
		d[3] = value;

		d[4] = value;
		d[5] = value;
		d[6] = value;
		d[7] = value;

		d += 8;
		i -= 1;
	}

	i = (bytesize >> 2) & 7;
	while (i)
	{
		d[0] = value;
		d += 1;
		i -= 1;
	}
}


//! memset interleaved
static inline void memset32_interlaced(void* dest, const u32 value, size_t pitch, u32 height, const interlaced_control interlaced)
{
	if (interlaced.m.bypass) return memset32(dest, value, pitch * height);

	u8* dst = (u8*)dest;
	interlace_scanline_data line;
	struct _sBurningRenderTarget
	{
		interlaced_control interlaced;
	};
	_sBurningRenderTarget RenderTarget = { interlaced };

	for (line.y = 0; line.y < height; line.y += SOFTWARE_DRIVER_2_STEP_Y)
	{
		if_interlace_scanline_active memset32(dst, value, pitch);
		dst += pitch;
	}
}

// byte-align structures
#include "irrpack.h"

//IEEE Standard for Floating - Point Arithmetic(IEEE 754)
typedef union {
	float f;
	unsigned int u;
	struct { unsigned int frac : 23; unsigned exp : 8; unsigned int sign : 1; } fields;
	struct { unsigned int frac_exp : 31; } abs;
} PACK_STRUCT ieee754;

// Default alignment
#include "irrunpack.h"

// 0.5f as integer
#define ieee754_zero_dot_5	0x3f000000
#define ieee754_one			0x3f800000
#define ieee754_two			0x40000000

#if 0
// integer log2 of a float ieee 754. [not used anymore]
static inline s32 s32_log2_f32(f32 f)
{
	//u32 x = IR ( f ); return ((x & 0x7F800000) >> 23) - 127;
	ieee754 _log2;
	_log2.f = f;
	return _log2.fields.exp ? _log2.fields.exp - 127 : 10000000; /*denormal very high number*/
}
#endif

// integer log2 of an integer. returning 0 as denormal
static inline s32 s32_log2_s32(u32 in)
{
	s32 ret = 0;
	while (in > 1)
	{
		in >>= 1;
		ret++;
	}
	return ret;
	//return s32_log2_f32( (f32) x);
	//ieee754 _log2;_log2.f = (f32) in; return _log2.fields.exp - 127;
}

#if 0
static inline s32 s32_abs(s32 x)
{
	s32 b = x >> 31;
	return (x ^ b) - b;
}


//! conditional set based on mask and arithmetic shift
REALINLINE u32 if_mask_a_else_b(const u32 mask, const u32 a, const u32 b)
{
	return (mask & (a ^ b)) ^ b;
}
#endif

// ------------------ Video---------------------------------------

// from CBlit.h
/*
	return alpha in [0;256] Granularity from 32-Bit ARGB
	add highbit alpha ( alpha > 127 ? + 1 )
*/
static inline u32 _extractAlpha(const u32 c)
{
	return (c >> 24) + (c >> 31);
}


/*!
	Pixel = dest * ( 1 - alpha ) + source * alpha
	alpha [0;256]
*/
REALINLINE u32 _PixelBlend32(const u32 c2, const u32 c1, const u32 alpha)
{
	u32 srcRB = c1 & 0x00FF00FF;
	u32 srcXG = c1 & 0x0000FF00;

	u32 dstRB = c2 & 0x00FF00FF;
	u32 dstXG = c2 & 0x0000FF00;


	u32 rb = srcRB - dstRB;
	u32 xg = srcXG - dstXG;

	rb *= alpha;
	xg *= alpha;
	rb >>= 8;
	xg >>= 8;

	rb += dstRB;
	xg += dstXG;

	rb &= 0x00FF00FF;
	xg &= 0x0000FF00;

	return rb | xg;
}

/*!
	Pixel = dest * ( 1 - alpha ) + source * alpha
	alpha [0;32]
*/
inline u16 _PixelBlend16(const u16 c2, const u16 c1, const u16 alpha)
{
	const u16 srcRB = c1 & 0x7C1F;
	const u16 srcXG = c1 & 0x03E0;

	const u16 dstRB = c2 & 0x7C1F;
	const u16 dstXG = c2 & 0x03E0;

	u32 rb = srcRB - dstRB;
	u32 xg = srcXG - dstXG;

	rb *= alpha;
	xg *= alpha;
	rb >>= 5;
	xg >>= 5;

	rb += dstRB;
	xg += dstXG;

	rb &= 0x7C1F;
	xg &= 0x03E0;

	return (u16)(rb | xg);
}

#if 0
/*
	Pixel = c0 * (c1/31). c0 Alpha retain
*/
inline u16 PixelMul16(const u16 c0, const u16 c1)
{
	return (u16)(((((c0 & 0x7C00) * (c1 & 0x7C00)) & 0x3E000000) >> 15) |
		((((c0 & 0x03E0) * (c1 & 0x03E0)) & 0x000F8000) >> 10) |
		((((c0 & 0x001F) * (c1 & 0x001F)) & 0x000003E0) >> 5) |
		(c0 & 0x8000));
}

/*
	Pixel = c0 * (c1/255). c0 Alpha Retain
*/
REALINLINE u32 PixelMul32(const u32 c0, const u32 c1)
{
	return	(c0 & 0xFF000000) |
		((((c0 & 0x00FF0000) >> 12) * ((c1 & 0x00FF0000) >> 12)) & 0x00FF0000) |
		((((c0 & 0x0000FF00) * (c1 & 0x0000FF00)) >> 16) & 0x0000FF00) |
		((((c0 & 0x000000FF) * (c1 & 0x000000FF)) >> 8) & 0x000000FF);
}
#endif

/*
	Pixel = clamp ( c0 + c1, 0, 255 )
*/
REALINLINE u32 PixelAdd32(const u32 c2, const u32 c1)
{
	u32 sum = (c2 & 0x00FFFFFF) + (c1 & 0x00FFFFFF);
	u32 low_bits = (c2 ^ c1) & 0x00010101;
	s32 carries = (sum - low_bits) & 0x01010100;
	u32 modulo = sum - carries;
	u32 clamp = carries - (carries >> 8);
	return modulo | clamp;
}

// 1 - Bit Alpha Blending
inline u16 _PixelBlend16(const u16 c2, const u16 c1)
{
	u16 mask = ((c1 & 0x8000) >> 15) + 0x7fff;
	return (c2 & mask) | (c1 & ~mask);
}


// ------------------ Fix Point ----------------------------------

#if defined(ENV64BIT)
typedef s32 tFixPoint;
typedef u32 tFixPointu;
#else
typedef s32 tFixPoint;
typedef u32 tFixPointu;
#endif

// Fix Point 12 fractional bits (overflow on s32)
#if 0
#define FIX_POINT_PRE			12
#define FIX_POINT_FRACT_MASK	0xFFF
#define FIX_POINT_UNSIGNED_MASK	0x7FFFF000
#define FIX_POINT_ONE			0x1000
#define FIX_POINT_ZERO_DOT_FIVE	0x0800
#define FIX_POINT_F32_MUL		4096.f
#endif

// Fix Point 11 fractional bits (overflow on s32)
#if 0
#define FIX_POINT_PRE			11
#define FIX_POINT_FRACT_MASK	0x7FF
#define FIX_POINT_UNSIGNED_MASK	0xFFFFF800
#define FIX_POINT_ONE			0x800
#define FIX_POINT_ZERO_DOT_FIVE	0x400
#define FIX_POINT_F32_MUL		2048.f
#endif

// Fix Point 10 fractional bits
#if 1
#define FIX_POINT_PRE			10
#define FIX_POINT_FRACT_MASK	0x000003FF
#define FIX_POINT_UNSIGNED_MASK	0x7FFFFC00
#define FIX_POINT_ONE			0x00000400
#define FIX_POINT_ZERO_DOT_FIVE	0x00000200
#define FIX_POINT_F32_MUL		1024.f
#endif

// Fix Point 9 fractional bits
#if 0
#define FIX_POINT_PRE			9
#define FIX_POINT_FRACT_MASK	0x1FF
#define FIX_POINT_UNSIGNED_MASK	0x7FFFFE00
#define FIX_POINT_ONE			0x200
#define FIX_POINT_ZERO_DOT_FIVE	0x100
#define FIX_POINT_F32_MUL		512.f
#endif

// Fix Point 7 fractional bits
#if 0
#define FIX_POINT_PRE			7
#define FIX_POINT_FRACT_MASK	0x7F
#define FIX_POINT_UNSIGNED_MASK	0x7FFFFF80
#define FIX_POINT_ONE			0x80
#define FIX_POINT_ZERO_DOT_FIVE	0x40
#define FIX_POINT_F32_MUL		128.f
#endif

#define	FIX_POINT_COLOR_MAX		( COLOR_MAX << FIX_POINT_PRE )
#define FIX_POINT_EPSILON   1

#define FIX_POINT_COLOR_FLOAT_MIN -0.5f
//#define FIX_POINT_COLOR_FLOAT_MAX (FIX_POINT_F32_MUL- ((FIX_POINT_F32_MUL-0.5.f)/(f32) COLOR_MAX))
#define FIX_POINT_COLOR_MAX_CENTER (COLOR_MAX * FIX_POINT_F32_MUL)



#if   FIX_POINT_PRE == 10 && COLOR_MAX == 255
#define FIX_POINT_HALF_COLOR	0x1FE00
#define FIX_POINT_COLOR_ERROR	4
#define FIX_POINT_BI_COLOR_ERROR	1536 /* 1.5 */

#elif FIX_POINT_PRE == 12 && COLOR_MAX == 255
#define FIX_POINT_HALF_COLOR	0x7F800
#define FIX_POINT_COLOR_ERROR	16
#define FIX_POINT_BI_COLOR_ERROR	6144 /* 1.5 */
#elif FIX_POINT_PRE == 10 && COLOR_MAX == 31
#define FIX_POINT_HALF_COLOR	0x3E00
#define FIX_POINT_COLOR_ERROR	32
#define FIX_POINT_BI_COLOR_ERROR	0 /* 1.5 */
#else
#define FIX_POINT_HALF_COLOR	( (tFixPoint) ( ((f32) COLOR_MAX / 2.f * FIX_POINT_F32_MUL ) ) )
#define FIX_POINT_COLOR_ERROR	(1<<(FIX_POINT_PRE-COLOR_MAX_LOG2))
#define FIX_POINT_BI_COLOR_ERROR ((tFixPoint) ( 1.5f * FIX_POINT_F32_MUL ))
#endif


/*
convert signed integer to fixpoint
*/
inline tFixPoint s32_to_fixPoint(const s32 x)
{
	return x << FIX_POINT_PRE;
}

#if 0
inline tFixPointu u32_to_fixPoint(const u32 x)
{
	return x << FIX_POINT_PRE;
}
#endif

inline u32 fixPointu_to_u32(const tFixPointu x)
{
	return (u32)(x >> FIX_POINT_PRE);
}


// 1/x * FIX_POINT
#define fix_inverse32(x) (FIX_POINT_F32_MUL / (x))
#define fix_inverse32_color(x) ((FIX_POINT_F32_MUL*COLOR_MAX) / (x))


/*
convert float to fixpoint
fast convert (fistp on x86) HAS to be used..
hints: compileflag /QIfist for msvc7. msvc 8.0 has smth different
others should use their favourite assembler..
*/
#if 0
static inline int f_round2(f32 f)
{
	f += (3 << 22);
	return IR(f) - 0x4b400000;
}
#endif

/*
	convert f32 to Fix Point.
	multiply is needed anyway, so scale mulby
*/
/*
REALINLINE tFixPoint tofix0 (const f32 x, const f32 mulby = FIX_POINT_F32_MUL )
{
	return (tFixPoint) (x * mulby);
}
*/
#define tofix(x,y) (tFixPoint)(x * y)


/*
	Fix Point , Fix Point Multiply
*/
/*
REALINLINE tFixPointu imulFixu(const tFixPointu x, const tFixPointu y)
{
	return (x * y) >> (tFixPointu) FIX_POINT_PRE;
}
*/
#define imulFixu(x,y) (((x) * (y)) >> (tFixPointu) FIX_POINT_PRE)


/*
	Fix Point , Fix Point Multiply
*/
REALINLINE tFixPoint imulFix(const tFixPoint x, const tFixPoint y)
{
	return (x * y) >> FIX_POINT_PRE;
}

#define imulFix_simple(x,y) ((x*y)>>FIX_POINT_PRE)
#define imulFix4(x,y) ((x*y)>>(FIX_POINT_PRE-2))

/*
	Multiply x * y * 1 FIX_POINT_COLOR_MAX
*/
REALINLINE tFixPoint imulFix_tex1(const tFixPoint x, const tFixPoint y)
{
#if SOFTWARE_DRIVER_2_TEXTURE_COLOR_FORMAT == ECF_A8R8G8B8
	return (((tFixPointu)x >> 2) * (((tFixPointu)y + FIX_POINT_ONE) >> 2)) >> (tFixPointu)(FIX_POINT_PRE + 4);
#else
	return (x * (y + FIX_POINT_ONE)) >> (FIX_POINT_PRE + 5);
#endif
}

/*
	Multiply x * y * 2
*/
REALINLINE tFixPoint imulFix_tex2(const tFixPoint x, const tFixPoint y)
{
	return (((tFixPointu)x >> 2) * ((tFixPointu)y >> 2)) >> (tFixPointu)(FIX_POINT_PRE + 3);
}

/*
	Multiply x * y * 4 clamp
*/

REALINLINE tFixPoint imulFix_tex4(const tFixPoint x, const tFixPoint y)
{
#if SOFTWARE_DRIVER_2_TEXTURE_COLOR_FORMAT == ECF_A8R8G8B8
	tFixPoint a = (((tFixPointu)x >> 2) * (((tFixPointu)y + FIX_POINT_ONE) >> 2)) >> (tFixPointu)(FIX_POINT_PRE + 2);
#else
	tFixPoint a = (x * (y + FIX_POINT_ONE)) >> (FIX_POINT_PRE + 3);
#endif
	tFixPoint mask = (a - FIX_POINT_COLOR_MAX) >> 31;
	return (a & mask) | (FIX_POINT_COLOR_MAX & ~mask);
}


/*!
	clamp FixPoint to maxcolor in FixPoint, min(a,COLOR_MAX)
*/
REALINLINE tFixPoint clampfix_maxcolor(const tFixPoint a)
{
	tFixPoint c = (a - FIX_POINT_COLOR_MAX) >> 31;
	return (a & c) | (FIX_POINT_COLOR_MAX & ~c);
}


/*!
	clamp FixPoint to 0 in FixPoint, max(a,0)
*/
REALINLINE tFixPoint clampfix_mincolor(const tFixPoint a)
{
	return a - (a & (a >> 31));
}

REALINLINE tFixPoint saturateFix(const tFixPoint a)
{
	return clampfix_mincolor(clampfix_maxcolor(a));
}


#if 0
// rount fixpoint to int
inline s32 roundFix(const tFixPoint x)
{
	return (s32)((x + FIX_POINT_ZERO_DOT_FIVE) >> FIX_POINT_PRE);
}
#endif

// x in [0;1[
#if 0
inline s32 f32_to_23Bits(const f32 x)
{
	f32 y = x + 1.f;
	return IR(y) & 0x7FFFFF;	// last 23 bits
}
#endif

/*!
	fixpoint in [0..Fixpoint_color] to tRenderTargetColorSample xrgb
*/
REALINLINE tRenderTargetColorSample fix_to_sample(const tFixPoint r, const tFixPoint g, const tFixPoint b)
{
	return
		(FIX_POINT_COLOR_MAX & FIX_POINT_COLOR_MAX) << (SHIFT_A - FIX_POINT_PRE) |
		(r & FIX_POINT_COLOR_MAX) << (SHIFT_R - FIX_POINT_PRE) |
		(g & FIX_POINT_COLOR_MAX) >> (FIX_POINT_PRE - SHIFT_G) |
		(b & FIX_POINT_COLOR_MAX) >> (FIX_POINT_PRE - SHIFT_B);
}

/*!
	fixpoint to tRenderTargetColorSample argb
	a in [0;1]
	rgb in [0;255] colormax
*/
REALINLINE tRenderTargetColorSample fix_to_sample(const tFixPoint a, const tFixPoint r, const tFixPoint g, const tFixPoint b)
{
	return
		(a & (FIX_POINT_FRACT_MASK - 1)) << (SHIFT_A - 1) |
		(r & FIX_POINT_COLOR_MAX) << (SHIFT_R - FIX_POINT_PRE) |
		(g & FIX_POINT_COLOR_MAX) >> (FIX_POINT_PRE - SHIFT_G) |
		(b & FIX_POINT_COLOR_MAX) >> (FIX_POINT_PRE - SHIFT_B);
}

// fixpoint in [0;Fixpoint_color] to tRenderTargetColorSample xrgb nearest
REALINLINE tRenderTargetColorSample fix_to_sample_nearest(const tFixPoint r, const tFixPoint g, const tFixPoint b)
{
	return
		(FIX_POINT_COLOR_MAX & FIX_POINT_COLOR_MAX) << (SHIFT_A - FIX_POINT_PRE) |
		((r + FIX_POINT_ZERO_DOT_FIVE) & FIX_POINT_COLOR_MAX) << (SHIFT_R - FIX_POINT_PRE) |
		((g + FIX_POINT_ZERO_DOT_FIVE) & FIX_POINT_COLOR_MAX) >> (FIX_POINT_PRE - SHIFT_G) |
		((b + FIX_POINT_ZERO_DOT_FIVE) & FIX_POINT_COLOR_MAX) >> (FIX_POINT_PRE - SHIFT_B);
}

// fixpoint in [0;Fixpoint_color] to tRenderTargetColorSample argb nearest
REALINLINE tRenderTargetColorSample fix_to_sample_nearest(const tFixPoint a, const tFixPoint r, const tFixPoint g, const tFixPoint b)
{
	return
		((a + FIX_POINT_ZERO_DOT_FIVE) & FIX_POINT_COLOR_MAX) << (SHIFT_A - FIX_POINT_PRE) |
		((r + FIX_POINT_ZERO_DOT_FIVE) & FIX_POINT_COLOR_MAX) << (SHIFT_R - FIX_POINT_PRE) |
		((g + FIX_POINT_ZERO_DOT_FIVE) & FIX_POINT_COLOR_MAX) >> (FIX_POINT_PRE - SHIFT_G) |
		((b + FIX_POINT_ZERO_DOT_FIVE) & FIX_POINT_COLOR_MAX) >> (FIX_POINT_PRE - SHIFT_B);
}


/*!
	return fixpoint from tRenderTargetColorSample granularity FIX_POINT_COLOR_MAX
*/
inline void sample_to_fix(tFixPoint& r, tFixPoint& g, tFixPoint& b, const tRenderTargetColorSample t00)
{
	(tFixPointu&)r = (t00 & MASK_R) >> (SHIFT_R - FIX_POINT_PRE);
	(tFixPointu&)g = (t00 & MASK_G) << (FIX_POINT_PRE - SHIFT_G);
	(tFixPointu&)b = (t00 & MASK_B) << (FIX_POINT_PRE - SHIFT_B);
}

/*!
	return fixpoint from tRenderTargetColorSample granularity FIX_POINT_COLOR_MAX
*/
inline void sample_to_fix(tFixPoint& a, tFixPoint& r, tFixPoint& g, tFixPoint& b, const tRenderTargetColorSample t00)
{
	(tFixPointu&)a = (t00 & MASK_A) >> (SHIFT_A - FIX_POINT_PRE);
	(tFixPointu&)r = (t00 & MASK_R) >> (SHIFT_R - FIX_POINT_PRE);
	(tFixPointu&)g = (t00 & MASK_G) << (FIX_POINT_PRE - SHIFT_G);
	(tFixPointu&)b = (t00 & MASK_B) << (FIX_POINT_PRE - SHIFT_B);
}

/*
	return (v * 1024u + 127u) * 0x8081u >> 23;
	0x8081 is the multiplicative inverse of 255 in 2^23 domain.
*/
inline tFixPointu u8_to_fix10(u32 v)
{
	return (( (v << FIX_POINT_PRE) + (COLOR_MAX/2)) * 0x8081u) >> 23;
}

/*!
	return fixpoint from tRenderTargetColorSample granularity 0..FIX_POINT_ONE
*/
inline void sample_to_fix_one(tFixPoint& r, tFixPoint& g, tFixPoint& b, const tRenderTargetColorSample t00)
{
	(tFixPointu&)r = (t00 & MASK_R) >> (SHIFT_R + COLOR_MAX_LOG2 - FIX_POINT_PRE);
	(tFixPointu&)g = (t00 & MASK_G) >> (SHIFT_G + COLOR_MAX_LOG2 - FIX_POINT_PRE);
	(tFixPointu&)b = (t00 & MASK_B) << (FIX_POINT_PRE - COLOR_MAX_LOG2);

	//0..255 -> 0..256 | c += c >= 0.5 ? 1 : 0
	r += (r & FIX_POINT_ZERO_DOT_FIVE) ? FIX_POINT_COLOR_ERROR : 0;
	g += (g & FIX_POINT_ZERO_DOT_FIVE) ? FIX_POINT_COLOR_ERROR : 0;
	b += (b & FIX_POINT_ZERO_DOT_FIVE) ? FIX_POINT_COLOR_ERROR : 0;

	// branchless rounding >= 0.5
	//r += ((r & FIX_POINT_HALF_COLOR) >> (FIX_POINT_PRE - 1)) & FIX_POINT_COLOR_ERROR;
}

/*!
	return fixpoint from tRenderTargetColorSample granularity 0..FIX_POINT_ONE
*/
inline void sample_to_fix_one(tFixPoint& a, tFixPoint& r, tFixPoint& g, tFixPoint& b, const tRenderTargetColorSample t00)
{
	(tFixPointu&)a = (t00 & MASK_A) >> (SHIFT_A + COLOR_MAX_LOG2 - FIX_POINT_PRE);
	(tFixPointu&)r = (t00 & MASK_R) >> (SHIFT_R + COLOR_MAX_LOG2 - FIX_POINT_PRE);
	(tFixPointu&)g = (t00 & MASK_G) >> (SHIFT_G + COLOR_MAX_LOG2 - FIX_POINT_PRE);
	(tFixPointu&)b = (t00 & MASK_B) << (FIX_POINT_PRE - COLOR_MAX_LOG2);

	//0..255 -> 0..256 | c += c >= 0.5 ? 1 : 0
	a += (a & FIX_POINT_ZERO_DOT_FIVE) ? FIX_POINT_COLOR_ERROR : 0;
	r += (r & FIX_POINT_ZERO_DOT_FIVE) ? FIX_POINT_COLOR_ERROR : 0;
	g += (g & FIX_POINT_ZERO_DOT_FIVE) ? FIX_POINT_COLOR_ERROR : 0;
	b += (b & FIX_POINT_ZERO_DOT_FIVE) ? FIX_POINT_COLOR_ERROR : 0;
}

/*!
	return fixpoint from tRenderTargetColorSample granularity 0..FIX_POINT_ONE
*/
inline void sample_to_fix_one(tFixPoint c[4], const tRenderTargetColorSample t00)
{
	c[0] = (t00 & MASK_A) >> (SHIFT_A + COLOR_MAX_LOG2 - FIX_POINT_PRE);
	c[1] = (t00 & MASK_R) >> (SHIFT_R + COLOR_MAX_LOG2 - FIX_POINT_PRE);
	c[2] = (t00 & MASK_G) >> (SHIFT_G + COLOR_MAX_LOG2 - FIX_POINT_PRE);
	c[3] = (t00 & MASK_B) << (FIX_POINT_PRE - COLOR_MAX_LOG2);

	//0..255 -> 0..256 | c += c >= 0.5 ? 1 : 0
	c[0] += (c[0] & FIX_POINT_ZERO_DOT_FIVE) ? FIX_POINT_COLOR_ERROR : 0;
	c[1] += (c[1] & FIX_POINT_ZERO_DOT_FIVE) ? FIX_POINT_COLOR_ERROR : 0;
	c[2] += (c[2] & FIX_POINT_ZERO_DOT_FIVE) ? FIX_POINT_COLOR_ERROR : 0;
	c[3] += (c[3] & FIX_POINT_ZERO_DOT_FIVE) ? FIX_POINT_COLOR_ERROR : 0;

}


/*!
	return fixpoint from tRenderTargetColorSample granularity FIX_POINT_COLOR_MAX
*/
inline void sample_to_fix(tFixPoint c[4], const tRenderTargetColorSample t00)
{
	c[0] = (t00 & MASK_A) >> (SHIFT_A - FIX_POINT_PRE);
	c[1] = (t00 & MASK_R) >> (SHIFT_R - FIX_POINT_PRE);
	c[2] = (t00 & MASK_G) << (FIX_POINT_PRE - SHIFT_G);
	c[3] = (t00 & MASK_B) << (FIX_POINT_PRE - SHIFT_B);
}


//! ----- FP24 1.23 fix point z-buffer

#if 1
typedef f32 fp24;
#else
struct fp24
{
	u32 v;

	fp24() {}

	fp24(const f32 f)
	{
		f32 y = f + 1.f;
		v = ((u32&)y) & 0x7FFFFF;	// last 23 bits
	}

	void operator=(const f32 f)
	{
		f32 y = f + 1.f;
		v = ((u32&)y) & 0x7FFFFF;	// last 23 bits
	}

	void operator+=(const fp24& other)
	{
		v += other.v;
	}

	operator f32 () const
	{
		f32 r = FR(v);
		return r + 1.f;
	}

};
#endif


// ------------------------ Internal Texture -----------------------------

//derived from Texture used in sampler
struct sInternalTexture
{
	//power-of-two
	const u8* data; //tTextureSample* Texture->lock(miplevel)
	video::CSoftwareTexture2* Texture;

	//pot2 wrapping mask in fixpoint
	u32 pitch;
	u32 pitchlog2;
	tFixPointu textureXMask; // s32_to_fixPoint(dim.Width - 1) & FIX_POINT_UNSIGNED_MASK
	tFixPointu textureYMask; // s32_to_fixPoint(dim.Height - 1) & FIX_POINT_UNSIGNED_MASK

	//s32 lodFactor; // magnify/minify

	void setTextureParam(s32 lodFactor)
	{
		// select mignify and magnify
		// it->lodFactor = lodFactor;

		//only mipmap chain (means positive lodFactor)
		u32 existing_level = Texture->getMipmapLevel(lodFactor);

#if !defined(PATCH_SUPERTUX_8_0_1_with_1_9_0)
		data = (const u8*)Texture->lock(video::ETLM_READ_ONLY, existing_level, 0);
#else
		data = (const u8*)Texture->lock(video::ETLM_READ_ONLY, existing_level);
#endif

		// prepare for optimal fixpoint
		pitch = Texture->getPitch();
		pitchlog2 = s32_log2_s32((u32)pitch);

		const core::dimension2d<u32>& dim = Texture->getSize();
		textureXMask = s32_to_fixPoint(dim.Width - 1) & FIX_POINT_UNSIGNED_MASK;
		textureYMask = s32_to_fixPoint(dim.Height - 1) & FIX_POINT_UNSIGNED_MASK;
	}

	void setFace(video::E_CUBE_SURFACE face)
	{
		const video::CSoftwareTexture2_Bound& b = Texture->getTexBound_index()[face];
		data = b.data;
		pitch = b.pitch;
		pitchlog2 = b.pitchlog2;
		textureXMask = b.textureXMask;
		textureYMask = b.textureYMask;
	}
	
};



// get video sample unfiltered
static inline tRenderTargetColorSample texelFetch(const sInternalTexture* burning_restrict tex, const tFixPointu tx, const tFixPointu ty)
{
	size_t ofs;

	ofs  = ((ty & tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
	ofs |= (tx & tex->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	// texel
	return *((tTextureSample*)((u8*)tex->data + ofs));
}

static inline tRenderTargetColorSample texelFetch_npot(const sInternalTexture* burning_restrict tex, const tFixPointu tx, const tFixPointu ty)
{
	size_t ofs;

	ofs = (ty >> FIX_POINT_PRE) * tex->pitch; // tex->Texture->getPitch();
	ofs += (tx & FIX_POINT_UNSIGNED_MASK) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	// texel
	return *((tTextureSample*)((u8*)tex->data + ofs));
}

// get video sample to fixpoint colormax
inline void sample_nearest_fix(tFixPoint& r, tFixPoint& g, tFixPoint& b,
	const sInternalTexture* t, const tFixPointu tx, const tFixPointu ty
)
{
	size_t ofs;

	ofs = (((ty + FIX_POINT_ZERO_DOT_FIVE) & t->textureYMask) >> FIX_POINT_PRE) << t->pitchlog2;
	ofs |= ((tx + FIX_POINT_ZERO_DOT_FIVE) & t->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	// texel
	tTextureSample t00;
	t00 = *((tTextureSample*)((u8*)t->data + ofs));

	r = (t00 & MASK_R) >> (SHIFT_R - FIX_POINT_PRE);
	g = (t00 & MASK_G) << (FIX_POINT_PRE - SHIFT_G);
	b = (t00 & MASK_B) << (FIX_POINT_PRE - SHIFT_B);

}

// get video sample to fixpoint colormax
inline void sample_nearest_fix(tFixPoint& a, tFixPoint& r, tFixPoint& g, tFixPoint& b,
	const sInternalTexture* t, const tFixPointu tx, const tFixPointu ty
)
{
	size_t ofs;

	ofs = (((ty + FIX_POINT_ZERO_DOT_FIVE) & t->textureYMask) >> FIX_POINT_PRE) << t->pitchlog2;
	ofs |= ((tx + FIX_POINT_ZERO_DOT_FIVE) & t->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	// texel
	tTextureSample t00;
	t00 = *((tTextureSample*)((u8*)t->data + ofs));

	a = (t00 & MASK_A) >> (SHIFT_A - FIX_POINT_PRE);
	r = (t00 & MASK_R) >> (SHIFT_R - FIX_POINT_PRE);
	g = (t00 & MASK_G) << (FIX_POINT_PRE - SHIFT_G);
	b = (t00 & MASK_B) << (FIX_POINT_PRE - SHIFT_B);

}

#if 0
// get video sample to fixpoint
static REALINLINE void sample_nearest_fix(tFixPoint& a,
	const sInternalTexture* t, const tFixPointu tx, const tFixPointu ty)
{
	size_t ofs;

	ofs = (((ty + FIX_POINT_ZERO_DOT_FIVE) & t->textureYMask) >> FIX_POINT_PRE) << t->pitchlog2;
	ofs |= ((tx + FIX_POINT_ZERO_DOT_FIVE) & t->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	// texel
	tTextureSample t00;
	t00 = *((tTextureSample*)((u8*)t->data + ofs));

	a = (t00 & MASK_A) >> (SHIFT_A - FIX_POINT_PRE);
}
#endif

/*
	load a sample from internal texture at position tx,ty to fixpoint
*/
#if defined(SOFTWARE_DRIVER_2_BILINEAR)

// texture2D in fixpoint color range bilinear
void getSample_texture(tFixPoint& r, tFixPoint& g, tFixPoint& b,
	const sInternalTexture* burning_restrict tex, const tFixPointu tx, const tFixPointu ty
);

// texture2D in fixpoint color range bilinear
void getSample_texture(tFixPoint& a, tFixPoint& r, tFixPoint& g, tFixPoint& b,
	const sInternalTexture* burning_restrict tex, const tFixPointu tx, const tFixPointu ty
);

// get Sample bilinear
void getSample_texture(tFixPoint& a,
	const sInternalTexture* burning_restrict tex, const tFixPointu tx, const tFixPointu ty
);

#else // SOFTWARE_DRIVER_2_BILINEAR

// get Sample linear == getSample_fixpoint

static REALINLINE void getSample_texture(tFixPoint& r, tFixPoint& g, tFixPoint& b,
	const sInternalTexture* burning_restrict tex, const tFixPointu tx, const tFixPointu ty
)
{
	size_t ofs;
	ofs = (((ty + FIX_POINT_ZERO_DOT_FIVE) & tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
	ofs += ((tx + FIX_POINT_ZERO_DOT_FIVE) & tex->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	// texel
	const tTextureSample t00 = *((tTextureSample*)((u8*)tex->data + ofs));

	(tFixPointu&)r = (t00 & MASK_R) >> (SHIFT_R - FIX_POINT_PRE);
	(tFixPointu&)g = (t00 & MASK_G) << (FIX_POINT_PRE - SHIFT_G);
	(tFixPointu&)b = (t00 & MASK_B) << (FIX_POINT_PRE - SHIFT_B);
}

static REALINLINE void getSample_texture(tFixPoint& a, tFixPoint& r, tFixPoint& g, tFixPoint& b,
	const sInternalTexture* burning_restrict tex, const tFixPointu tx, const tFixPointu ty
)
{
	size_t ofs;
	ofs = (((ty + FIX_POINT_ZERO_DOT_FIVE) & tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
	ofs += ((tx + FIX_POINT_ZERO_DOT_FIVE) & tex->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	// texel
	const tTextureSample t00 = *((tTextureSample*)((u8*)tex->data + ofs));

	(tFixPointu&)a = (t00 & MASK_A) >> (SHIFT_A - FIX_POINT_PRE);
	fix_alpha_color_max(a);
	(tFixPointu&)r = (t00 & MASK_R) >> (SHIFT_R - FIX_POINT_PRE);
	(tFixPointu&)g = (t00 & MASK_G) << (FIX_POINT_PRE - SHIFT_G);
	(tFixPointu&)b = (t00 & MASK_B) << (FIX_POINT_PRE - SHIFT_B);
}


// get Sample bilinear
static REALINLINE void getSample_texture(tFixPoint& a,
	const sInternalTexture* burning_restrict tex, const tFixPointu tx, const tFixPointu ty
)
{
	size_t ofs;
	ofs = (((ty + FIX_POINT_ZERO_DOT_FIVE) & tex->textureYMask) >> FIX_POINT_PRE) << tex->pitchlog2;
	ofs += ((tx + FIX_POINT_ZERO_DOT_FIVE) & tex->textureXMask) >> (FIX_POINT_PRE - BV_TEXTURE_LOG2_STEP);

	// texel
	const tTextureSample t00 = *((tTextureSample*)((u8*)tex->data + ofs));

	(tFixPointu&)a = (t00 & MASK_A) >> (SHIFT_A - FIX_POINT_PRE);
	fix_alpha_color_max(a);
}

#endif // SOFTWARE_DRIVER_2_BILINEAR



#if 0
// some 1D defines
struct sIntervall
{
	s32 start;
	s32 end;
};

// returning intersection width
inline s32 intervall_intersect_test(const sIntervall& a, const sIntervall& b)
{
	return core::s32_min(a.end, b.end) - core::s32_max(a.start, b.start);
}

#endif

// strings
static inline void tiny_strncpy(char* to, const char* from, const size_t count)
{
	for (size_t r = 0; r < count && (*to = *from) != '\0'; ++from, ++to, ++r);
	*to = '\0';
}

#define tiny_strcpy(a, b) tiny_strncpy(a,b,sizeof(a)-1)


// tiny_isequal = !strncmp(a,b,sizeof(a)-1)
static inline int tiny_isequal(const char* s1, const char* s2, size_t n)
{
	do {
		if (*s1 != *s2++) return 0;
		if (*s1++ == 0)
			break;
	} while (--n != 0);
	return 1;
}

#define tiny_istoken(a, b) tiny_isequal(a,b,sizeof(a)-1) != 0

static inline float clampf01(const float v)
{
	return v < 0.f ? 0.f : v > 1.f ? 1.f : v;
}

//! clamp(value,0,1)
#if 0
static inline float clampfuv(const float v, const float b)
{
	// b = 1.f - (2.f * (1/width))
	return v < b ? b : v > 1.f - b ? 1.f - b : v;
	//return v < b ? b : v > 1.f-b ? 1.f-b : v;
}
#endif


burning_namespace_end

#endif
