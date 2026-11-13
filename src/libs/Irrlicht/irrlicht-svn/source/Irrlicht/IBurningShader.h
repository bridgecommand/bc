// Copyright (C) 2002-2022 Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef IRR_I_BURNING_SHADER_H_INCLUDED
#define IRR_I_BURNING_SHADER_H_INCLUDED

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_
#include "SoftwareDriver2_compile_config.h"
#include "IReferenceCounted.h"
#include "irrMath.h"
#include "IImage.h"
#include "S2DVertex.h"
#include "rect.h"
#include "S4DVertex.h"
#include "irrArray.h"
#include "SLight.h"
#include "SMaterial.h"
#include "os.h"
#include "IMaterialRenderer.h"
#include "IMaterialRendererServices.h"
#include "IGPUProgrammingServices.h"
#include "IShaderConstantSetCallBack.h"

burning_namespace_start

struct SBurningShaderLight
{
	//SLight org;
	//s32 HardwareLightIndex;
	sVec4 pos;	//light position input
	sVec4 pos4; //light position Model*View (Identity*View)
	//sVec4 pos4n; //Norm direction to infinite light  = Normalize( Position ) 
	//sVec4 halfVector; //Norm( VP_inf_norm + <0,0,1> ) 

	E_LIGHT_TYPE Type;
	f32 linearAttenuation;
	f32 constantAttenuation;
	f32 quadraticAttenuation;

	sVec4 spotDirection;
	sVec4 spotDirection4;
	f32 spotCosCutoff;
	f32 spotCosInnerCutoff;
	f32 spotExponent;
	f32 _pad;

	sVec3Color AmbientColor;
	sVec3Color DiffuseColor;
	sVec3Color SpecularColor;

	//normal,parallax
	sVec4 pos_local;	//modelinverse
	f32 nmap_linearAttenuation;

	bool LightIsOn;
	SBurningShaderLight()
	{
		LightIsOn = false;
	}
};

enum eTransformLightFlags
{
	//ENABLED		= 0x01,
	TL_SCISSOR = 0x02,
	TL_LIGHT = 0x04,
	TL_SPECULAR = 0x08,
	TL_FOG = 0x10,
	TL_NORMALIZE_NORMALS = 0x20,
	TL_TEXTURE_TRANSFORM = 0x40,		// need eyespace matrices
	TL_LIGHT_LOCAL_VIEWER = 0x80,
	TL_LIGHT0_IS_NORMAL_MAP = 0x100,	// sVec4 Light Vector is used as normal or specular

	TL_COLORMAT_AMBIENT = 0x200,
	TL_COLORMAT_DIFFUSE = 0x400,
	TL_COLORMAT_SPECULAR = 0x800,

};

struct SBurningShaderEyeSpace
{
	SBurningShaderEyeSpace() {}
	virtual ~SBurningShaderEyeSpace() {}
	void init()
	{
		Light.set_used(0);
		Global_AmbientLight.set(0.2f,0.2f,0.2f,1.f);

		fog_scale = 0.f;
		TL_Flag = TL_LIGHT_LOCAL_VIEWER;
	}
	void deleteAllDynamicLights()
	{
		Light.set_used(0);
		TL_Flag &= ~(TL_LIGHT | TL_SPECULAR);
	}

	core::array<SBurningShaderLight> Light;
	sVec3Color Global_AmbientLight;

	//sVec4 cam_eye_pos; //Camera Position in eye Space (0,0,-1)
	//sVec4 cam_world_pos; //Camera Position in world Space
	//sVec4 vertex4; //eye coordinate position of vertex
	sVec4 normal; // normal in eye space,transpose(inverse(mat3(mv_matrix)); gl_NormalMatrix
	sVec4 vertex; //eye coordinate position of vertex projected

	//derivative of vertex
	//f32 cam_distance; // vertex.length();
	sVec4 vertexn; //vertex.normalize(); eye = -vertex.normalize()

	// objectspace
	core::matrix4 mvi;	// inverse Model*View
	sVec4 leye;	//eye vector unprojected

	f32 fog_scale; // 1 / (fog.end-fog.start)
	size_t TL_Flag; // eTransformLightFlags
};


