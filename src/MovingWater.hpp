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

#include "FFTWave.hpp"

namespace irr
{
namespace scene
{

	class MovingWaterSceneNode : public IMeshSceneNode, video::IShaderConstantSetCallBack
	{
	public:

		//! constructor
		MovingWaterSceneNode(ISceneNode* parent, ISceneManager* mgr, ISceneNode* ownShip,	s32 id,
			irr::u32 disableShaders,
			bool withReflection,
			irr::u32 segments = 32,
			const core::vector3df& position = core::vector3df(0,0,0),
			const core::vector3df& rotation = core::vector3df(0,0,0)
			);

		//! destructor
		//virtual ~MovingWaterSceneNode();
		~MovingWaterSceneNode();

		//! frame registration
		//virtual void OnRegisterSceneNode();
		void OnRegisterSceneNode();

		//! animated update
		//virtual void OnAnimate(u32 timeMs);
		void OnAnimate(u32 timeMs);

		void OnSetConstants(video::IMaterialRendererServices* services, s32 userData);

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

		//void setVerticalScale(f32 scale);
		void resetParameters(float A, vector2 w, float seaState);

		f32 getWaveHeight(f32 relPosX, f32 relPosZ) const;
		irr::core::vector2df getLocalNormals(irr::f32 relPosX, irr::f32 relPosZ) const;


	private:

        //Shader related
        int matWorldViewProjection;//Identifiers, much faster than string matching...
        int matViewInverse;
        int matWorldReflectionViewProj;
        int matWorld;
        int baseMap;
        int reflectionMap;
        int idLightLevel;
        int idSeaState;
        bool firstRun;
        bool IsOpenGL;//Our constants set callback isn't limited to D3D9
		irr::u32 disableShaders;
		bool withReflection;
        irr::video::IVideoDriver* driver; //Here so we can save a call during the execution

        irr::scene::ICameraSceneNode* _camera; //Local camera for reflections
        irr::video::ITexture* _reflectionMap;

        irr::u32 segments;

        //f32 addWave(const core::vector3df &source, f32 time) const;
		f32 lightLevel;
		f32 seaState;
		f32 tileWidth;
		//f32 scaleFactorVertical;
		IMesh* mesh;
		IMesh* flatMesh;
		cOcean* ocean;

		ISceneNode* ownShipSceneNode;

		core::aabbox3d<f32> boundingBox;

		int localisnan(double x) const;
        int localisinf(double x) const;
	};

} // end namespace scene
} // end namespace irr

#endif

