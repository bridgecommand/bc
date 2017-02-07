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

#include "MovingWater.hpp"
#include "Utilities.hpp"

#include <iostream>

namespace irr
{
namespace scene
{

//! constructor
MovingWaterSceneNode::MovingWaterSceneNode(f32 waveHeight, f32 waveSpeed, f32 waveLength,
		ISceneNode* parent, ISceneManager* mgr, s32 id,
		const core::vector3df& position, const core::vector3df& rotation)
	//: IMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale),
	: IMeshSceneNode(parent, mgr, id, position, rotation, core::vector3df(1.0f,1.0f,1.0f)),
	WaveLength(waveLength), WaveSpeed(waveSpeed), WaveHeight(waveHeight), LightLevel(0.75)
{
	#ifdef _DEBUG
	setDebugName("MovingWaterSceneNode");
	#endif

	//scaleFactorVertical = 1.0;

	//From Mel demo (http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=51130&start=15#p296723) START
	irr::video::E_DRIVER_TYPE driverType = mgr->getVideoDriver()->getDriverType();
	driver = mgr->getVideoDriver();
	irr::video::ITexture* cubemap;
	irr::video::IImage* cubemapImages[6];

	IsOpenGL = (driverType==irr::video::EDT_OPENGL);
    firstRun = true;

	//cubemapConstants* cns = new cubemapConstants(driverType==irr::video::EDT_OPENGL);
    //So far there are no materials ready to use a cubemap, so we provide our own.
    irr::s32 shader;

    if(driverType==irr::video::EDT_DIRECT3D9)
        shader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
            "shaders/shader.hlsl",
            "vs_main",
            irr::video::EVST_VS_2_0,
            "shaders/shader.hlsl",
            "ps_main",
            irr::video::EPST_PS_2_0,
            this, //For callbacks
            irr::video::EMT_SOLID
            );
    else //OpenGL
        shader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
            "shaders/Water_vs.glsl",
            "main",
            irr::video::EVST_VS_2_0,
            "shaders/Water_ps.glsl",
            "main",
            irr::video::EPST_PS_2_0,
            this, //For callbacks
            irr::video::EMT_SOLID
            );

    shader = shader==-1?0:shader; //Just in case something goes horribly wrong...

    //creating the cubemap... For now, Irrlicht's cubemaps need to be created on the fly out of images.
    //Loading the images
    cubemapImages[0] = driver->createImageFromFile("media/cube_ft.jpg");
    cubemapImages[1] = driver->createImageFromFile("media/cube_bk.jpg");
    cubemapImages[2] = driver->createImageFromFile("media/cube_up.jpg");
    cubemapImages[3] = driver->createImageFromFile("media/cube_dn.jpg");
    cubemapImages[4] = driver->createImageFromFile("media/cube_lf.jpg");
    cubemapImages[5] = driver->createImageFromFile("media/cube_rt.jpg");

    bool cubemapCreated = false;

    if (cubemapImages[0] && cubemapImages[1] && cubemapImages[2] && cubemapImages[3] && cubemapImages[4] && cubemapImages[5]) {

        //creating the cubemap itself
        cubemap = driver->addTextureCubemap(
            "irrlicht2.cubemap",
            cubemapImages[0],
            cubemapImages[1],
            cubemapImages[2],
            cubemapImages[3],
            cubemapImages[4],
            cubemapImages[5]
        );

        if (cubemap) {
            cubemapCreated = true;
        }


        //We're done with the images, so we're releasing them. Unlike the Textures, the Images can be dropped.
        for(int i=0;i<6;i++) {
            cubemapImages[i]->drop();
        }
    }
    //From Mel demo (http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=51130&start=15#p296723) END

	//FIXME: Hardcoded or defined in multiple places
	tileWidth = 100; //Width in metres - Note this is used in Simulation model normalisation as 100, so visible jumps in water are minimised
    segments = 32; //How many tiles per segment
    irr::f32 segmentSize = tileWidth / segments;

    ocean = new cOcean(segments, 0.00005f, vector2(32.0f,32.0f), tileWidth); //Note that the A and w parameters will get overwritten by ocean->resetParameters() dependent on the model's weather