enum eBurningStencilOp
{
	StencilOp_KEEP = 0x1E00,
	StencilOp_INCR = 0x1E02,
	StencilOp_DECR = 0x1E03
};

enum eBurningVertexFragmentShaderID
{
	BVT_Fix = 0,
	BFT_Fix,

	BVT_IRR_1100_0x12c79d1c,	/* example 10 opengl.vert */
	BFT_IRR_0225_0x3bf0d5a1,	/* example 10 opengl.frag new */
	BVT_IRR_0815_0x1f847599,	/* example 27 pp_opengl.vert */
	BFT_IRR_0385_0xa0194718,	/* example 27 pp_opengl.frag */
	BVT_IRR_0837_0x884022e6,    /* example 28 cubeMapReflection.vert */
	BFT_IRR_0342_0x11b0394b,    /* example 28 cubeMapReflection.fraq */
#if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0_Shader)
	BVT_STK_1259_0xc8226e1a,	/* supertuxkart bubble.vert */
	BVT_STK_0958_0xa048973b,	/* supertuxkart motion_blur.vert */
	BFT_STK_2971_0xd34c333c,	/* supertuxkart motion_blur.frag */
	BVT_STK_1204_0x072a4094,	/* supertuxkart splatting.vert */
	BFT_STK_1840_0x6c2f4ae2,	/* supertuxkart splatting.frag */
	BVT_STK_1060_0x07eb3472,	/* supertuxkart splatting2.vert */
	BFT_STK_1612_0x1e372102,	/* supertuxkart splatting2.frag */
	BVT_STK_1309_0x1fd689c2,	/* supertuxkart normalmap.vert */
	BVT_STK_1303_0xd872cdb6,	/* supertuxkart water.vert */
	BVT_STK_1713_0x9ca851d6,	/* supertuxkart water.frag */
#endif
	BV_FRAGMENT_SHADER = 0x8B30,
	BV_VERTEX_SHADER = 0x8B31,
};

struct BVCompiledShader
{
	eBurningVertexFragmentShaderID id; // buildin
	eBurningVertexFragmentShaderID shaderType;
	SVSize vertexFormat;

	void init(eBurningVertexFragmentShaderID id);
};

class IBurningShader;
struct SBurningShaderMaterial
{
	SMaterial org;
	SMaterial lastMaterial;
	bool resetRenderStates;

	E_MATERIAL_TYPE Fallback_MaterialType;
	BVCompiledShader VertexShader;
	BVCompiledShader FragmentShader;
	IBurningShader* shader; // shader instance Driver->MaterialRenderers[MaterialType];
	IBurningShader* shader_org;

	SMaterial mat2D;
	//SMaterial save3D;

	size_t CullFlag; //eCullFlag
	u32 depth_write;
	u32 depth_test;

	sVec4 AmbientColor;
	sVec4 DiffuseColor;
	sVec4 SpecularColor;
	sVec4 EmissiveColor;

};

enum EBurningFFShader
{
	ETR_COLOR = 0,
	ETR_TEXTURE_GOURAUD,
	ETR_TEXTURE_GOURAUD_WIRE,
	ETR_TEXTURE_GOURAUD_NOZ,
	ETR_TEXTURE_GOURAUD_ADD,
	ETR_TEXTURE_GOURAUD_ADD_NOZ,	// depth test yes, depth write no

	ETR_TEXTURE_GOURAUD_VERTEX_ALPHA,

	ETR_TEXTURE_GOURAUD_LIGHTMAP_M1,
	ETR_TEXTURE_GOURAUD_LIGHTMAP_M2,
	ETR_TEXTURE_GOURAUD_LIGHTMAP_M4,
	ETR_TEXTURE_LIGHTMAP_M4,

	ETR_TEXTURE_GOURAUD_DETAIL_MAP,
	ETR_TEXTURE_GOURAUD_LIGHTMAP_ADD,

	ETR_TEXTURE_GOURAUD_ALPHA,
	ETR_TEXTURE_GOURAUD_ALPHA_NOZ,
	ETR_TEXTURE_GOURAUD_ALPHA_NOZ_NOPERSPECTIVE_CORRECT,

	ETR_NORMAL_MAP_SOLID,
	ETR_PARALLAX_MAP_SOLID,
	ETR_STENCIL_SHADOW,

