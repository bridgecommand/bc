/*
 * Copyright (c) 2013, elvman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY elvman ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _REALISTIC_WATER_SCENE_NODE_H
#define _REALISTIC_WATER_SCENE_NODE_H

#include <irrlicht.h>

using namespace irr;

class RealisticWaterSceneNode: public scene::ISceneNode, video::IShaderConstantSetCallBack
{
public:
	RealisticWaterSceneNode(scene::ISceneManager* sceneManager, f32 width, f32 height,
							const irr::core::stringc& resourcePath = irr::core::stringc(),
							core::dimension2du renderTargetSize=core::dimension2du(512,512),scene::ISceneNode* parent = NULL, s32 id = -1);
	virtual ~RealisticWaterSceneNode();

	// frame
	virtual void OnRegisterSceneNode();

	virtual void OnAnimate(u32 timeMs);

	// renders terrain
	virtual void render();
    
	// returns the axis aligned bounding box of terrain
	virtual const core::aabbox3d<f32>& getBoundingBox() const;

	virtual void OnSetConstants(video::IMaterialRendererServices* services, s32 userData);

	void setWindForce(f32 windForce);
	void setWindDirection(const core::vector2df& windDirection);
	void setWaveHeight(f32 waveHeight);

	void setWaterColor(const video::SColorf& waterColor);
	void setColorBlendFactor(f32 colorBlendFactor);

private:

	scene::ICameraSceneNode*		_camera;
	scene::ISceneNode*				_waterSceneNode;

	video::IVideoDriver*			_videoDriver;
	scene::ISceneManager*			_sceneManager;
	
	core::dimension2d<f32>			_size;

	s32								_shaderMaterial;

	scene::IAnimatedMesh*			_waterMesh;

	video::ITexture*				_refractionMap;
	video::ITexture*				_reflectionMap;

	f32								_windForce;
	core::vector2df					_windDirection;
	f32								_waveHeight;

	video::SColorf					_waterColor;
	f32								_colorBlendFactor;

	u32								_time;
};

#endif