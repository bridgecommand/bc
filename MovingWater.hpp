//Based on CWaterSurfaceSceneNode

// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __MOVING_WATER_HPP_INCLUDED__
#define __MOVING_WATER_HPP_INCLUDED__

#include "irrlicht.h"

namespace irr
{
namespace scene
{

	class MovingWaterSceneNode : public IMeshSceneNode
	{
	public:

		//! constructor
		MovingWaterSceneNode(f32 waveHeight, f32 waveSpeed, f32 waveLength,
            ISceneNode* parent, ISceneManager* mgr,	s32 id,
			const core::vector3df& position = core::vector3df(0,0,0),
			const core::vector3df& rotation = core::vector3df(0,0,0),
			const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f));

		//! destructor
		//virtual ~MovingWaterSceneNode();
		~MovingWaterSceneNode();

		//! frame registration
		//virtual void OnRegisterSceneNode();
		void OnRegisterSceneNode();

		//! animated update
		//virtual void OnAnimate(u32 timeMs);
		void OnAnimate(u32 timeMs);

		//! Update mesh
		//virtual void setMesh(IMesh* mesh);
		void setMesh(IMesh* mesh);

		//! Returns type of the scene node
		//virtual ESCENE_NODE_TYPE getType() const { return ESNT_WATER_SURFACE; }
		ESCENE_NODE_TYPE getType() const { return ESNT_WATER_SURFACE; }

		void setMaterialTexture(u32 textureLayer, video::ITexture * texture);

		virtual void render();
		virtual const core::aabbox3d<f32>& getBoundingBox() const;
		virtual IMesh* getMesh(void);
		virtual IShadowVolumeSceneNode* addShadowVolumeSceneNode(const IMesh* shadowMesh=0, s32 id=-1, bool zfailmethod=true, f32 infinity=1000.0f);
		virtual void setReadOnlyMaterials(bool readonly);
		virtual bool isReadOnlyMaterials() const;


	private:

		f32 WaveLength;
		f32 WaveSpeed;
		f32 WaveHeight;
		IMesh* mesh;
	};

} // end namespace scene
} // end namespace irr

#endif