	ETR_TEXTURE_BLEND,
	ETR_TRANSPARENT_REFLECTION_2_LAYER,

	//ETR_REFERENCE,
	//ETR_INVALID,

	ETR2_COUNT
};

typedef enum
{
	BL_VERTEX_PROGRAM = 1,
	BL_FRAGMENT_PROGRAM = 2,
	BL_TYPE_FLOAT = 4,
	BL_TYPE_INT = 8,
	BL_TYPE_UINT = 16,

	BL_VERTEX_FLOAT = (BL_VERTEX_PROGRAM | BL_TYPE_FLOAT),
	BL_VERTEX_INT = (BL_VERTEX_PROGRAM | BL_TYPE_INT),
	BL_VERTEX_UINT = (BL_VERTEX_PROGRAM | BL_TYPE_UINT),
	BL_FRAGMENT_FLOAT = (BL_FRAGMENT_PROGRAM | BL_TYPE_FLOAT),
	BL_FRAGMENT_INT = (BL_FRAGMENT_PROGRAM | BL_TYPE_INT),
	BL_FRAGMENT_UINT = (BL_FRAGMENT_PROGRAM | BL_TYPE_UINT),

	BL_UNIFORM_FLOAT = (BL_VERTEX_PROGRAM | BL_FRAGMENT_PROGRAM | BL_TYPE_FLOAT),

	BL_ACTIVE_UNIFORM_MAX_LENGTH = 28
} EBurningUniformFlags;

struct BurningUniform
{
	c8 name[BL_ACTIVE_UNIFORM_MAX_LENGTH];
	u32 type; //EBurningUniformFlags
	//int location; // UniformLocation is index
	f32 data[16];	// simple LocalParameter

	bool operator==(const BurningUniform& other) const
	{
		return (
			(type & (BL_VERTEX_PROGRAM| BL_FRAGMENT_PROGRAM)) == (other.type & (BL_VERTEX_PROGRAM | BL_FRAGMENT_PROGRAM)))
			&& tiny_istoken(name, other.name);
	}

};


struct PushShaderData
{
	IBurningShader* CurrentShader;
	size_t EdgeTestPass; /* edge_test_flag*/
	void push(IBurningShader* shader);
	void pop();
};

sVec4 BL_Sampler_nearest(const int sampler, const sVec4& coord, const s32 coord_count, const SBurningShaderMaterial* MaterialLink);

// RenderTarget with attached buffers
struct sBurningRenderTarget
{
	//memory, "BackBuffer" in Driver, Todo: Shader uses color instead of surface
	video::CImage* backbuffer;
	CDepthBuffer* depth;
	CStencilBuffer* stencil;

	video::ITexture* texture;		// RenderTargetTexture;
	//video::IImage* surface;		// RenderTargetSurface backbuffer || texture
	video::CImage* color;			// target inside shader = surface<
	core::dimension2d<u32> size;	// RenderTargetSize from surface;

	sVec2 ratio_size_screen;		// RatioRenderTargetScreen Smaller Render Target
	interlaced_control interlaced;

	sBurningRenderTarget()
		:backbuffer(0),depth(0), stencil(0),
		texture(0), color(0), size(0,0),
		ratio_size_screen(1.f,1.f)
	{
		// interlaced_disabled
		interlaced.raw = 0;
		interlaced.m.bypass = 1;
	}

	void shader_constructor_grab(sBurningRenderTarget& target); // from driver
	void grab_color(video::IImage* surface);
	void drop();

	void clearBuffers(const u32 flag /*E_CLEAR_BUFFER_FLAG*/, SColor color_value, const f32 depth_value, const u32 stencil_value);
	void OnResize(const core::dimension2d<u32>& size);
	void setRenderTargetImage(video::CImage* new_color, video::IImage* new_depth, video::IImage* new_stencil, const core::dimension2d<u32>& ScreenSize);
};

// 2D Region closed [x0;x1] cloned AbsRectangle
struct AbsRectangle2
{
	s32 x0;
	s32 y0;
	s32 x1;
	s32 y1;

