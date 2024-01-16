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
//#include "Utilities.hpp"

#include <iostream>
#include <cstdint>
//#include <cmath>

namespace irr
{
namespace scene
{

//! constructor
MovingWaterSceneNode::MovingWaterSceneNode(ISceneNode* parent, ISceneManager* mgr, ISceneNode* ownShip, irr::s32 id, irr::u32 disableShaders, bool withReflection, irr::u32 segments,
		const irr::core::vector3df& position, const irr::core::vector3df& rotation)
	//: IMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale),
	: IMeshSceneNode(parent, mgr, id, position, rotation, irr::core::vector3df(1.0f,1.0f,1.0f)), lightLevel(0.75), seaState(0.5), disableShaders(disableShaders), withReflection(withReflection), segments(segments)
{
	#ifdef _DEBUG
	setDebugName("MovingWaterSceneNode");
	#endif

	//scaleFactorVertical = 1.0;

	driver = mgr->getVideoDriver();
	ownShipSceneNode = ownShip;


	//From Mel demo (http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=51130&start=15#p296723) START
	irr::video::E_DRIVER_TYPE driverType = mgr->getVideoDriver()->getDriverType();

	IsOpenGL = (driverType==irr::video::EDT_OPENGL);
    firstRun = true;

	//cubemapConstants* cns = new cubemapConstants(driverType==irr::video::EDT_OPENGL);
    //So far there are no materials ready to use a cubemap, so we provide our own.
    irr::s32 shader=0;

	if (!disableShaders) {
		irr::io::path vertexShader;
		irr::io::path pixelShader;
		if (driverType == irr::video::EDT_DIRECT3D9) {
            //DirectX, not currently used
            vertexShader = "shaders/Water_vs.hlsl";
            pixelShader = "shaders/Water_ps.hlsl";
		} else {
            //OpenGL
            if (withReflection) {
                vertexShader = "shaders/Water_vs.glsl";
                pixelShader = "shaders/Water_ps.glsl";
            } else {
                vertexShader = "shaders/Water_vs_noReflection.glsl";
                pixelShader = "shaders/Water_ps_noReflection.glsl";
            }
            
		}
		
		shader = driver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(
				vertexShader,
				"main",
				irr::video::EVST_VS_2_0,
				pixelShader,
				"main",
				irr::video::EPST_PS_2_0,
				this, //For callbacks
				irr::video::EMT_SOLID
        );
	}
    shader = shader==-1?0:shader; //Just in case something goes horribly wrong...

	//FIXME: Hardcoded or defined in multiple places
	tileWidth = 100; //Width in metres - Note this is used in Simulation model normalisation as 100, so visible jumps in water are minimised
    irr::f32 segmentSize = tileWidth / segments;

    ocean = new cOcean(segments, 0.00005f, vector2(32.0f,32.0f), tileWidth); //Note that the A and w parameters will get overwritten by ocean->resetParameters() dependent on the model's weather

	mesh = mgr->addHillPlaneMesh( "myHill",
                           irr::core::dimension2d<irr::f32>(segmentSize,segmentSize),
                           irr::core::dimension2d<irr::u32>(segments,segments),
                           0,
                           0.0f,
                           irr::core::dimension2d<irr::f32>(0,0),
                           irr::core::dimension2d<irr::f32>(tileWidth/(irr::f32)(segments),tileWidth/(irr::f32)(segments)));


    flatMesh = mgr->getMesh("media/flatsea.x");
    if (!flatMesh) {
        std::cerr << "Could not load flat sea mesh from media/flatsea.x" << std::endl;
        exit(EXIT_FAILURE);
    }


    //For testing, make wireframe
    /*
    for (irr::u32 i=0; i<mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = mesh->getMeshBuffer(i);
        if (mb)
        {
            mb->getMaterial().setFlag(video::EMF_WIREFRAME, true);
        }
    }
    */

    //Create local camera for reflections
    _camera = 0;
    _reflectionMap = 0;
    
	if (!disableShaders) {
		if (withReflection) {
            _camera = mgr->addCameraSceneNode(0, irr::core::vector3df(0, 0, 0), irr::core::vector3df(0, 0, 0), -1, false);
            _reflectionMap = driver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(512, 512)); //TODO: Check hardcoding here
		}
		
		irr::video::ITexture* bumpTexture = driver->getTexture("/media/waterbump.png");

		for (irr::u32 i = 0; i < mesh->getMeshBufferCount(); ++i)
		{
			scene::IMeshBuffer* mb = mesh->getMeshBuffer(i);
			if (mb)
			{
				mb->getMaterial().setTexture(0, bumpTexture);
				if (withReflection) {
                    mb->getMaterial().setTexture(1, _reflectionMap);
				}
				mb->getMaterial().MaterialType = (irr::video::E_MATERIAL_TYPE)shader;
				mb->getMaterial().FogEnable = true;
			}
		}


		for (irr::u32 i = 0; i < flatMesh->getMeshBufferCount(); ++i)
		{
			scene::IMeshBuffer* mb = flatMesh->getMeshBuffer(i);
			if (mb)
			{
				mb->getMaterial().setTexture(0, bumpTexture);
				if (withReflection) {
                    mb->getMaterial().setTexture(1, _reflectionMap);
                }
				mb->getMaterial().MaterialType = (irr::video::E_MATERIAL_TYPE)shader;
				mb->getMaterial().FogEnable = true;

			}
		}
	}


