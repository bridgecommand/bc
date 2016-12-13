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

//NOTE: This uses a modified version of Irrlicht for the water surface scene node, which bases the waves
//on the absolute position, so you can tile multiple water nodes seamlessly.

#include <vector>

#include "Water.hpp"
#include "Utilities.hpp"

using namespace irr;

Water::Water()
{

}

Water::~Water()
{
    //dtor
}

void Water::load(irr::scene::ISceneManager* smgr, irr::f32 weather, bool advancedWater)
{
    this->advancedWater = advancedWater;

    if (advancedWater) {
        realisticWater = new RealisticWaterSceneNode(smgr, 4000, 4000, "./",irr::core::dimension2du(512, 512),smgr->getRootSceneNode());
        realisticWater->setMaterialFlag(video::EMF_FOG_ENABLE, true);
        realisticWater->setWaveHeight(weather);
        realisticWater->setWaveLength(std::max(0.01,weather/10.0));
    } else {
        irr::video::IVideoDriver* driver = smgr->getVideoDriver();

        //Set tile width
        //FIXME: Hardcoded or defined in multiple places
        tileWidth = 100; //Width in metres - Note this is used in Simulation model normalisation as 1000, so visible jumps in water are minimised
        irr::u32 segments = 100; //How many tiles per segment
        irr::f32 segmentSize = tileWidth / segments;

        //some water (from demo 8)
        waterNode = new irr::scene::MovingWaterSceneNode(0.25f,1.0f,4.0f,smgr->getRootSceneNode(),smgr,0);
        waterNode->setMaterialTexture(0, driver->getTexture("media/water.bmp"));
        waterNode->setMaterialFlag(video::EMF_FOG_ENABLE, true);
        //waterNode = smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.25f, 300.0f, 10.0f);
        //add secondary meshes around the central water mesh: Note - Irrlicht code has been modified to get the edges to match, by basing on absolute X,Z position.
        /*
        std::vector<irr::scene::ISceneNode*> secondaryWaterNodes;
        secondaryWaterNodes.push_back(smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.25f, 300.0f, 10.0f,waterNode,-1,core::vector3df(-1*tileWidth,0,-1*tileWidth)));
        secondaryWaterNodes.push_back(smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.25f, 300.0f, 10.0f,waterNode,-1,core::vector3df(-1*tileWidth,0, 0*tileWidth)));
        secondaryWaterNodes.push_back(smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.25f, 300.0f, 10.0f,waterNode,-1,core::vector3df(-1*tileWidth,0, 1*tileWidth)));
        secondaryWaterNodes.push_back(smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.25f, 300.0f, 10.0f,waterNode,-1,core::vector3df( 0*tileWidth,0,-1*tileWidth)));
        secondaryWaterNodes.push_back(smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.25f, 300.0f, 10.0f,waterNode,-1,core::vector3df( 0*tileWidth,0, 1*tileWidth)));
        secondaryWaterNodes.push_back(smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.25f, 300.0f, 10.0f,waterNode,-1,core::vector3df( 1*tileWidth,0,-1*tileWidth)));
        secondaryWaterNodes.push_back(smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.25f, 300.0f, 10.0f,waterNode,-1,core::vector3df( 1*tileWidth,0, 0*tileWidth)));
        secondaryWaterNodes.push_back(smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.25f, 300.0f, 10.0f,waterNode,-1,core::vector3df( 1*tileWidth,0, 1*tileWidth)));
        for(std::vector<irr::scene::ISceneNode*>::iterator it = secondaryWaterNodes.begin(); it != secondaryWaterNodes.end(); ++it) {
            (*it)->setMaterialTexture(0, driver->getTexture("media/water.bmp"));
            (*it)->setMaterialFlag(video::EMF_FOG_ENABLE, true);
        }
        */

        waterNode->setPosition(core::vector3df(0,-0.25f,0));

        waterNode->setMaterialTexture(0, driver->getTexture("media/water.bmp"));
        waterNode->setMaterialFlag(video::EMF_FOG_ENABLE, true);

    }

}

void Water::update(irr::f32 tideHeight, irr::core::vector3df viewPosition, u32 lightLevel, irr::f32 weather)
{
    if (advancedWater) {
        realisticWater->setPosition(core::vector3df(0,tideHeight,0));
        realisticWater->setWaveHeight(weather);
        realisticWater->setWaveLength(std::max(0.01,weather/10.0));
        f32 lightIntensity = lightLevel/256.0;
        realisticWater->setLightIntensity(lightIntensity);
    } else {
        //Round these to nearest segmentWidth
        f32 xPos = tileWidth * Utilities::round(viewPosition.X/tileWidth);
        f32 yPos = tideHeight;
        f32 zPos = tileWidth * Utilities::round(viewPosition.Z/tileWidth);

        waterNode->setPosition(core::vector3df(xPos,yPos,zPos));

        //scale with weather
        waterNode->setScale(core::vector3df(1.0,weather,1.0)); //This also scales the child nodes
    }

}
