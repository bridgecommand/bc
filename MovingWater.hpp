//Based on CWaterSurfaceSceneNode, with the original copyright notice:
// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

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


        f32 addWave(const core::vector3df &source, f32 time) const;
		f32 WaveLength;
		f32 WaveSpeed;
		f32 WaveHeight;
		IMesh* mesh;
	};

} // end namespace scene
} // end namespace irr

#endif

