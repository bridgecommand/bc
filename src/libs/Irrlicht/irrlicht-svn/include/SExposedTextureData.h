// Copyright (C) 2026 Michael Zeilfelder
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef S_EXPOSED_TEXTURE_DATA_H_INCLUDED
#define S_EXPOSED_TEXTURE_DATA_H_INCLUDED

// forward declarations for internal pointers
struct IDirect3DTexture9;
struct IDirect3DCubeTexture9;
struct IDirect3DSurface9;

namespace irr
{
namespace video
{

//! Structure for holding texture data internally used by drivers
/** This data can be retrieved by ITexture::getExposedTextureData(). 
Note that this structure does not contain any valid data if the driver 
does not return any exposed data.
*/
struct SExposedTextureData
{
	SExposedTextureData() {D3D9.Texture=0; D3D9.CubeTexture=0; D3D9.RTTSurface=0;}

	// For driver type EDT_DIRECT3D9
	struct SD3D9
	{
		IDirect3DTexture9* Texture;
		IDirect3DCubeTexture9* CubeTexture;
		IDirect3DSurface9* RTTSurface;
	};

	// For OpenGL driver types
	struct SOpenGL
	{
		unsigned int TextureType;	// GLenum: GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP
		unsigned int TextureName;	// GLuint: Name of a GL texture object
	};

	union
	{
		SD3D9 D3D9;
		SOpenGL OpenGL;
	};
};

} // end namespace video
} // end namespace irr

#endif
