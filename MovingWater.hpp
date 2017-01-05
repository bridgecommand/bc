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

//From Mel demo (http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=51130&start=15#p296723) START
class cubemapConstants : public irr::video::IShaderConstantSetCallBack
{
    int matWorldViewProjection;//Identifiers, much faster than string matching...
    int matViewInverse;
    int matWorld;
    int baseMap;
    int reflectionMap;

    bool firstRun;
    bool IsOpenGL;//Our constants set callback isn't limited to D3D9

    irr::video::IVideoDriver* driver; //Here so we can save a call during the execution

public:
    cubemapConstants(bool isGL)
    {
        IsOpenGL = isGL;
        firstRun = true;
    }

    void OnSetConstants(irr::video::IMaterialRendererServices* services, irr::s32 userData)
    {
        if(firstRun)
        {
            firstRun = false;

            driver = services->getVideoDriver();
            //Looking for our constants IDs...
            matViewInverse = services->getVertexShaderConstantID("matViewInverse");

            if(IsOpenGL)
            {
                baseMap = services->getPixelShaderConstantID("baseMap");
                reflectionMap = services->getPixelShaderConstantID("reflectionMap");
            }
            else
            {
                matWorldViewProjection=services->getVertexShaderConstantID("matWorldViewProjection");
                matWorld = services->getVertexShaderConstantID("matWorld");
            }
        }

        //Setting up our constants...
        irr::core::matrix4 mat;

        mat = driver->getTransform(irr::video::ETS_VIEW);
        mat.makeInverse();
        services->setVertexShaderConstant(matViewInverse,mat.pointer(),16);

        if(IsOpenGL)
        {
            int sampler=0;
            services->setPixelShaderConstant(baseMap,&sampler,1);
            sampler=1;
            services->setPixelShaderConstant(reflectionMap,&sampler,1);
        }
        else
        {
            mat = driver->getTransform(irr::video::ETS_PROJECTION);
            mat *= driver->getTransform(irr::video::ETS_VIEW);
            mat *= driver->getTransform(irr::video::ETS_WORLD);
            services->setVertexShaderConstant(matWorldViewProjection,mat.pointer(),16);

            mat = driver->getTransform(irr::video::ETS_WORLD);
            services->setVertexShaderConstant(matWorld,mat.pointer(),16);
        }
    }
}; //From Mel demo (http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=51130&start=15#p296723) END

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


        //f32 addWave(const core::vector3df &source, f32 time) const;
		f32 WaveLength;
		f32 WaveSpeed;
		f32 WaveHeight;
		f32 tileWidth;
		IMesh* mesh;
		cOcean* ocean;
	};

} // end namespace scene
} // end namespace irr

#endif

