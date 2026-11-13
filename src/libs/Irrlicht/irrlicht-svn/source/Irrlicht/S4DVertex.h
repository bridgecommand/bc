// Copyright (C) 2002-2022 Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef S_4D_VERTEX_H_INCLUDED
#define S_4D_VERTEX_H_INCLUDED

#include "SoftwareDriver2_compile_config.h"
#include "SoftwareDriver2_helper.h"
#include "irrAllocator.h"
#include "EPrimitiveTypes.h"
#include "SVertexIndex.h"

burning_namespace_start

struct sVec4;

//! sVec2 used in BurningShader texture coordinates
struct sVec2
{
	union
	{
		struct { f32 x, y; };
		struct { f32 s, t; };
	};

	sVec2() {}

	sVec2(f32 s) : x(s), y(s) {}
	sVec2(f32 _x, f32 _y)
		: x(_x), y(_y) {
	}

	void set(f32 _x, f32 _y)
	{
		x = _x;
		y = _y;
	}

	// f = a * t + b * ( 1 - t )
	void interpolate(const sVec2& burning_restrict ia, const sVec2& burning_restrict ib, const ipoltype it)
	{
		x = (f32)(ib.x + ((ia.x - ib.x) * it));
		y = (f32)(ib.y + ((ia.y - ib.y) * it));
	}

	sVec2 operator-(const sVec2& other) const
	{
		return sVec2(x - other.x, y - other.y);
	}

	sVec2 operator+(const sVec2& other) const
	{
		return sVec2(x + other.x, y + other.y);
	}

	void operator+=(const sVec2& other)
	{
		x += other.x;
		y += other.y;
	}

	sVec2 operator*(const f32 s) const
	{
		return sVec2(x * s, y * s);
	}

	void operator*=(const f32 s)
	{
		x *= s;
		y *= s;
	}

	void operator=(const sVec2& other)
	{
		x = other.x;
		y = other.y;
	}

	// shader
/*
	void operator=(const core::vector2df& other)
	{
		x = other.X;
		y = other.Y;
	}
*/
	sVec2 _st() const
	{
		return sVec2(x, y);
	}
	sVec2& _st()
	{
		return *this;
	}
	void operator=(const sVec4& other);
	f32 length_xy() const
	{
		return sqrtf(x * x + y * y);
	}

	sVec2 operator/(const sVec2& other) const
	{
		return sVec2(x / other.x, y / other.y);
	}

	sVec2 operator-() const
	{
		return sVec2(-x, -y);
	}
};

#include "irrpack.h"

//! sVec2Pack is Irrlicht S3DVertex,S3DVertex2TCoords,S3DVertexTangents Texture Coordinates.
// Start address is not 4 byte aligned
struct sVec2Pack
{
	f32 x, y;
};

//! sVec3Pack used in BurningShader, packed direction
struct sVec3Pack
{
	f32 x, y, z;
	//f32 _can_pack;

	sVec3Pack() {}
	sVec3Pack(f32 _x, f32 _y, f32 _z)
		: x(_x), y(_y), z(_z) {
	}

	// f = a * t + b * ( 1 - t )
	void interpolate(const sVec3Pack& burning_restrict v0, const sVec3Pack& burning_restrict v1, const ipoltype t)
	{
		x = (f32)(v1.x + ((v0.x - v1.x) * t));
		y = (f32)(v1.y + ((v0.y - v1.y) * t));
		z = (f32)(v1.z + ((v0.z - v1.z) * t));
	}

	sVec3Pack operator-(const sVec3Pack& other) const
	{
		return sVec3Pack(x - other.x, y - other.y, z - other.z);
	}

	sVec3Pack operator+(const sVec3Pack& other) const
	{
		return sVec3Pack(x + other.x, y + other.y, z + other.z);
	}

	sVec3Pack operator*(const f32 s) const
	{
		return sVec3Pack(x * s, y * s, z * s);
	}

	void operator+=(const sVec3Pack& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
	}

	void operator=(const sVec3Pack& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}

}  PACK_STRUCT;

#include "irrunpack.h"

typedef s32 tFixPoint_Q0_30;

struct ALIGN(16) sVec4Fix
{
	union
	{
		struct { tFixPoint_Q0_30 x, y, z, w; };
		struct { tFixPoint_Q0_30 r, g, b, a; };
		struct { tFixPoint_Q0_30 s, t, p, q; };
	};

	// scale from fixpoint10 color max to fixpoint one
	void color_max_to_one()
	{
		// 1/255 = round(2^24/255) = 65793. reduce input to fit in 32bit
		//x = (x * 65793u + 0x00800000u) >> 24;
		//y = (y * 65793u + 0x00800000u) >> 24;
		//z = (z * 65793u + 0x00800000u) >> 24;
		//w = (w * 65793u + 0x00800000u) >> 24;

		x = ((x >> 4) * 65793u + 0x00080000u) >> 20;
		y = ((y >> 4) * 65793u + 0x00080000u) >> 20;
		z = ((z >> 4) * 65793u + 0x00080000u) >> 20;
		w = ((w >> 4) * 65793u + 0x00080000u) >> 20;
	}

	void normalize_q22_10_signed() {
		// Q22.10 from fix10 255.0 to Q0.30 1.0

		//int32_t scale_int = 4112;                     // integer part
		//int32_t scale_frac = (int32_t)((1 << 30) / 261120.0 - 4112) * (1 << 16);  // Q16 fractional

		x = (x * 4112);
		y = (y * 4112);
		z = (z * 4112);
		w = (w * 4112);
	}

	// += vec4 * f32
	// imulFix_tex1. vec4 and y in fix10 colorange
	void add_mul_rgb(const sVec4Fix& v, const tFixPoint scale)
	{
		//r += imulFix_tex1(v.r, scale);
		//g += imulFix_tex1(v.g, scale);
		//b += imulFix_tex1(v.b, scale);

		if (scale == FIX_POINT_COLOR_MAX)
		{
			r += v.r;
			g += v.g;
			b += v.b;
		}
		else
		{
#if SOFTWARE_DRIVER_2_TEXTURE_COLOR_FORMAT == ECF_A8R8G8B8
			const tFixPointu s = ((tFixPointu)scale + FIX_POINT_ONE) >> 2;
			r += (((tFixPointu)v.r >> 2) * s) >> (FIX_POINT_PRE + 4);
			g += ((tFixPointu)(v.g >> 2) * s) >> (FIX_POINT_PRE + 4);
			b += ((tFixPointu)(v.b >> 2) * s) >> (FIX_POINT_PRE + 4);
#else
			const tFixPointu s = scale + FIX_POINT_ONE;
			r += (v.r * s) >> (FIX_POINT_PRE + 5);
			g += (v.g * s) >> (FIX_POINT_PRE + 5);
			b += (v.b * s) >> (FIX_POINT_PRE + 5);
#endif
		}
	}

};

