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

void Water::load(irr::scene::ISceneManager* smgr)
{
    irr::video::IVideoDriver* driver = smgr->getVideoDriver();

    //Set tile width
    tileWidth = 1000; //Width in metres - Note this is used in Simulation model normalisation as 1000, so visible jumps in water are minimised
    irr::u32 segments = 10; //How many tiles per segment
    irr::f32 segmentSize = tileWidth / segments;

    //some water (from demo 8)
    scene::IAnimatedMesh* waterMesh = smgr->addHillPlaneMesh( "myHill",
                       core::dimension2d<f32>(segmentSize,segmentSize),
                       core::dimension2d<u32>(segments,segments),
                       0,
                       0.0f,
                       core::dimension2d<f32>(0,0),
                       core::dimension2d<f32>(10,10));

    waterNode = smgr->addWaterSurfaceSceneNode(waterMesh->getMesh(0), 0.25f, 300.0f, 10.0f);
    //add secondary meshes around the central water mesh: Note - Irrlicht code has been modified to get the edges to match, by basing on absolute X,Z position.
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

    waterNode->setPosition(core::vector3df(0,-0.25f,0));

    waterNode->setMaterialTexture(0, driver->getTexture("media/water.bmp"));
    waterNode->setMaterialFlag(video::EMF_FOG_ENABLE, true);
}

void Water::update(irr::f32 tideHeight, irr::core::vector3df viewPosition)
{

    //Round these to nearest segmentWidth
    f32 xPos = tileWidth * Utilities::round(viewPosition.X/tileWidth);
    f32 yPos = tideHeight;
    f32 zPos = tileWidth * Utilities::round(viewPosition.Z/tileWidth);

    waterNode->setPosition(core::vector3df(xPos,yPos,zPos));

}