	//! 2D Intersection test
	static inline bool intersect(AbsRectangle2& dest, const AbsRectangle2& a, const AbsRectangle2& b)
	{
		dest.x0 = core::s32_max(a.x0, b.x0);
		dest.y0 = core::s32_max(a.y0, b.y0);
		dest.x1 = core::s32_min(a.x1, b.x1);
		dest.y1 = core::s32_min(a.y1, b.y1);
		return dest.x0 < dest.x1 && dest.y0 < dest.y1;
	}
};

class IBurningShader : public IMaterialRenderer, public IMaterialRendererServices, public IShaderConstantSetCallBack
{
public:
	//friend class CBurningVideoDriver;
	//! Constructor
	IBurningShader(CBurningVideoDriver* driver, E_MATERIAL_TYPE baseMaterial );

	//! Constructor
	IBurningShader(
		CBurningVideoDriver* driver,
		s32& outMaterialTypeNr,
		const c8* vertexShaderProgram = 0,
		const c8* vertexShaderEntryPointName = 0,
		E_VERTEX_SHADER_TYPE vsCompileTarget = video::EVST_VS_1_1,
		const c8* pixelShaderProgram = 0,
		const c8* pixelShaderEntryPointName = 0,
		E_PIXEL_SHADER_TYPE psCompileTarget = video::EPST_PS_1_1,
		const c8* geometryShaderProgram = 0,
		const c8* geometryShaderEntryPointName = "main",
		E_GEOMETRY_SHADER_TYPE gsCompileTarget = EGST_GS_4_0,
		scene::E_PRIMITIVE_TYPE inType = scene::EPT_TRIANGLES,
		scene::E_PRIMITIVE_TYPE outType = scene::EPT_TRIANGLE_STRIP,
		u32 verticesOut = 0,
		IShaderConstantSetCallBack* callback = 0,
		E_MATERIAL_TYPE baseMaterial = EMT_SOLID,
		s32 userData = 0);

	//! destructor
	virtual ~IBurningShader();

	//! sets a render target
	virtual void linkRenderTarget(const sBurningRenderTarget& driver_target, const core::rect<s32>& viewPort);
	

	//! sets the Texture
	virtual void setTextureParam(glslEmu::sampler2D sampler, video::CSoftwareTexture2* texture, s32 lodFactor);
	virtual void drawTriangle(const s4DVertex* burning_restrict a, const s4DVertex* burning_restrict b, const s4DVertex* burning_restrict c)
	{
		irr_unreferenced_parameter(a);
		irr_unreferenced_parameter(b);
		irr_unreferenced_parameter(c);
	};
	virtual void drawLine(const s4DVertex* a, const s4DVertex* b);
	virtual void drawPoint(const s4DVertex* a);

	void drawWireFrameTriangle(s4DVertex* a, s4DVertex* b, s4DVertex* c);

	void setEdgeTest(const int wireFrame, const int pointCloud)
	{
		EdgeTestPass = pointCloud ? edge_test_point : wireFrame ? edge_test_left : edge_test_pass;
	}

	void pushShader(PushShaderData* data, int save);
	virtual bool canWireFrame() { return false; }
	virtual bool canPointCloud() { return false; }

	void setStencilOp(eBurningStencilOp sfail, eBurningStencilOp dpfail, eBurningStencilOp dppass);

	//IShaderConstantSetCallBack
	virtual void OnSetConstants(IMaterialRendererServices* services, s32 userData) IRR_OVERRIDE
	{
		irr_unreferenced_parameter(services);
		irr_unreferenced_parameter(userData);
	};
	virtual void OnSetMaterial(const SMaterial& material) IRR_OVERRIDE
	{
		irr_unreferenced_parameter(material);
	}