//! sVec4 used in Driver,BurningShader, direction/color
struct ALIGN(16) sVec4
{
	union
	{
		struct { f32 x, y, z, w; };
		struct { f32 r, g, b, a; };
		struct { f32 s, t, p, q; };
	};

#if defined(__GNUC__)
	//has one warning i can't find yet
	sVec4(f32 _x = 0.f, f32 _y = 0.f, f32 _z = 0.f, f32 _w = 0.f)
		: x(_x), y(_y), z(_z), w(_w) {
	}
#else
	sVec4() {}
	sVec4(f32 _x)
		: x(_x), y(_x), z(_x), w(_x) {
	}
	sVec4(f32 _x, f32 _y, f32 _z = 0.f, f32 _w = 0.f)
		: x(_x), y(_y), z(_z), w(_w) {
	}

#endif

	// f = a * t + b * ( 1 - t )
	REALINLINE void interpolate(const sVec4 & burning_restrict ia, const sVec4 & burning_restrict ib, const ipoltype it)
	{
		x = (f32)(ib.x + ((ia.x - ib.x) * it));
		y = (f32)(ib.y + ((ia.y - ib.y) * it));
		z = (f32)(ib.z + ((ia.z - ib.z) * it));
		w = (f32)(ib.w + ((ia.w - ib.w) * it));
	}