	if (disableShaders) {
		for (irr::u32 i = 0; i < mesh->getMeshBufferCount(); ++i)
		{
			scene::IMeshBuffer* mb = mesh->getMeshBuffer(i);
			if (mb)
			{
				mb->getMaterial().FogEnable = true;
			}
		}


		for (irr::u32 i = 0; i < flatMesh->getMeshBufferCount(); ++i)
		{
			scene::IMeshBuffer* mb = flatMesh->getMeshBuffer(i);
			if (mb)
			{
				mb->getMaterial().FogEnable = true;
			}
		}
	}

    //Hard code bounding box to be large - we always want to render water, and we actually render multiple displaced copies of the mesh, so just getting the mesh bounding box isn't correct.
    //TODO: Look here if there's a problem with the water disappearing or if we implement collision with water.
    boundingBox = irr::core::aabbox3d<irr::f32>(-10000,-100,-10000,10000,100,10000);

}


//! destructor
MovingWaterSceneNode::~MovingWaterSceneNode()
{
	// Mesh is dropped in IMeshSceneNode destructor (??? FIXME: Probably not true!)
    delete ocean;

    if (_camera)
	{
		_camera->drop();
		_camera = NULL;
	}

	if (_reflectionMap)
	{
		_reflectionMap->drop();
		_reflectionMap = NULL;
	}

}

//From OpenCV via http://stackoverflow.com/a/20723890
int MovingWaterSceneNode::localisinf(double x) const
{
    union { uint64_t u; double f; } ieee754;
    ieee754.f = x;
    return ( (unsigned)(ieee754.u >> 32) & 0x7fffffff ) == 0x7ff00000 &&
           ( (unsigned)ieee754.u == 0 );
}

int MovingWaterSceneNode::localisnan(double x) const
{
    union { uint64_t u; double f; } ieee754;
    ieee754.f = x;
    return ( (unsigned)(ieee754.u >> 32) & 0x7fffffff ) +
           ( (unsigned)ieee754.u != 0 ) > 0x7ff00000;
}
//End From OpenCV via http://stackoverflow.com/a/20723890


void MovingWaterSceneNode::resetParameters(float A, vector2 w, float seaState)
{
    ocean->resetParameters(A,w);
    this->seaState = seaState;
}

void MovingWaterSceneNode::OnSetConstants(video::IMaterialRendererServices* services, irr::s32 userData)
{
    //From Mel's cubemap demo
	if (!disableShaders) {
		if (firstRun) {
			firstRun = false;

			driver = services->getVideoDriver();
			//Looking for our constants IDs...
			matViewInverse = services->getVertexShaderConstantID("matViewInverse");
			if (withReflection) {
                matWorldReflectionViewProj = services->getVertexShaderConstantID("WorldReflectionViewProj");
			}
			idLightLevel = services->getVertexShaderConstantID("lightLevel");
			idSeaState = services->getVertexShaderConstantID("seaState");

			if (IsOpenGL)
			{
				if (withReflection) {
                    baseMap = services->getPixelShaderConstantID("baseMap");
                    reflectionMap = services->getPixelShaderConstantID("reflectionMap");
				}
			}
			else
			{
				matWorldViewProjection = services->getVertexShaderConstantID("matWorldViewProjection");
				matWorld = services->getVertexShaderConstantID("matWorld");
			}
		}

		//Setting up our constants...
		irr::core::matrix4 mat;

		mat = driver->getTransform(irr::video::ETS_VIEW);
		mat.makeInverse();
		services->setVertexShaderConstant(matViewInverse, mat.pointer(), 16);

		if (withReflection) {
            irr::core::matrix4 worldReflectionViewProj = driver->getTransform(video::ETS_PROJECTION);
            worldReflectionViewProj *= _camera->getViewMatrix();;
            worldReflectionViewProj *= driver->getTransform(video::ETS_WORLD);
            services->setVertexShaderConstant(matWorldReflectionViewProj, worldReflectionViewProj.pointer(), 16);
		}

		if (IsOpenGL)
		{
			int sampler = 0;
			if (withReflection) {
                services->setPixelShaderConstant(baseMap, &sampler, 1);
			}
			sampler = 1;
			if (withReflection) {
                services->setPixelShaderConstant(reflectionMap, &sampler, 1);
			}
			services->setPixelShaderConstant(idLightLevel, &lightLevel, 1);
			services->setPixelShaderConstant(idSeaState, &seaState, 1);
		}
		else
		{
			mat = driver->getTransform(irr::video::ETS_PROJECTION);
			mat *= driver->getTransform(irr::video::ETS_VIEW);
			mat *= driver->getTransform(irr::video::ETS_WORLD);
			services->setVertexShaderConstant(matWorldViewProjection, mat.pointer(), 16);

			mat = driver->getTransform(irr::video::ETS_WORLD);
			services->setVertexShaderConstant(matWorld, mat.pointer(), 16);
		}
		//End from Mel's cubemap demo
	}

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
void MovingWaterSceneNode::setVerticalScale(irr::f32 scale)
{
    scaleFactorVertical = scale;
}
*/

void MovingWaterSceneNode::OnAnimate(irr::u32 timeMs)
{
	//std::cout << "In OnAnimate()" << std::endl;
	if (mesh && IsVisible)
	{

        //Set light level
        video::SColorf ambientLight = this->getSceneManager()->getAmbientLight();
        lightLevel = (ambientLight.r + ambientLight.g + ambientLight.b) / 3.0; //Average

		const irr::f32 time = timeMs / 1000.f;

		//Update the FFT Calculation
		ocean->evaluateWavesFFT(time);
		vertex_ocean* vertices = ocean->getVertices();

		const irr::u32 meshBufferCount = mesh->getMeshBufferCount();

		for (irr::u32 b=0; b<meshBufferCount; ++b)
		{
			const irr::u32 vtxCnt = mesh->getMeshBuffer(b)->getVertexCount();

			for (irr::u32 i=0; i<vtxCnt; ++i) {
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
	//Fixme: Need to store timeMs in something accessible to the shader for ripples

	//Render reflection to texture
	if (IsVisible && !disableShaders)
	{
		
		if (withReflection) {
            //fixes glitches with incomplete refraction
            const irr::f32 CLIP_PLANE_OFFSET_Y = 0.0f;

            irr::core::rect<irr::s32> currentViewPort = driver->getViewPort(); //Get the previous viewPort

            setVisible(false); //hide the water

            bool reShowOwnShip = false;

            if (ownShipSceneNode->isVisible()) {
                ownShipSceneNode->setVisible(false);
                reShowOwnShip = true;
            }

            //reflection
            driver->setRenderTarget(_reflectionMap, irr::video::ECBF_COLOR|irr::video::ECBF_DEPTH); //render to reflection

            //get current camera
            scene::ICameraSceneNode* currentCamera = SceneManager->getActiveCamera();
            irr::f32 currentAspect = currentCamera->getAspectRatio();

            //use this aspect ratio
            _camera->setAspectRatio(currentAspect);

            //set FOV and far value from current camera
            _camera->setFarValue(currentCamera->getFarValue());
            irr::f32 renderScale = 1.5; //This matches the scaling in the shader, to avoid artefacts near the edge of the screen
            irr::f32 renderFOV = 2*atan(renderScale * tan(currentCamera->getFOV()/2));
            _camera->setFOV(renderFOV);

            irr::core::vector3df position = currentCamera->getAbsolutePosition();
            position.Y = -position.Y + 2 * RelativeTranslation.Y; //position of the water
            _camera->setPosition(position);

            irr::core::vector3df target = currentCamera->getTarget();

            //invert Y position of current camera
            target.Y = -target.Y + 2 * RelativeTranslation.Y;
            _camera->setTarget(target);

            //set the reflection camera
            SceneManager->setActiveCamera(_camera);

            //reflection clipping plane
            irr::core::plane3d<irr::f32> reflectionClipPlane(0, RelativeTranslation.Y - CLIP_PLANE_OFFSET_Y, 0, 0, 1, 0);
            driver->setClipPlane(0, reflectionClipPlane, true);

            SceneManager->drawAll(); //draw the scene

            //disable clip plane
            driver->enableClipPlane(0, false);

            //set back old render target
            driver->setRenderTarget(0, 0);

            //set back the active camera
            SceneManager->setActiveCamera(currentCamera);

            setVisible(true); //show it again

            if (reShowOwnShip) {
                ownShipSceneNode->setVisible(true);
            }

            //Reset :: Fixme: Doesn't seem to be working on old PC
            driver->setViewPort(irr::core::rect<irr::s32>(0,0,10,10));//Set to a dummy value first to force the next call to make the change
            driver->setViewPort(currentViewPort);
            currentCamera->setAspectRatio(currentAspect);
		}
	}
}

irr::f32 MovingWaterSceneNode::getWaveHeight(irr::f32 relPosX, irr::f32 relPosZ) const
{

    //Adjust relative position by 1/2 tile width


    //Get the wave height (not including tide height) at this position relative to the origin of the water
    irr::f32 relPosXInternal = fmod(relPosX+tileWidth/2,tileWidth);
    irr::f32 relPosZInternal = fmod(relPosZ+tileWidth/2,tileWidth);

    //TODO: Probably not needed?
    while (relPosXInternal < 0)
        relPosXInternal+=tileWidth;
    while (relPosZInternal < 0)
        relPosZInternal+=tileWidth;

    irr::f32 xIndexFloat = (irr::f32)(segments+1)*relPosXInternal/tileWidth;
    irr::f32 zIndexFloat = (irr::f32)(segments+1)*relPosZInternal/tileWidth;
    xIndexFloat = (segments+1) - xIndexFloat; //Sign of x is flipped when heights are applied!

    //std::cout << "xIndexF:" << xIndexFloat << " zIndexF:" << zIndexFloat << " segments+1:" << segments+1 << std::endl;

    //Bilinear interpolation
    unsigned int xIndex0 = floor(xIndexFloat);
    unsigned int zIndex0 = floor(zIndexFloat);
    unsigned int xIndex1 = ceil(xIndexFloat);
    unsigned int zIndex1 = ceil(zIndexFloat);

    //If any indexes are equal to segments+1, set to 0 (as sea tiles)
    if (xIndex0 == (segments+1)) {xIndex0=0;}
    if (zIndex0 == (segments+1)) {zIndex0=0;}
    if (xIndex1 == (segments+1)) {xIndex1=0;}
    if (zIndex1 == (segments+1)) {zIndex1=0;}

    irr::f32 interpX = xIndexFloat - xIndex0;
    irr::f32 interpZ = zIndexFloat - zIndex0;

    unsigned int index00 = (segments+1) * zIndex0 + xIndex0;
    unsigned int index01 = (segments+1) * zIndex1 + xIndex0;
    unsigned int index10 = (segments+1) * zIndex0 + xIndex1;
    unsigned int index11 = (segments+1) * zIndex1 + xIndex1;

    vertex_ocean* vertices = ocean->getVertices();

    //Error checking here?
    irr::f32 height00 = vertices[index00].y;
    irr::f32 height01 = vertices[index01].y;
    irr::f32 height10 = vertices[index10].y;
    irr::f32 height11 = vertices[index11].y;


    irr::f32 localHeight = height00*(1-interpX)*(1-interpZ) + height10*interpX*(1-interpZ) + height01*(1-interpX)*interpZ + height11*interpX*interpZ;

    if (localisnan(localHeight) || localisinf(localHeight)) {
        return 0;
    } else {
        return localHeight;
    }

}

irr::core::vector2df MovingWaterSceneNode::getLocalNormals(irr::f32 relPosX, irr::f32 relPosZ) const
{

    //Adjust relative position by 1/2 tile width

    //Get the wave normal
    irr::f32 relPosXInternal = fmod(relPosX+tileWidth/2,tileWidth);
    irr::f32 relPosZInternal = fmod(relPosZ+tileWidth/2,tileWidth);

    //TODO: Probably not needed?
    while (relPosXInternal < 0)
        relPosXInternal+=tileWidth;
    while (relPosZInternal < 0)
        relPosZInternal+=tileWidth;

    irr::f32 xIndexFloat = (irr::f32)(segments+1)*relPosXInternal/tileWidth;
    irr::f32 zIndexFloat = (irr::f32)(segments+1)*relPosZInternal/tileWidth;
    xIndexFloat = (segments+1) - xIndexFloat; //Sign of x is flipped when heights are applied!

    //std::cout << "xIndexF:" << xIndexFloat << " zIndexF:" << zIndexFloat << " segments+1:" << segments+1 << std::endl;

    //Bilinear interpolation
    unsigned int xIndex0 = floor(xIndexFloat);
    unsigned int zIndex0 = floor(zIndexFloat);
    unsigned int xIndex1 = ceil(xIndexFloat);
    unsigned int zIndex1 = ceil(zIndexFloat);

    //If any indexes are equal to segments+1, set to 0 (as sea tiles)
    if (xIndex0 == (segments+1)) {xIndex0=0;}
    if (zIndex0 == (segments+1)) {zIndex0=0;}
    if (xIndex1 == (segments+1)) {xIndex1=0;}
    if (zIndex1 == (segments+1)) {zIndex1=0;}

    irr::f32 interpX = xIndexFloat - xIndex0;
    irr::f32 interpZ = zIndexFloat - zIndex0;

    unsigned int index00 = (segments+1) * zIndex0 + xIndex0;
    unsigned int index01 = (segments+1) * zIndex1 + xIndex0;
    unsigned int index10 = (segments+1) * zIndex0 + xIndex1;
    unsigned int index11 = (segments+1) * zIndex1 + xIndex1;

    vertex_ocean* vertices = ocean->getVertices();

    //Error checking here?
    irr::f32 nx00 = vertices[index00].nx;
    irr::f32 nx01 = vertices[index01].nx;
    irr::f32 nx10 = vertices[index10].nx;
    irr::f32 nx11 = vertices[index11].nx;

    irr::f32 nz00 = vertices[index00].nz;
    irr::f32 nz01 = vertices[index01].nz;
    irr::f32 nz10 = vertices[index10].nz;
    irr::f32 nz11 = vertices[index11].nz;

    irr::f32 localNx = nx00*(1-interpX)*(1-interpZ) + nx10*interpX*(1-interpZ) + nx01*(1-interpX)*interpZ + nx11*interpX*interpZ;
    irr::f32 localNz = nz00*(1-interpX)*(1-interpZ) + nz10*interpX*(1-interpZ) + nz01*(1-interpX)*interpZ + nz11*interpX*interpZ;

    if (localisnan(localNx) || localisinf(localNx) || localisnan(localNz) || localisinf(localNz)) {
        return irr::core::vector2df(0,0);
    } else {
        return irr::core::vector2df(localNx,localNz);
    }

}

void MovingWaterSceneNode::setMesh(IMesh* mesh)
{
    //std::cout << "In setMesh()" << std::endl;
}


void MovingWaterSceneNode::render()
{

    //std::cout << "In render()" << std::endl;

	if (!mesh || !driver) {
		std::cerr << "Could not render" << std::endl;
		return;
    }

	//driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	//Draw main water
	for (irr::u32 i=0; i<mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = mesh->getMeshBuffer(i);
        if (mb)
        {
            const video::SMaterial& material = mb->getMaterial();

            // only render transparent buffer if this is the transparent render pass
            // and solid only in solid pass: TODO: Does this need implementing?
            driver->setMaterial(material);

            irr::core::vector3df basicPosition = AbsoluteTransformation.getTranslation();

            //Draw multiple copies of tileable water
            for (int j = -10; j<=10; j++) {
                for (int k = -10; k<=10; k++) {
                    AbsoluteTransformation.setTranslation(basicPosition + irr::core::vector3df(j*tileWidth,0,k*tileWidth));
                    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
                    driver->drawMeshBuffer(mb);
                }
            }

            AbsoluteTransformation.setTranslation(basicPosition);

        } else {
            std::cerr << "No meshbuffer to render" << std::endl;
        }
    }

    //Draw flat sea beyond the animated sea
	for (irr::u32 i=0; i<flatMesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = flatMesh->getMeshBuffer(i);
        if (mb)
        {
            const video::SMaterial& material = mb->getMaterial();

            // only render transparent buffer if this is the transparent render pass
            // and solid only in solid pass: TODO: Does this need implementing?
            driver->setMaterial(material);

            driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
            driver->drawMeshBuffer(mb);


        } else {
            std::cerr << "No meshbuffer to render" << std::endl;
        }
    }

}

const irr::core::aabbox3d<irr::f32>& MovingWaterSceneNode::getBoundingBox() const
{
    return boundingBox;
}

IMesh* MovingWaterSceneNode::getMesh(void)
{
    //std::cerr << "In getMesh()" << std::endl;
    return mesh;
}

IShadowVolumeSceneNode* MovingWaterSceneNode::addShadowVolumeSceneNode(const IMesh* shadowMesh, irr::s32 id, bool zfailmethod, irr::f32 infinity)
{
    //std::cerr << "In addShadowVolumeSceneNode()" << std::endl;
    return 0;
}

void MovingWaterSceneNode::setReadOnlyMaterials(bool readonly)
{
    //Ignored
    //std::cerr << "In setReadOnlyMaterials()" << std::endl;
}

bool MovingWaterSceneNode::isReadOnlyMaterials() const
{
    //std::cout << "In isReadOnlyMaterials()" << std::endl;
    return true; //Fixme: Check!
}

void MovingWaterSceneNode::setMaterialTexture(irr::u32 textureLayer, video::ITexture * texture)
{
    if (textureLayer >= video::MATERIAL_MAX_TEXTURES)
        return;

    for (irr::u32 i = 0; i<mesh->getMeshBufferCount(); i++) {
        mesh->getMeshBuffer(i)->getMaterial().setTexture(textureLayer, texture);
    }

	//also set for far mesh
	for (irr::u32 i = 0; i<flatMesh->getMeshBufferCount(); i++) {
		flatMesh->getMeshBuffer(i)->getMaterial().setTexture(textureLayer, texture);
	}
    //for (irr::u32 i=0; i<getMaterialCount(); ++i)
    //    getMaterial(i).setTexture(textureLayer, texture);
}


}
}
