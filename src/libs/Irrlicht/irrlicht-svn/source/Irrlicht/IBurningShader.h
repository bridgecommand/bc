// Copyright (C) 2002-2012 Nikolaus Gebhardt / Thomas Alten
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef IRR_I_BURNING_SHADER_H_INCLUDED
#define IRR_I_BURNING_SHADER_H_INCLUDED

#include "SoftwareDriver2_compile_config.h"
#include "IReferenceCounted.h"
#include "irrMath.h"
#include "IImage.h"
#include "S2DVertex.h"
#include "rect.h"
#include "CDepthBuffer.h"
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
	bool LightIsOn;

	sVec3Color AmbientColor;
	sVec3Color DiffuseColor;
	sVec3Color SpecularColor;

	//normal,parallax
	sVec4 pos_local;	//modelinverse
	f32 nmap_linearAttenuation;

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

	f32 fog_scale; // 1 / (fog.end-fog.start)

	size_t TL_Flag; // eTransformLightFlags

	// objectspace
	core::matrix4 mvi;	// inverse Model*View
	sVec4 leye;	//eye vector unprojected
};

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

enum eBurningStencilOp
{
	StencilOp_KEEP = 0x1E00,
	StencilOp_INCR = 0x1E02,
	StencilOp_DECR = 0x1E03
};

enum eBurningVertexShader
{
	BVT_Fix = 0,
	BVT_815_0x1f847599,		/* example 27 pp_opengl.vert */
	BVT_opengl_vsh_shaderexample,

	STK_1259_0xc8226e1a,	/* supertuxkart bubble.vert */
	STK_958_0xa048973b,		/* supertuxkart motion_blur.vert */
	STK_1204_0x072a4094,	/* supertuxkart splatting.vert */
	STK_1309_0x1fd689c2,	/* supertuxkart normalmap.vert */
	STK_1303_0xd872cdb6,	/* supertuxkart water.vert */
};

struct SBurningShaderMaterial
{
	SMaterial org;
	SMaterial lastMaterial;
	bool resetRenderStates;

	E_MATERIAL_TYPE Fallback_MaterialType;
	eBurningVertexShader VertexShader;

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
	ETR_FLAT = 0,
	ETR_FLAT_WIRE,
	ETR_GOURAUD,
	ETR_GOURAUD_WIRE,
	ETR_TEXTURE_FLAT,
	ETR_TEXTURE_FLAT_WIRE,
	ETR_TEXTURE_GOURAUD,
	ETR_TEXTURE_GOURAUD_WIRE,
	ETR_TEXTURE_GOURAUD_NOZ,
	ETR_TEXTURE_GOURAUD_ADD,
	ETR_TEXTURE_GOURAUD_ADD_NO_Z,

	ETR_TEXTURE_GOURAUD_VERTEX_ALPHA,

	ETR_TEXTURE_GOURAUD_LIGHTMAP_M1,
	ETR_TEXTURE_GOURAUD_LIGHTMAP_M2,
	ETR_TEXTURE_GOURAUD_LIGHTMAP_M4,
	ETR_TEXTURE_LIGHTMAP_M4,

	ETR_TEXTURE_GOURAUD_DETAIL_MAP,
	ETR_TEXTURE_GOURAUD_LIGHTMAP_ADD,

	ETR_GOURAUD_NOZ,
	//ETR_GOURAUD_ALPHA,
	ETR_GOURAUD_ALPHA_NOZ,

	ETR_TEXTURE_GOURAUD_ALPHA,
	ETR_TEXTURE_GOURAUD_ALPHA_NOZ,
	ETR_TEXTURE_GOURAUD_ALPHA_NOZ_NOPERSPECTIVE_CORRECT,

	ETR_NORMAL_MAP_SOLID,
	ETR_PARALLAX_MAP_SOLID,
	ETR_STENCIL_SHADOW,

	ETR_TEXTURE_BLEND,
	ETR_TRANSPARENT_REFLECTION_2_LAYER,

	ETR_COLOR,

	//ETR_REFERENCE,
	ETR_INVALID,

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
		return ((type & 3) == (other.type & 3)) && tiny_istoken(name, other.name);
	}

};

class IBurningShader;
struct PushShaderData
{
	IBurningShader* CurrentShader;
	size_t EdgeTestPass; /* edge_test_flag*/
	void push(IBurningShader* shader);
	void pop();
};

class CBurningVideoDriver;
class IBurningShader : public IMaterialRenderer, public IMaterialRendererServices, public IShaderConstantSetCallBack
{
public:
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
	virtual void setRenderTarget(video::IImage* surface, const core::rect<s32>& viewPort, const interlaced_control interlaced);