	sVec4 operator-(const sVec4 & other) const
	{
		return sVec4(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	sVec4 operator+(const sVec4 & other) const
	{
		return sVec4(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	void operator+=(const sVec4 & other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
	}

	sVec4 operator*(const f32 vs) const
	{
		return sVec4(x * vs, y * vs, z * vs, w * vs);
	}

	sVec4 operator*(const sVec4 & other) const
	{
		return sVec4(x * other.x, y * other.y, z * other.z, w * other.w);
	}

	void operator*=(const sVec4 & other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
	}

#if defined(burning_glsl_emu_test)
	sVec4 operator*(const f64 vs) const
	{
		return sVec4((f32)(x * vs), (f32)(y * vs), (f32)(z * vs), (f32)(w * vs));
	}

/*
	sVec4(const sVec4& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}
	sVec4& operator=(const sVec4 & other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
		return *this;
	}
*/
#endif

	//outside shader
	void set(f32 _x, f32 _y, f32 _z, f32 _w)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}


	REALINLINE ipoltype dot_xyzw(const sVec4 & other) const
	{
		return (ipoltype)x * other.x + (ipoltype)y * other.y + (ipoltype)z * other.z + (ipoltype)w * other.w;
	}

	REALINLINE f32 dot_xyz(const sVec4 & other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}

	REALINLINE f32 dot(const irr::core::vector3df & other) const
	{
		return x * other.X + y * other.Y + z * other.Z;
	}

	REALINLINE f32 dot_minus_xyz(const sVec4 & other) const
	{
		return x * -other.x + y * -other.y + z * -other.z;
	}

	void mul_xyz(const f32 vs)
	{
		x *= vs;
		y *= vs;
		z *= vs;
	}

	f32 length_xyz() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	void normalize_dir_xyz()
	{
		//const f32 l = core::reciprocal_squareroot(x * x + y * y + z * z);
		f32 l = x * x + y * y + z * z;
		if (l > 0.00000001f)
		{
			l = 1.f / sqrtf(l);
			x *= l;
			y *= l;
			z *= l;
		}
		else
		{
			x = 0.f;
			y = -1.f;
			z = 0.f;
		}
	}

	void normalize_dir_xyz_zero()
	{
		//const f32 l = core::reciprocal_squareroot(x * x + y * y + z * z);
		f32 l = x * x + y * y + z * z;
		if (l > 0.00000001f)
		{
			l = 1.f / sqrtf(l);
			x *= l;
			y *= l;
			z *= l;
		}
		else
		{
			x = 0.f;
			y = 0.f;
			z = 0.f;
		}
	}



	//unpack sVec3 to aligned during runtime
	sVec4(const sVec3Pack & other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = 0.f;
	}

	void normalize_pack_xyz(sVec3Pack & out, const f32 len, const f32 ofs) const
	{
		//const f32 l = len * core::reciprocal_squareroot ( r * r + g * g + b * b );
		f32 l = x * x + y * y + z * z;

		l = l > 0.00000001f ? len / sqrtf(l) : 0.f;
		out.x = (x * l) + ofs;
		out.y = (y * l) + ofs;
		out.z = (z * l) + ofs;
	}

	//shader suppport
	sVec4(const sVec4 & a, double _w)
	{
		x = a.x;
		y = a.y;
		z = a.z;
		w = (float)_w;
	}
	sVec4(const sVec2 & a)
	{
		x = a.x;
		y = a.y;
		z = 0.f;
		w = 0.f;
	}

	sVec4 _xyz() const
	{
		return sVec4(x, y, z, 0.f);
	}

	//operator f32* () { return &x; }

	void clampf01()
	{
		if (x < 0.f) x = 0.f; else if (x > 1.f) x = 1.f;
		if (y < 0.f) y = 0.f; else if (y > 1.f) y = 1.f;
		if (z < 0.f) z = 0.f; else if (z > 1.f) z = 1.f;
		if (w < 0.f) w = 0.f; else if (w > 1.f) w = 1.f;
	}

	//Color
	void setA8R8G8B8(const u32 argb);

	//explicit sVec4(const tRenderTargetColorSample sample)
	void setTextureSample(const tTextureSample sample)
	{
		// set in [0..1] range
		static const f32 sample_to_one = 1.f / COLOR_MAX;
		r = ((sample & MASK_R) >> (SHIFT_R)) * sample_to_one;
		g = ((sample & MASK_G) >> (SHIFT_G)) * sample_to_one;
		b = ((sample & MASK_B) >> (SHIFT_B)) * sample_to_one;
		a = ((sample & MASK_A) >> (SHIFT_A)) * sample_to_one;
	}

	//explicit sVec4(const sVec4Fix& a)
	void setFix10(const sVec4Fix& fix10)
	{
		// set in [0..1] range
		static const f32 fix10_to_one = 1.f / FIX_POINT_COLOR_MAX;
		x = fix10.x * fix10_to_one;
		y = fix10.y * fix10_to_one;
		z = fix10.z * fix10_to_one;
		w = fix10.w * fix10_to_one;
	}

	//already clamped
#if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader) && 0
	// gl_FragColor = vec4(color, 1.0);
	/*explicit*/ operator tRenderTargetColorSample() const
	{
		return (irr::tRenderTargetColorSample)(
			(irr::core::s32_clamp(0, (s32)floorf(r * COLOR_MAX), COLOR_MAX) << SHIFT_R) |
			(irr::core::s32_clamp(0, (s32)floorf(g * COLOR_MAX), COLOR_MAX) << SHIFT_G) |
			(irr::core::s32_clamp(0, (s32)floorf(b * COLOR_MAX), COLOR_MAX) << SHIFT_B) |
			(irr::core::s32_clamp(0, (s32)floorf(a * COLOR_MAX), COLOR_MAX) << SHIFT_A)
			)
			;
	}
#endif

	tRenderTargetColorSample getRenderTargetSample() const // unclamped
	{
		return (tRenderTargetColorSample)(
			((u32)floorf(r * COLOR_MAX) << SHIFT_R) |
			((u32)floorf(g * COLOR_MAX) << SHIFT_G) |
			((u32)floorf(b * COLOR_MAX) << SHIFT_B) |
			((u32)floorf(a * COLOR_MAX) << SHIFT_A)
			);
	}

	void set(const f32 vs)
	{
		r = vs;
		g = vs;
		b = vs;
		a = vs;
	}

	void setColorf(const video::SColorf & color)
	{
		r = color.r;
		g = color.g;
		b = color.b;
		a = color.a;
	}

	void add_rgb(const sVec4 & other)
	{
		r += other.r;
		g += other.g;
		b += other.b;
	}

	void mad_rgb(const sVec4 & other, const f32 v)
	{
		r += other.r * v;
		g += other.g * v;
		b += other.b * v;
	}

	void mad_rgbv(const sVec4 & v0, const sVec4 & v1)
	{
		r += v0.r * v1.r;
		g += v0.g * v1.g;
		b += v0.b * v1.b;
	}

	//sVec4 is a,r,g,b, alpha pass
#if 0
	void sat_alpha_pass(sVec4 & dest, const u32 argb) const
	{
		dest.a = ((argb & 0xFF000000) >> 24) * (1.f / 255.f);
		dest.r = r <= 1.f ? r : 1.f;
		dest.g = g <= 1.f ? g : 1.f;
		dest.b = b <= 1.f ? b : 1.f;
	}
#endif
	void sat_alpha_pass(sVec4 & dest, const f32 vertex_alpha) const
	{
		dest.a = vertex_alpha;
		dest.r = r <= 1.f ? r : 1.f;
		dest.g = g <= 1.f ? g : 1.f;
		dest.b = b <= 1.f ? b : 1.f;
	}


	void sat_mul_xyz(sVec4 & dest, const sVec4 & v1) const
	{
		f32 v;
		dest.a = 1.f;
		v = r * v1.r;	dest.r = v < 1.f ? v : 1.f;
		v = g * v1.g;	dest.g = v < 1.f ? v : 1.f;
		v = b * v1.b;	dest.b = v < 1.f ? v : 1.f;
	}

	void sat_mul_xyz(sVec3Pack & dest, const sVec4 & v1) const
	{
		f32 v;
		v = r * v1.r;	dest.x = v < 1.f ? v : 1.f;
		v = g * v1.g;	dest.y = v < 1.f ? v : 1.f;
		v = b * v1.b;	dest.z = v < 1.f ? v : 1.f;
	}

	void operator/=(const sVec4 & other)
	{
		x = other.x != 0.f ? x / other.x : 0.f;
		y = other.y != 0.f ? y / other.y : 0.f;
		z = other.z != 0.f ? z / other.z : 0.f;
		w = other.w != 0.f ? w / other.w : 0.f;
	}


};

//shader
inline void irr::video::sVec2::operator=(const sVec4& other)
{
	x = other.x;
	y = other.y;
}

//!during runtime sVec3Pack
typedef sVec4 sVec3Pack_unpack;

typedef sVec4 sVec3Color;

//internal BurningShaderFlag for a Vertex (Attributes)
enum e4DVertexFlag
{
	VERTEX4D_CLIPMASK = 0x0000003F,
	VERTEX4D_CLIP_INSIDE = 0,
	VERTEX4D_CLIP_NEAR = 0x00000001,
	VERTEX4D_CLIP_FAR = 0x00000002,
	VERTEX4D_CLIP_LEFT = 0x00000004,
	VERTEX4D_CLIP_RIGHT = 0x00000008,
	VERTEX4D_CLIP_BOTTOM = 0x00000010,
	VERTEX4D_CLIP_TOP = 0x00000020,
	VERTEX4D_INSIDE = 0x0000003F,

	//VERTEX4D_PROJECTED = 0x00000100,
	//VERTEX4D_VAL_ZERO = 0x00000200,
	//VERTEX4D_VAL_ONE = 0x00000400,

	VERTEX4D_FORMAT_MASK = 0xFFFF0000,

	VERTEX4D_FORMAT_MASK_TEXTURE = 0x000F0000,
	VERTEX4D_FORMAT_TEXTURE_1 = 0x00010000,
	VERTEX4D_FORMAT_TEXTURE_2 = 0x00020000,
	VERTEX4D_FORMAT_TEXTURE_3 = 0x00030000,
	VERTEX4D_FORMAT_TEXTURE_4 = 0x00040000,

	VERTEX4D_FORMAT_MASK_COLOR = 0x00F00000,
	VERTEX4D_FORMAT_COLOR_1 = 0x00100000,
	VERTEX4D_FORMAT_COLOR_2_FOG = 0x00200000,
	VERTEX4D_FORMAT_COLOR_3 = 0x00300000,
	VERTEX4D_FORMAT_COLOR_4 = 0x00400000,

	VERTEX4D_FORMAT_MASK_LIGHT = 0x0F000000,
	VERTEX4D_FORMAT_LIGHT_1 = 0x01000000,
	//VERTEX4D_FORMAT_LIGHT_2    = 0x02000000,

	VERTEX4D_FORMAT_MASK_TANGENT = 0xF0000000,
	VERTEX4D_FORMAT_BUMP_DOT3 = 0x10000000,
	VERTEX4D_FORMAT_PARALLAX = 0x20000000,
	//VERTEX4D_FORMAT_SPECULAR   = 0x20000000,

};

union s4DVertexFlag
{
	struct
	{
		u32 clipmask : 6;
		u32 _u0 : 2;
		u32 projected : 1;
		u32 _u1 : 7;
		u32 tex : 4;
		u32 col : 4;
		u32 light : 4;
		u32 tan : 4;
	};
	u32 raw;
};

//! vertex layout
enum e4DVertexType
{
	E4VT_STANDARD = 0,			// EVT_STANDARD, video::S3DVertex.
	E4VT_2TCOORDS = 1,			// EVT_2TCOORDS, video::S3DVertex2TCoords.
	E4VT_TANGENTS = 2,			// EVT_TANGENTS, video::S3DVertexTangents

	//encode attributes
	E4VT_SHADER = 3,
	E4VT_TANGENTS_PARALLAX = 4,
	E4VT_REFLECTION_MAP = 5,
	E4VT_SHADOW = 6,			// float * 3
	E4VT_NO_TEXTURE = 7,		// runtime if texture missing
	E4VT_LINE = 8,

	E4VT_COUNT
};

enum e4DIndexType
{
	E4IT_16BIT = 1, // EIT_16BIT,
	E4IT_32BIT = 2, // EIT_32BIT,
	E4IT_NONE = 4, //
};

#if defined(BURNINGVIDEO_RENDERER_BEAUTIFUL) || defined(PATCH_SUPERTUX_8_0_1_with_1_9_0)
//currently with precalculated texcoo in pixelspace shared texcoord do not work
#define BURNING_MATERIAL_MAX_SAMPLER 8
#define BURNING_MATERIAL_MAX_TEXTURES 8
#define BURNING_MATERIAL_MAX_COLORS 10
#define BURNING_MATERIAL_MAX_LIGHT_TANGENT 1

//ensure handcrafted sizeof(s4DVertex)
#define sizeof_s4DVertex	256

#else
#define BURNING_MATERIAL_MAX_SAMPLER 2
#define BURNING_MATERIAL_MAX_TEXTURES 2
#ifdef SOFTWARE_DRIVER_2_USE_VERTEX_COLOR
#define BURNING_MATERIAL_MAX_COLORS 1
#else
#define BURNING_MATERIAL_MAX_COLORS 0
#endif
#define BURNING_MATERIAL_MAX_LIGHT_TANGENT 1

//ensure handcrafted sizeof(s4DVertex)
#define sizeof_s4DVertex	64
#endif

// dummy Vertex. used for calculation vertex memory size
struct s4DVertex_proxy
{
	sVec4 Pos;
#if BURNING_MATERIAL_MAX_TEXTURES > 0
	sVec2 Tex[BURNING_MATERIAL_MAX_TEXTURES];
#endif
#if BURNING_MATERIAL_MAX_COLORS > 0
	sVec4 Color[BURNING_MATERIAL_MAX_COLORS];
#endif
#if BURNING_MATERIAL_MAX_LIGHT_TANGENT > 0
	sVec3Pack LightTangent[BURNING_MATERIAL_MAX_LIGHT_TANGENT];
#endif
	u32 flag; // e4DVertexFlag

};


/*!
	Internal BurningVideo Vertex
*/
struct s4DVertex
{
	sVec4 Pos;
#if BURNING_MATERIAL_MAX_TEXTURES > 0
	sVec2 Tex[BURNING_MATERIAL_MAX_TEXTURES];
#endif
#if BURNING_MATERIAL_MAX_COLORS > 0
	sVec4 Color[BURNING_MATERIAL_MAX_COLORS];
#endif
#if BURNING_MATERIAL_MAX_LIGHT_TANGENT > 0
	sVec3Pack LightTangent[BURNING_MATERIAL_MAX_LIGHT_TANGENT];
#endif

	s4DVertexFlag flag; // e4DVertexFlag


#if BURNING_MATERIAL_MAX_COLORS < 1 || BURNING_MATERIAL_MAX_LIGHT_TANGENT < 1
	u8 __align[sizeof_s4DVertex - sizeof(s4DVertex_proxy)];
#endif

	// f = a * t + b * ( 1 - t )
	REALINLINE void interpolate(const s4DVertex& burning_restrict b, const s4DVertex& burning_restrict a, const ipoltype t)
	{
		Pos.interpolate(a.Pos, b.Pos, t);
#if 0
		Tex[0].interpolate(a.Tex[0], b.Tex[0], t);
		Tex[1].interpolate(a.Tex[1], b.Tex[1], t);
		Color[0].interpolate(a.Color[0], b.Color[0], t);
		LightTangent[0].interpolate(a.LightTangent[0], b.LightTangent[0], t);
#endif

		size_t i;
		size_t size;

#if BURNING_MATERIAL_MAX_TEXTURES > 0
		size = (flag.raw & VERTEX4D_FORMAT_MASK_TEXTURE) >> 16;
		for (i = 0; i != size; ++i)
		{
			Tex[i].interpolate(a.Tex[i], b.Tex[i], t);
		}
#endif

#if BURNING_MATERIAL_MAX_COLORS > 0
		size = (flag.raw & VERTEX4D_FORMAT_MASK_COLOR) >> 20;
		for (i = 0; i != size; ++i)
		{
			Color[i].interpolate(a.Color[i], b.Color[i], t);
		}
#endif

#if BURNING_MATERIAL_MAX_LIGHT_TANGENT > 0
		size = (flag.raw & VERTEX4D_FORMAT_MASK_LIGHT) >> 24;
		for (i = 0; i != size; ++i)
		{
			LightTangent[i].interpolate(a.LightTangent[i], b.LightTangent[i], t);
		}
#endif

	}

	REALINLINE void reset_interpolate()
	{
#if 1
#if BURNING_MATERIAL_MAX_TEXTURES > 0
		Tex[0].x = 0.f;
		Tex[0].y = 0.f;
#endif
#if BURNING_MATERIAL_MAX_TEXTURES > 1
		Tex[1].x = 0.f;
		Tex[1].y = 0.f;
#endif
#if BURNING_MATERIAL_MAX_TEXTURES > 2
		Tex[2].x = 0.f;
		Tex[2].y = 0.f;
#endif
#if BURNING_MATERIAL_MAX_TEXTURES > 3
		Tex[3].x = 0.f;
		Tex[3].y = 0.f;
#endif
#endif

#if BURNING_MATERIAL_MAX_COLORS > 0
		Color[0].r = 0.f;
		Color[0].g = 0.f;
		Color[0].b = 0.f;
		Color[0].a = 1.f;
#endif

#if BURNING_MATERIAL_MAX_COLORS > 1
		//specular
		Color[1].r = 0.f;
		Color[1].g = 0.f;
		Color[1].b = 0.f;
		Color[1].a = 1.f;
#endif

#if BURNING_MATERIAL_MAX_COLORS > 2
		Color[2].r = 0.f;
		Color[2].g = 0.f;
		Color[2].b = 0.f;
		Color[2].a = 1.f;
#endif

#if BURNING_MATERIAL_MAX_COLORS > 3
		Color[3].r = 0.f;
		Color[3].g = 0.f;
		Color[3].b = 0.f;
		Color[3].a = 1.f;
#endif

#if BURNING_MATERIAL_MAX_LIGHT_TANGENT > 0
		LightTangent[0].x = 0.f;
		LightTangent[0].y = 0.f;
		LightTangent[0].z = 0.f;
#endif
	}

};

// ----------------- Vertex Cache ---------------------------

// Buffer is used as interleaved pairs of S4DVertex (0 ... ndc, 1 .. dc and projected)
typedef s4DVertex s4DVertexPair;
#define sizeof_s4DVertexPairRel 2
#define s4DVertex_ofs(index) ((index)*sizeof_s4DVertexPairRel)
#define s4DVertex_pro(index) (((index)*sizeof_s4DVertexPairRel) + 1)

struct SAligned4DVertex
{
	SAligned4DVertex()
		:data(0), ElementSize(0), mem(0) {
	}

	virtual ~SAligned4DVertex()
	{
		if (mem)
		{
			delete[] mem;
			mem = 0;
		}
	}

	void resize(size_t element)
	{
		if (element > ElementSize)
		{
			if (mem) delete[] mem;
			size_t byteSize = align_next(element * sizeof_s4DVertex, 4096);
			mem = new u8[byteSize];
		}
		ElementSize = element;
		data = (s4DVertex*)mem;
	}

	s4DVertex* data;	//align to 16 byte
	size_t ElementSize;

private:

	u8* mem;
};

//#define memcpy_s4DVertexPair(dst,src) memcpy(dst,src,sizeof_s4DVertex * 2)
static REALINLINE void memcpy_s4DVertexPair(void* burning_restrict dst, const void* burning_restrict src)
{
	//test alignment -> if already in aligned data
#if 0
	if (((size_t)dst & 0xC) | ((size_t)src & 0xC))
	{
		int g = 1;
	}
#endif

#if defined(ENV64BIT) && (sizeof_s4DVertex * sizeof_s4DVertexPairRel == 128)
	u64* burning_restrict dst64 = (u64*)dst;
	const u64* burning_restrict src64 = (const u64*)src;

	dst64[0] = src64[0];
	dst64[1] = src64[1];
	dst64[2] = src64[2];
	dst64[3] = src64[3];
	dst64[4] = src64[4];
	dst64[5] = src64[5];
	dst64[6] = src64[6];
	dst64[7] = src64[7];

	dst64[8] = src64[8];
	dst64[9] = src64[9];
	dst64[10] = src64[10];
	dst64[11] = src64[11];
	dst64[12] = src64[12];
	dst64[13] = src64[13];
	dst64[14] = src64[14];
	dst64[15] = src64[15];

#elif defined(ENV64BIT) && (sizeof_s4DVertex * sizeof_s4DVertexPairRel == 256)
	u64* burning_restrict dst64 = (u64*)dst;
	const u64* burning_restrict src64 = (const u64*)src;

	dst64[0] = src64[0];
	dst64[1] = src64[1];
	dst64[2] = src64[2];
	dst64[3] = src64[3];
	dst64[4] = src64[4];
	dst64[5] = src64[5];
	dst64[6] = src64[6];
	dst64[7] = src64[7];

	dst64[8] = src64[8];
	dst64[9] = src64[9];
	dst64[10] = src64[10];
	dst64[11] = src64[11];
	dst64[12] = src64[12];
	dst64[13] = src64[13];
	dst64[14] = src64[14];
	dst64[15] = src64[15];

	dst64[16] = src64[16];
	dst64[17] = src64[17];
	dst64[18] = src64[18];
	dst64[19] = src64[19];
	dst64[20] = src64[20];
	dst64[21] = src64[21];
	dst64[22] = src64[22];
	dst64[23] = src64[23];

	dst64[24] = src64[24];
	dst64[25] = src64[25];
	dst64[26] = src64[26];
	dst64[27] = src64[27];
	dst64[28] = src64[28];
	dst64[29] = src64[29];
	dst64[30] = src64[30];
	dst64[31] = src64[31];
#elif defined(ENV64BIT) && (sizeof_s4DVertex * sizeof_s4DVertexPairRel == 512)
	u64* burning_restrict dst64 = (u64*)dst;
	const u64* burning_restrict src64 = (const u64*)src;

	dst64[0] = src64[0];
	dst64[1] = src64[1];
	dst64[2] = src64[2];
	dst64[3] = src64[3];
	dst64[4] = src64[4];
	dst64[5] = src64[5];
	dst64[6] = src64[6];
	dst64[7] = src64[7];

	dst64[8] = src64[8];
	dst64[9] = src64[9];
	dst64[10] = src64[10];
	dst64[11] = src64[11];
	dst64[12] = src64[12];
	dst64[13] = src64[13];
	dst64[14] = src64[14];
	dst64[15] = src64[15];

	dst64[16] = src64[16];
	dst64[17] = src64[17];
	dst64[18] = src64[18];
	dst64[19] = src64[19];
	dst64[20] = src64[20];
	dst64[21] = src64[21];
	dst64[22] = src64[22];
	dst64[23] = src64[23];

	dst64[24] = src64[24];
	dst64[25] = src64[25];
	dst64[26] = src64[26];
	dst64[27] = src64[27];
	dst64[28] = src64[28];
	dst64[29] = src64[29];
	dst64[30] = src64[30];
	dst64[31] = src64[31];

	dst64[32] = src64[32];
	dst64[33] = src64[33];
	dst64[34] = src64[34];
	dst64[35] = src64[35];
	dst64[36] = src64[36];
	dst64[37] = src64[37];
	dst64[38] = src64[38];
	dst64[39] = src64[39];

	dst64[40] = src64[40];
	dst64[41] = src64[41];
	dst64[42] = src64[42];
	dst64[43] = src64[43];
	dst64[44] = src64[44];
	dst64[45] = src64[45];
	dst64[46] = src64[46];
	dst64[47] = src64[47];

	dst64[48] = src64[48];
	dst64[49] = src64[49];
	dst64[50] = src64[50];
	dst64[51] = src64[51];
	dst64[52] = src64[52];
	dst64[53] = src64[53];
	dst64[54] = src64[54];
	dst64[55] = src64[55];

	dst64[56] = src64[56];
	dst64[57] = src64[57];
	dst64[58] = src64[58];
	dst64[59] = src64[59];
	dst64[60] = src64[60];
	dst64[61] = src64[61];
	dst64[62] = src64[62];
	dst64[63] = src64[63];

#else
	u32* dst32 = (u32*)dst;
	const u32* src32 = (const u32*)src;

	size_t len = sizeof_s4DVertex * sizeof_s4DVertexPairRel;
	while (len >= 32)
	{
		*dst32++ = *src32++;
		*dst32++ = *src32++;
		*dst32++ = *src32++;
		*dst32++ = *src32++;
		*dst32++ = *src32++;
		*dst32++ = *src32++;
		*dst32++ = *src32++;
		*dst32++ = *src32++;
		len -= 32;
	}
	/*
		while (len >= 4)
		{
			*dst32++ = *src32++;
			len -= 4;
		}
	*/
#endif
}


//! hold info for different Vertex Types (Attribute mapping)
struct SVSize
{
	s4DVertexFlag Format;		// e4DVertexFlag VERTEX4D_FORMAT_MASK_TEXTURE
	u32 Pitch;		// sizeof Vertex
	u32 TexSize;	// amount Textures
	u32 TexCooSize;	// amount TextureCoordinates
	u32 ColSize;	// amount Color Interpolators
	u32 VaryingSize;
};


typedef size_t tCacheIndex;

// index cache info
struct SCacheInfo
{
	tCacheIndex index;
	tCacheIndex hit;
};

//must at least hold all possible (clipped) vertices of primitive.
#define VERTEXCACHE_ELEMENT	16			
#define VERTEXCACHE_MISS ((tCacheIndex)-1)
struct SVertexShader
{
	SVertexShader() {}
	~SVertexShader() {}

	// Transformed and lite, clipping state
	// + Clipped, Projected
	SAligned4DVertex mem;

	// source
	const void* vertices;
	const void* indices;

	u32 vertexCount;
	u32 indexCount;
	u32 indicesIndex;
	u32 indicesRun;
	u32 indicesPitch;

	// primitives consist of x vertices
	u32 primitiveHasVertex;
	u32 primitiveRun;

	//VertexType
	SVSize vSize[E4VT_COUNT];

	e4DVertexType vType;		//E_VERTEX_TYPE
	scene::E_PRIMITIVE_TYPE pType;		//scene::E_PRIMITIVE_TYPE
	e4DIndexType iType;		//E_INDEX_TYPE iType

	REALINLINE tCacheIndex index(u32 i) const
	{
		tCacheIndex o;
		if (i >= indexCount)
			i = 0;
		switch (iType)
		{
		case E4IT_16BIT: o = ((u16*)indices)[i]; break;
		case E4IT_32BIT: o = ((u32*)indices)[i]; break;
		default: case E4IT_NONE: o = i; break;
		}
		return o;
	}

	inline s4DVertexPair* vertex(const tCacheIndex sourceIndex) const
	{
		for (tCacheIndex i = 0; i < VERTEXCACHE_ELEMENT; i += 2)
		{
			if (info[i].index == sourceIndex)
			{
				//return mem.data + s4DVertex_ofs(i);
				return (s4DVertexPair*)((u8*)mem.data + (sizeof_s4DVertex * sizeof_s4DVertexPairRel) * i);
			}

			if (info[i + 1].index == sourceIndex)
			{
				return (s4DVertexPair*)((u8*)mem.data + (sizeof_s4DVertex * sizeof_s4DVertexPairRel) * (i + 1));
			}

		}
		return mem.data; //error
	}

	void setPrimitiveType(const scene::E_PRIMITIVE_TYPE pType, const u32 primitiveCount);
	void setIndices(const void* indices, const video::E_INDEX_TYPE iType);

	SCacheInfo info[VERTEXCACHE_ELEMENT];
	SCacheInfo info_temp[VERTEXCACHE_ELEMENT];

	void set_info_miss();

	tCacheIndex fillIndex;
	void get_next_index_cacheline();
	void getPrimitive(s4DVertexPair* face[4], CBurningVideoDriver* driver);
};


// swap 2 pointer
REALINLINE void swapVertexPointer(const s4DVertex*& burning_restrict v1, const s4DVertex*& burning_restrict v2)
{
	const s4DVertex* b = v1;
	v1 = v2;
	v2 = b;
}

//#define swapVertexPointer(v1,v2) _swap = v1, v1 = v2, v2 = _swap


// ------------------------ Internal Scanline Rasterizer -----------------------------


enum eBurningCullFlag
{
	CULL_FRONT = 1,
	CULL_BACK = 2,
	CULL_INVISIBLE = 4,	//primitive smaller than a pixel (AreaMinDrawSize)
	CULL_FRONT_AND_BACK = 8,

	CULL_EPSILON_001 = 981668463, /*0.001f*/
	CULL_EPSILON_00001 = 925353388, /* 0.00001f*/
	CULL_EPSILON_01 = 0x3e000000 /*0.125f*/

};

// internal scan convert
typedef size_t edge_index;

struct sScanConvertData
{
	//sorted scanline vertex
	union
	{
		struct {
			const s4DVertex* burning_restrict ea;
			const s4DVertex* burning_restrict eb;
			const s4DVertex* burning_restrict ec;
		};
		const s4DVertex* burning_restrict edge[4];
	};

	ieee754 dc_area;		// magnitude crossproduct (area of parallelogram * 0.5 = triangle screen size, winding)
	f32 _packme;
	size_t left;			// major edge left/right
	size_t right;			// !left

	// top left fill convention y run
	s32 yStart;
	s32 yEnd;

	u32 ipolsize_c;
	u32 ipolsize_t;
	u32 ipolsize_l;
	u32 _packme2;

	f32 invDeltaY[4];	// inverse edge delta for screen space sorted triangle 

	f32 x[2];			// x coordinate
	f32 slopeX[2];		// x slope along edges

#if defined ( SOFTWARE_DRIVER_2_USE_WBUFFER ) || defined ( SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT )
	f32 w[2];			// w coordinate
	f32 slopeW[2];		// w slope along edges
#else
	f32 z[2];			// z coordinate
	fp24 slopeZ[2];		// z slope along edges
#endif

#if BURNING_MATERIAL_MAX_COLORS > 0
	sVec4 c[BURNING_MATERIAL_MAX_COLORS][2];		// color
	sVec4 slopeC[BURNING_MATERIAL_MAX_COLORS][2];	// color slope along edges
#endif

#if BURNING_MATERIAL_MAX_TEXTURES > 0
	sVec2 t[BURNING_MATERIAL_MAX_TEXTURES][2];		// texture
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES][2];	// texture slope along edges
#endif

#if BURNING_MATERIAL_MAX_LIGHT_TANGENT > 0
	sVec3Pack_unpack l[BURNING_MATERIAL_MAX_LIGHT_TANGENT][2];		// Light Tangent
	sVec3Pack_unpack slopeL[BURNING_MATERIAL_MAX_LIGHT_TANGENT][2];	// tangent slope along edges
#endif

#if defined(burning_glsl_emu_test)
	int set_face(s4DVertexPair* face[4], const size_t /*eCullFlag*/ cullFlag)
	{
		edge[0] = face[0] + s4DVertex_pro(0);
		edge[1] = face[1] + s4DVertex_pro(0);
		edge[2] = face[2] + s4DVertex_pro(0);

		//backface
		dc_area.f = (
			(edge[1]->Pos.x - edge[0]->Pos.x) * (edge[2]->Pos.y - edge[0]->Pos.y) -
			(edge[2]->Pos.x - edge[0]->Pos.x) * (edge[1]->Pos.y - edge[0]->Pos.y)
			) /* * 0.5f */;
		size_t sign = dc_area.fields.sign ? CULL_BACK : CULL_FRONT;
		sign |= dc_area.abs.frac_exp < CULL_EPSILON_00001 ? CULL_INVISIBLE : 0;
		if (cullFlag & sign)
			return 1;

		// sort on height, y
		if (F32_A_GREATER_B(edge[0]->Pos.y, edge[1]->Pos.y)) swapVertexPointer(edge[0], edge[1]);
		if (F32_A_GREATER_B(edge[1]->Pos.y, edge[2]->Pos.y)) swapVertexPointer(edge[1], edge[2]);
		if (F32_A_GREATER_B(edge[0]->Pos.y, edge[1]->Pos.y)) swapVertexPointer(edge[0], edge[1]);

		// calculate delta y of the edges
		const f32 ca = edge[2]->Pos.y - edge[0]->Pos.y;
		const f32 ba = edge[1]->Pos.y - edge[0]->Pos.y;
		const f32 cb = edge[2]->Pos.y - edge[1]->Pos.y;
		invDeltaY[0] = fill_step_y(ca);
		invDeltaY[1] = fill_step_y(ba);
		invDeltaY[2] = fill_step_y(cb);

		//if (F32_LOWER_EQUAL_0(invDeltaY[0]))
		//	return 1;

		// find if the major edge is left or right aligned
		left = ((edge[1]->Pos.x - edge[0]->Pos.x) * ca - (edge[2]->Pos.x - edge[0]->Pos.x) * ba) < 0.f ? 1 : 0;
		right = 1 - left;

		//extract amount of interpolaters
		ipolsize_t = (edge[0]->flag.raw & VERTEX4D_FORMAT_MASK_TEXTURE) >> 16;
		ipolsize_c = (edge[0]->flag.raw & VERTEX4D_FORMAT_MASK_COLOR) >> 20;
		ipolsize_l = (edge[0]->flag.raw & VERTEX4D_FORMAT_MASK_LIGHT) >> 24;

		return 0;
	}

	void set_ipol(size_t dir, edge_index a, edge_index b, size_t pol)
	{
		x[dir] = edge[a]->Pos.x;
		w[dir] = edge[a]->Pos.w;
		slopeX[dir] = (edge[b]->Pos.x - edge[a]->Pos.x) * invDeltaY[pol];
		slopeW[dir] = (edge[b]->Pos.w - edge[a]->Pos.w) * invDeltaY[pol];

		u32 i;
		for (i = 0; i != ipolsize_t; ++i)
		{
			t[i][dir] = edge[a]->Tex[i];
			slopeT[i][dir] = (edge[b]->Tex[i] - edge[a]->Tex[i]) * invDeltaY[pol];
		}

		for (i = 0; i != ipolsize_c; ++i)
		{
			c[i][dir] = edge[a]->Color[i];
			slopeC[i][dir] = (edge[b]->Color[i] - edge[a]->Color[i]) * invDeltaY[pol];
		}

		for (i = 0; i != ipolsize_l; ++i)
		{
			l[i][dir] = edge[a]->LightTangent[i];
			slopeL[i][dir] = (edge[b]->LightTangent[i] - edge[a]->LightTangent[i]) * invDeltaY[pol];
		}

	}

	void set_y(edge_index a, edge_index b)
	{
		// apply top-left fill convention, top part
		yStart = fill_convention_top(edge[a]->Pos.y);
		yEnd = fill_convention_down(edge[b]->Pos.y);

#ifdef SOFTWARE_DRIVER_2_SUBTEXEL
		f32 subPixel = ((f32)yStart) - edge[a]->Pos.y;

		// correct to pixel center
		x[0] += slopeX[0] * subPixel;
		x[1] += slopeX[1] * subPixel;

		w[0] += slopeW[0] * subPixel;
		w[1] += slopeW[1] * subPixel;

		u32 i;
		for (i = 0; i != ipolsize_t; ++i)
		{
			t[i][0] += slopeT[i][0] * subPixel;
			t[i][1] += slopeT[i][1] * subPixel;
		}

		for (i = 0; i != ipolsize_c; ++i)
		{
			c[i][0] += slopeC[i][0] * subPixel;
			c[i][1] += slopeC[i][1] * subPixel;
		}

		for (i = 0; i != ipolsize_l; ++i)
		{
			l[i][0] += slopeL[i][0] * subPixel;
			l[i][1] += slopeL[i][1] * subPixel;
		}
#endif
	}

	void ipol_step()
	{
		x[0] += slopeX[0];
		x[1] += slopeX[1];

		w[0] += slopeW[0];
		w[1] += slopeW[1];


		u32 i;
		for (i = 0; i != ipolsize_t; ++i)
		{
			t[i][0] += slopeT[i][0];
			t[i][1] += slopeT[i][1];
		}

		for (i = 0; i != ipolsize_c; ++i)
		{
			c[i][0] += slopeC[i][0];
			c[i][1] += slopeC[i][1];
		}

		for (i = 0; i != ipolsize_l; ++i)
		{
			l[i][0] += slopeL[i][0];
			l[i][1] += slopeL[i][1];
		}

	}

	void advance_midpoint()
	{
		const f32 dy = edge[1]->Pos.y - edge[0]->Pos.y;	// 1.f/scan.invDeltaY[1]

		x[0] = edge[0]->Pos.x + slopeX[0] * dy;
		w[0] = edge[0]->Pos.w + slopeW[0] * dy;

		u32 i;
		for (i = 0; i != ipolsize_t; ++i)
		{
			t[i][0] = edge[0]->Tex[i] + slopeT[i][0] * dy;
		}

		for (i = 0; i != ipolsize_c; ++i)
		{
			c[i][0] = edge[0]->Color[i] + slopeC[i][0] * dy;
		}

		for (i = 0; i != ipolsize_l; ++i)
		{
			l[i][0] = sVec3Pack_unpack(edge[0]->LightTangent[i]) + slopeL[i][0] * dy;
		}

	}
#endif

};

struct sScanConvertDataNext
{
	sVec2 t[BURNING_MATERIAL_MAX_TEXTURES][2];		// texture
	sVec2 slopeT[BURNING_MATERIAL_MAX_TEXTURES][2];	// texture slope along edges
	f32 w[2];			// w coordinate
	f32 slopeW[2];		// w slope along edges

};

// passed to scan Line
struct sScanLineData
{
	s32 y;				// y position of scanline
	u8 _unused_pack[4];
	f32 x[2];			// x start, x end of scanline

#if defined ( SOFTWARE_DRIVER_2_USE_WBUFFER ) || defined ( SOFTWARE_DRIVER_2_PERSPECTIVE_CORRECT )
	f32 w[2];			// w start, w slope of scanline
#else
	f32 z[2];			// z start, z slope of scanline
#endif

#if defined(burning_glsl_emu_test)
	f32 inversew[2];	// 1/w per fragment, dFdx
#endif	

#if BURNING_MATERIAL_MAX_COLORS > 0
	sVec4 c[BURNING_MATERIAL_MAX_COLORS][2];			// color start, color slope of scanline
#endif

#if BURNING_MATERIAL_MAX_TEXTURES > 0
	sVec2 t[BURNING_MATERIAL_MAX_TEXTURES][2];		// texture start, texture slope of scanline
#endif

#if BURNING_MATERIAL_MAX_LIGHT_TANGENT > 0
	sVec3Pack_unpack l[BURNING_MATERIAL_MAX_LIGHT_TANGENT][2];		// Light Tangent start, slope scanline
#endif

	tRenderTargetColorSample* dst;
	fp24* depth;

	s32 x_edgetest;		// slope x
	s32 xStart;
	s32 xEnd;
	s32 xRun;

#if defined(burning_glsl_emu_test)
	int set(const sScanConvertData& scan)
	{

		x[0] = scan.x[scan.left];
		x[1] = scan.x[scan.right];

		// apply top-left fill-convention, left
		xStart = fill_convention_left(x[0]);
		xEnd = fill_convention_right(x[1]);
		if (xEnd < xStart)
			return 1;

		// slopes
		const f32 invDeltaX = fill_step_x(x[1] - x[0]);
#ifdef SOFTWARE_DRIVER_2_SUBTEXEL
		const f32 subPixel = ((f32)xStart) - x[0];
#endif

		w[1] = (scan.w[scan.right] - scan.w[scan.left]) * invDeltaX;
		w[0] = scan.w[scan.left] + w[1] * subPixel;

		u32 i;
		for (i = 0; i != scan.ipolsize_t; ++i)
		{
			t[i][1] = (scan.t[i][scan.right] - scan.t[i][scan.left]) * invDeltaX;
			t[i][0] = scan.t[i][scan.left] + t[i][1] * subPixel;
		}

		for (i = 0; i != scan.ipolsize_c; ++i)
		{
			c[i][1] = (scan.c[i][scan.right] - scan.c[i][scan.left]) * invDeltaX;
			c[i][0] = scan.c[i][scan.left] + c[i][1] * subPixel;
		}

		for (i = 0; i != scan.ipolsize_l; ++i)
		{
			l[i][1] = (scan.l[i][scan.right] - scan.l[i][scan.left]) * invDeltaX;
			l[i][0] = scan.l[i][scan.left] + l[i][1] * subPixel;
		}

		return 0;
	}


	void ipol_step(const sScanConvertData& scan)
	{
		w[0] += w[1];

		u32 i;
		for (i = 0; i != scan.ipolsize_t; ++i)
		{
			t[i][0] += t[i][1];
		}
		for (i = 0; i != scan.ipolsize_c; ++i)
		{
			c[i][0] += c[i][1];
		}
		for (i = 0; i != scan.ipolsize_l; ++i)
		{
			l[i][0] += l[i][1];
		}

	}
#endif // #if defined(burning_glsl_emu_test)
};

//GLSL Emu
struct SBurningShaderMaterial;
struct glslEmu
{
	//link
	const SBurningShaderMaterial* MaterialLink;
	void OnSetMaterial_BL(const SBurningShaderMaterial& material);

	//derivative
	u32 varying_count;

	typedef u32 sampler2D;
	typedef u32 samplerCube;
	typedef sVec2 vec2;
	typedef sVec4 vec3;
	typedef sVec4 vec4;

#if defined(burning_glsl_emu_test)
	//pointing back
	struct glslEmu2
	{
		const SBurningShaderMaterial* MaterialLink;
		u32 varying_count;
	};
	glslEmu2 glsl;

	//in
	vec4 gl_FragCoord;
	vec4 gl_Color;
	vec4 gl_TexCoord[BURNING_MATERIAL_MAX_TEXTURES];
	//vec4 gl_TexCoord4[1];
	//out
	vec4 gl_FragColor;

	//derivative
	int _discard;

	// fragment shader
	void call_fragmentShader();

	typedef void (glslEmu::* tFragmentShader) ();
	tFragmentShader fragmentShader;
	void frag_default()
	{
		gl_FragColor = gl_Color;
	}
	void frag_BFT_IRR_0342_0x11b0394b();
	void frag_BFT_STK_1612_0x1e372102();
#endif
};


/*
	load a color value. mulby controls [0;1] or [0;ColorMax]
	aka getSample_color
*/
REALINLINE void vec4_to_fix(tFixPoint& r, tFixPoint& g, tFixPoint& b, const sVec4& v, const f32 mulby)
{
	r = tofix(v.r, mulby);
	g = tofix(v.g, mulby);
	b = tofix(v.b, mulby);
}


REALINLINE void vec4_to_fix(tFixPoint& a, tFixPoint& r, tFixPoint& g, tFixPoint& b, const sVec4& v, const f32 mulby)
{
	a = tofix(v.a, mulby);
	r = tofix(v.r, mulby);
	g = tofix(v.g, mulby);
	b = tofix(v.b, mulby);
}

burning_namespace_end
#endif