	mesh = mgr->addHillPlaneMesh( "myHill",
                           core::dimension2d<f32>(segmentSize,segmentSize),
                           core::dimension2d<u32>(segments,segments),
                           0,
                           0.0f,
                           core::dimension2d<f32>(0,0),
                           core::dimension2d<f32>(tileWidth/(f32)segments,tileWidth/(f32)segments));

    /*
    //For testing, make wireframe
    for (u32 i=0; i<mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = mesh->getMeshBuffer(i);
        if (mb)
        {
            mb->getMaterial().setFlag(video::EMF_WIREFRAME, true);
        }
    }
    */



    for (u32 i=0; i<mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = mesh->getMeshBuffer(i);
        if (mb)
        {
            mb->getMaterial().setTexture(0,driver->getTexture("media/water.bmp"));
            if (cubemapCreated) {
                mb->getMaterial().setTexture(1,cubemap);
                mb->getMaterial().MaterialType = (irr::video::E_MATERIAL_TYPE)shader;
            }
            mb->getMaterial().FogEnable = true;

        }
    }


    //Hard code bounding box to be large - we always want to render water, and we actually render multiple displaced copies of the mesh, so just getting the mesh bounding box isn't correct.
    //TODO: Look here if there's a problem with the water disappearing or if we implement collision with water.
    boundingBox = core::aabbox3d<f32>(-10000,-100,-10000,10000,100,10000);

}


//! destructor
MovingWaterSceneNode::~MovingWaterSceneNode()
{
	// Mesh is dropped in IMeshSceneNode destructor (??? FIXME: Probably not true!)
    delete ocean;

}

void MovingWaterSceneNode::resetParameters(float A, vector2 w)
{
    ocean->resetParameters(A,w);
}