	//IMaterialRenderer
	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) IRR_OVERRIDE;

	virtual void OnSetMaterial_BL(const SBurningShaderMaterial& material)
	{
		irr_unreferenced_parameter(material);
	};

	//called after user callback OnRender (Shader)
	virtual bool OnRender(IMaterialRendererServices* service, E_VERTEX_TYPE vtxtype) IRR_OVERRIDE;
	virtual void OnRender_BL(IMaterialRendererServices* service)
	{
		irr_unreferenced_parameter(service);
	}

	virtual void OnUnsetMaterial() IRR_OVERRIDE;

	//! Returns if the material is transparent.
	virtual bool isTransparent() const IRR_OVERRIDE;

	//! Access the callback provided by the users when creating shader materials
	virtual IShaderConstantSetCallBack* getShaderConstantSetCallBack() const IRR_OVERRIDE;

	// implementations for the render services
	void setBasicRenderStates(const SMaterial& material, const SMaterial& lastMaterial, bool resetAllRenderstates);
	virtual s32 getVertexShaderConstantID(const c8* name) IRR_OVERRIDE;
	virtual s32 getPixelShaderConstantID(const c8* name) IRR_OVERRIDE;
	virtual void setVertexShaderConstant(const f32* data, s32 startRegister, s32 constantAmount = 1) IRR_OVERRIDE;
	virtual void setPixelShaderConstant(const f32* data, s32 startRegister, s32 constantAmount = 1) IRR_OVERRIDE;
	virtual bool setVertexShaderConstant(s32 index, const f32* floats, int count) IRR_OVERRIDE;
	virtual bool setVertexShaderConstant(s32 index, const s32* ints, int count) IRR_OVERRIDE;
	virtual bool setVertexShaderConstant(s32 index, const u32* ints, int count) IRR_OVERRIDE;
	virtual bool setPixelShaderConstant(s32 index, const f32* floats, int count) IRR_OVERRIDE;
	virtual bool setPixelShaderConstant(s32 index, const s32* ints, int count) IRR_OVERRIDE;
	virtual bool setPixelShaderConstant(s32 index, const u32* ints, int count)  IRR_OVERRIDE;
	virtual IVideoDriver* getVideoDriver() IRR_OVERRIDE;

#if defined(PATCH_SUPERTUX_8_0_1_with_1_9_0)
	virtual bool setVertexShaderConstant(const c8* name, const f32* floats, int count)
	{
		return setVertexShaderConstant(getVertexShaderConstantID(name), floats, count);
	}
	virtual bool setVertexShaderConstant(const c8* name, const bool* bools, int count)
	{
		return setVertexShaderConstant(getVertexShaderConstantID(name), (const s32*)bools, count);
	}
	virtual bool setVertexShaderConstant(const c8* name, const s32* ints, int count)
	{
		return setVertexShaderConstant(getVertexShaderConstantID(name), ints, count);
	}

	virtual bool setPixelShaderConstant(const c8* name, const f32* floats, int count)
	{
		return setPixelShaderConstant(getPixelShaderConstantID(name), floats, count);
	}
	virtual bool setPixelShaderConstant(const c8* name, const bool* bools, int count)
	{
		return setPixelShaderConstant(getPixelShaderConstantID(name), (const s32*)bools, count);
	}
	virtual bool setPixelShaderConstant(const c8* name, const s32* ints, int count)
	{
		return setPixelShaderConstant(getPixelShaderConstantID(name), ints, count);
	}
#endif

	//used if no color interpolation is defined
	void setPrimitiveColor(const video::SColor& color)
	{
		PrimitiveColor = color_to_sample(color);
	}
	void setTLFlag(size_t in /*eTransformLightFlags*/)
	{
		TL_Flag = in;
	}
	void setFog(SColor color_fog)
	{
		fog_color_sample = color_to_sample(color_fog);
		sample_to_fix(fog_color, fog_color_sample);
	}
	void setScissor(const AbsRectangle2& scissor)
	{
		Scissor = scissor;
	}

	u32 fragment_draw_count;

	const f32* getUniform(const c8* name, EBurningUniformFlags flags) const;

protected:

	void constructor_IBurningShader(CBurningVideoDriver* driver, E_MATERIAL_TYPE baseMaterial);

	CBurningVideoDriver* Driver;
	IShaderConstantSetCallBack* CallBack;
	E_MATERIAL_TYPE BaseMaterial;
	s32 UserData;

	core::array<BurningUniform> UniformInfo;
	s32 getShaderConstantID(EBurningUniformFlags program, const c8* name);
	bool setShaderConstantID(EBurningUniformFlags flags, s32 index, const void* data, size_t u32_count);

	//doesn't hold data. map name to s4DVertex FixedFunction: SVSize
	core::array<BurningUniform> VaryingInfo;

	// passed from driver
	sBurningRenderTarget RenderTarget;

	static const tFixPointu dithermask[4 * 4];

	//draw degenerate triangle as line (left edge) drawTriangle -> holes,drawLine dda/bresenham
	size_t EdgeTestPass; //edge_test_flag
	

	eBurningStencilOp stencilOp[4];
	tFixPoint AlphaRef;
	int RenderPass_ShaderIsTransparent;

	//move or friendclass