	//! sets the Texture
	virtual void setTextureParam(const size_t stage, video::CSoftwareTexture2* texture, s32 lodFactor);
	virtual void drawTriangle(const s4DVertex* burning_restrict a, const s4DVertex* burning_restrict b, const s4DVertex* burning_restrict c) {};
	virtual void drawLine(const s4DVertex* a, const s4DVertex* b);
	virtual void drawPoint(const s4DVertex* a);

	void drawWireFrameTriangle(s4DVertex* a, s4DVertex* b, s4DVertex* c);

	virtual void OnSetMaterialBurning(const SBurningShaderMaterial& material) {};

	void setEdgeTest(const int wireFrame, const int pointCloud)
	{
		EdgeTestPass = pointCloud ? edge_test_point : wireFrame ? edge_test_left : edge_test_pass;
	}

	void pushShader(PushShaderData* data, int save);
	virtual bool canWireFrame() { return false; }
	virtual bool canPointCloud() { return false; }

	void setStencilOp(eBurningStencilOp sfail, eBurningStencilOp dpfail, eBurningStencilOp dppass);

	//IShaderConstantSetCallBack
	virtual void OnSetConstants(IMaterialRendererServices* services, s32 userData) IRR_OVERRIDE {};
	virtual void OnSetMaterial(const SMaterial& material) IRR_OVERRIDE { }

	//IMaterialRenderer
	virtual void OnSetMaterial(const SMaterial& material, const SMaterial& lastMaterial,
		bool resetAllRenderstates, IMaterialRendererServices* services) IRR_OVERRIDE;

	virtual bool OnRender(IMaterialRendererServices* service, E_VERTEX_TYPE vtxtype) IRR_OVERRIDE;

	virtual void OnUnsetMaterial() IRR_OVERRIDE;

	//! Returns if the material is transparent.
	virtual bool isTransparent() const IRR_OVERRIDE;

	//! Access the callback provided by the users when creating shader materials
	virtual IShaderConstantSetCallBack* getShaderConstantSetCallBack() const IRR_OVERRIDE;

	// implementations for the render services
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
		color_to_fix(fog_color, fog_color_sample);
	}
	void setScissor(const AbsRectangle& scissor)
	{
		Scissor = scissor;
	}

	u32 fragment_draw_count;

	const f32* getUniform(const c8* name, EBurningUniformFlags flags) const;

protected:
	//friend class CBurningVideoDriver;

	void constructor_IBurningShader(CBurningVideoDriver* driver, E_MATERIAL_TYPE baseMaterial);

	CBurningVideoDriver* Driver;
	IShaderConstantSetCallBack* CallBack;
	E_MATERIAL_TYPE BaseMaterial;
	s32 UserData;

	core::array<BurningUniform> UniformInfo;
	s32 getShaderConstantID(EBurningUniformFlags program, const c8* name);
	bool setShaderConstantID(EBurningUniformFlags flags, s32 index, const void* data, size_t u32_count);

	video::CImage* RenderTarget;
	CDepthBuffer* DepthBuffer;
	CStencilBuffer* Stencil;
	tVideoSample ColorMask;

	sInternalTexture IT[BURNING_MATERIAL_MAX_TEXTURES];

	static const tFixPointu dithermask[4 * 4];

	//draw degenerate triangle as line (left edge) drawTriangle -> holes,drawLine dda/bresenham
	size_t EdgeTestPass; //edge_test_flag
	interlaced_control Interlaced; // passed from driver

	eBurningStencilOp stencilOp[4];
	tFixPoint AlphaRef;
	int RenderPass_ShaderIsTransparent;

	sScanConvertData ALIGN(16) scan;
	sScanLineData line;
	tVideoSample PrimitiveColor; //used if no color interpolation is defined

	size_t /*eTransformLightFlags*/ TL_Flag;
	tFixPoint fog_color[4];
	tVideoSample fog_color_sample;

	AbsRectangle Scissor;

	//core::stringc VertexShaderProgram;
	//core::stringc PixelShaderProgram;
	eBurningVertexShader VertexShaderProgram_buildin;

	inline tVideoSample color_to_sample(const video::SColor& color) const
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
IBurningShader* createTriangleRendererGouraud2(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererGouraudNoZ2(CBurningVideoDriver* driver);
IBurningShader* createTRGouraudAlphaNoZ2(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererGouraudWire2(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererTextureFlat2(CBurningVideoDriver* driver);
IBurningShader* createTriangleRendererTextureFlatWire2(CBurningVideoDriver* driver);
IBurningShader* createTRFlat2(CBurningVideoDriver* driver);
IBurningShader* createTRFlatWire2(CBurningVideoDriver* driver);
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

#endif