void MovingWaterSceneNode::OnSetConstants(video::IMaterialRendererServices* services, s32 userData)
{
    //From Mel's cubemap demo
    if(firstRun) {
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
    //End from Mel's cubemap demo



    services->setPixelShaderConstant(services->getVertexShaderConstantID("LightLevel"), &LightLevel, 1);
}


//! frame
void MovingWaterSceneNode::OnRegisterSceneNode()
{
    //std::cout << "In OnRegisterSceneNode()" << std::endl;

	if (IsVisible) {
        SceneManager->registerNodeForRendering(this);
    }

    ISceneNode::OnRegisterSceneNode();
}

/*
void MovingWaterSceneNode::setVerticalScale(f32 scale)
{
    scaleFactorVertical = scale;
}
*/

void MovingWaterSceneNode::OnAnimate(u32 timeMs)
{
	//std::cout << "In OnAnimate()" << std::endl;
	if (mesh && IsVisible)
	{

        //Set light level
        video::SColorf ambientLight = this->getSceneManager()->getAmbientLight();
        LightLevel = (ambientLight.r + ambientLight.g + ambientLight.b) / 3.0; //Average

		const f32 time = timeMs / 1000.f;

		//Update the FFT Calculation
		ocean->evaluateWavesFFT(time);
		vertex_ocean* vertices = ocean->getVertices();

		const u32 meshBufferCount = mesh->getMeshBufferCount();

		for (u32 b=0; b<meshBufferCount; ++b)
		{
			const u32 vtxCnt = mesh->getMeshBuffer(b)->getVertexCount();

			for (u32 i=0; i<vtxCnt; ++i) {
				mesh->getMeshBuffer(b)->getPosition(i).X = -1*vertices[i].x; //Swap sign to maintain correct rotation order of vertices: TODO: Look at basic definition of X and Z coordinate system between water and FFTWave
				mesh->getMeshBuffer(b)->getPosition(i).Y = vertices[i].y;
				mesh->getMeshBuffer(b)->getPosition(i).Z = vertices[i].z;

				//Set normals (TODO: Disable normal calculation in FFT for speed)
				//mesh->getMeshBuffer(b)->getNormal(i).X = -1*vertices[i].nx;
				//mesh->getMeshBuffer(b)->getNormal(i).Y = vertices[i].ny;
				//mesh->getMeshBuffer(b)->getNormal(i).Z = vertices[i].nz;
            }
            //Manually recalculate normals
            SceneManager->getMeshManipulator()->recalculateNormals(mesh->getMeshBuffer(b));
        }// end for all mesh buffers
		mesh->setDirty(scene::EBT_VERTEX);
	}
	IMeshSceneNode::OnAnimate(timeMs);
}

f32 MovingWaterSceneNode::getWaveHeight(f32 relPosX, f32 relPosZ) const
{

    //Adjust relative position by 1/2 tile width


    //Get the wave height (not including tide height) at this position relative to the origin of the water
    f32 relPosXInternal = fmod(relPosX+tileWidth/2,tileWidth);
    f32 relPosZInternal = fmod(relPosZ+tileWidth/2,tileWidth);

    //TODO: Probably not needed?
    while (relPosXInternal < 0)
        relPosXInternal+=tileWidth;
    while (relPosZInternal < 0)
        relPosZInternal+=tileWidth;


    unsigned int xIndex = Utilities::round((f32)(segments+1)*relPosXInternal/tileWidth);
    unsigned int zIndex = Utilities::round((f32)(segments+1)*relPosZInternal/tileWidth);
    xIndex = (segments+1) - xIndex; //Sign of x is flipped when heights are applied!

    unsigned int overallIndex = (segments+1) * zIndex + xIndex; //This should be the index of the closest vertex

    vertex_ocean* vertices = ocean->getVertices();
    f32 localHeight = vertices[overallIndex].y; //TODO: Error checking here!

    //std::cout << "Index:"  << overallIndex << " RequestedX:" << relPosX << " RequestedZ:" << relPosZ << " VertexX:" << vertices[overallIndex].x << " VertexZ:" << vertices[overallIndex].z << std::endl;

    return localHeight;

}

void MovingWaterSceneNode::setMesh(IMesh* mesh)
{
    //std::cout << "In setMesh()" << std::endl;
}


void MovingWaterSceneNode::render()
{

    //std::cout << "In render()" << std::endl;

	if (!mesh || !driver) {
		std::cout << "Could not render" << std::endl;
		return;
    }

	//driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	for (u32 i=0; i<mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = mesh->getMeshBuffer(i);
        if (mb)
        {
            const video::SMaterial& material = mb->getMaterial();

            // only render transparent buffer if this is the transparent render pass
            // and solid only in solid pass: TODO: Does this need implementing?
            driver->setMaterial(material);

            core::vector3df basicPosition = AbsoluteTransformation.getTranslation();

            //Draw multiple copies of tileable water
            for (int j = -10; j<=10; j++) {
                for (int k = -10; k<=10; k++) {
                    AbsoluteTransformation.setTranslation(basicPosition + core::vector3df(j*tileWidth,0,k*tileWidth));
                    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
                    driver->drawMeshBuffer(mb);
                }
            }

            AbsoluteTransformation.setTranslation(basicPosition);

        } else {
            std::cout << "No meshbuffer to render" << std::endl;
        }
    }

}

const core::aabbox3d<f32>& MovingWaterSceneNode::getBoundingBox() const
{
    return boundingBox;
}

IMesh* MovingWaterSceneNode::getMesh(void)
{
    //std::cout << "In getMesh()" << std::endl;
    return mesh;
}

IShadowVolumeSceneNode* MovingWaterSceneNode::addShadowVolumeSceneNode(const IMesh* shadowMesh, s32 id, bool zfailmethod, f32 infinity)
{
    //std::cout << "In addShadowVolumeSceneNode()" << std::endl;
    return 0;
}

void MovingWaterSceneNode::setReadOnlyMaterials(bool readonly)
{
    //Ignored
    //std::cout << "In setReadOnlyMaterials()" << std::endl;
}

bool MovingWaterSceneNode::isReadOnlyMaterials() const
{
    //std::cout << "In isReadOnlyMaterials()" << std::endl;
    return true; //Fixme: Check!
}

void MovingWaterSceneNode::setMaterialTexture(u32 textureLayer, video::ITexture * texture)
{
    if (textureLayer >= video::MATERIAL_MAX_TEXTURES)
        return;

    for (u32 i = 0; i<mesh->getMeshBufferCount(); i++) {
        mesh->getMeshBuffer(i)->getMaterial().setTexture(textureLayer, texture);
    }
    //for (u32 i=0; i<getMaterialCount(); ++i)
    //    getMaterial(i).setTexture(textureLayer, texture);
}


}
}