public:
	sInternalTexture IT[BURNING_MATERIAL_MAX_SAMPLER];

	sScanConvertData ALIGN(16) scan;
	sScanLineData line;

	glslEmu glsl;

#if defined(burning_glsl_emu_test)
	void glslEmu_drawFace(s4DVertexPair* face[4]);
protected:
	void glslEmu_drawScanLine();
#endif

protected:
	tRenderTargetColorSample PrimitiveColor; //used if no color interpolation is defined

	//passed/duplicate from driver
	size_t /*eTransformLightFlags*/ TL_Flag;
	tFixPoint fog_color[4];
	tRenderTargetColorSample fog_color_sample;

	AbsRectangle2 Scissor;

	//core::stringc VertexShaderProgram;
	//core::stringc PixelShaderProgram;
	BVCompiledShader VertexShaderProgram;
	BVCompiledShader FragmentShaderProgram;

	inline tRenderTargetColorSample color_to_sample(const video::SColor& color) const
	{
		//RenderTarget->getColorFormat()
#if SOFTWARE_DRIVER_2_RENDERTARGET_COLOR_FORMAT == ECF_A8R8G8B8
		return color.color;
#else
		return color.toA1R5G5B5();
#endif
	}

};


IBurningShader* createTriangleRendererTextureGouraud2(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererTextureLightMap2_M1(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererTextureLightMap2_M2(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererTextureLightMap2_M4(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererGTextureLightMap2_M4(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererTextureLightMap2_Add(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererTextureDetailMap2(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererTextureVertexAlpha2(CBurningVideoDriver* driver);


IBurningShader* createTriangleRendererTextureGouraudWire2(CBurningVideoDriver* driver);
//IBurningShader* createTriangleRendererGouraud2(CBurningVideoDriver* driver);
//IBurningShader* createTriangleRendererGouraudNoZ2(CBurningVideoDriver* driver);
//IBurningShader* createTRGouraudAlphaNoZ2(CBurningVideoDriver* driver);
//IBurningShader* createTriangleRendererGouraudWire2(CBurningVideoDriver* driver);
//IBurningShader* createTriangleRendererTextureFlat2(CBurningVideoDriver* driver);
//IBurningShader* createTriangleRendererTextureFlatWire2(CBurningVideoDriver* driver);
//IBurningShader* createTRFlat2(CBurningVideoDriver* driver);
//IBurningShader* createTRFlatWire2(CBurningVideoDriver* driver);
IBurningShader* createTRTextureGouraudNoZ2(CBurningVideoDriver* driver);
IBurningShader* createTRTextureGouraudAdd2(CBurningVideoDriver* driver);
IBurningShader* createTRTextureGouraudAddNoZ2(CBurningVideoDriver* driver);

IBurningShader* createTRTextureGouraudAlpha(CBurningVideoDriver* driver);
IBurningShader* createTRTextureGouraudAlphaNoZ(CBurningVideoDriver* driver);
IBurningShader* createTRTextureBlend(CBurningVideoDriver* driver);
IBurningShader* createTRTextureInverseAlphaBlend(CBurningVideoDriver* driver);

IBurningShader* createTRNormalMap(CBurningVideoDriver* driver, s32& outMaterialTypeNr, E_MATERIAL_TYPE baseMaterial);
IBurningShader* createTRParallaxMap(CBurningVideoDriver* driver, s32& outMaterialTypeNr, E_MATERIAL_TYPE baseMaterial);
IBurningShader* createTRStencilShadow(CBurningVideoDriver* driver);

IBurningShader* createTriangleRendererReference(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererTexture_transparent_reflection_2_layer(CBurningVideoDriver* driver);

IBurningShader* create_burning_shader_color(CBurningVideoDriver* driver);

burning_namespace_end

#endif // _IRR_COMPILE_WITH_BURNINGSVIDEO_

#endif
