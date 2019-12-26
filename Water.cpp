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
#include <iostream>

#include "Water.hpp"
#include "Utilities.hpp"

//using namespace irr;

Water::Water()
{

}

Water::~Water()
{
    //dtor
}

void Water::load(irr::scene::ISceneManager* smgr, irr::f32 weather, irr::u32 disableShaders)
{

    irr::video::IVideoDriver* driver = smgr->getVideoDriver();

    //Set tile width
    //FIXME: Hardcoded or defined in multiple places
    tileWidth = 100; //Width in metres - Note this is used in Simulation model normalisation as 1000, so visible jumps in water are minimised


    waterNode = new irr::scene::MovingWaterSceneNode(smgr->getRootSceneNode(),smgr,0,disableShaders);

    //waterNode->setPosition(irr::core::vector3df(0,-0.25f,0));

    waterNode->setMaterialTexture(0, driver->getTexture("media/water.bmp"));

}

void Water::update(irr::f32 tideHeight, irr::core::vector3df viewPosition, irr::u32 lightLevel, irr::f32 weather)
{
    //Round these to nearest tileWidth
    irr::f32 xPos = tileWidth * Utilities::round(viewPosition.X/tileWidth);
    irr::f32 yPos = tideHeight;
    irr::f32 zPos = tileWidth * Utilities::round(viewPosition.Z/tileWidth);

    //std::cout << "xPos: " << xPos << " yPos: " << yPos << " zPos: " << zPos << std::endl;

    waterNode->setPosition(irr::core::vector3df(xPos,yPos,zPos));

    //scale with weather
    //waterNode->setVerticalScale(sqrt(weather));
    waterNode->resetParameters((weather+0.25)*0.000025f, vector2((weather+0.25)/12.0*32.0f,(weather+0.25)/12.0*32.0f),weather+0.25); //TODO: Work out what this relationship should be!

}

irr::f32 Water::getWaveHeight(irr::f32 relPosX, irr::f32 relPosZ) const
{
    return waterNode->getWaveHeight(relPosX,relPosZ);
}

irr::core::vector2df Water::getLocalNormals(irr::f32 relPosX, irr::f32 relPosZ) const
{
    return waterNode->getLocalNormals(relPosX,relPosZ);
}


irr::core::vector3df Water::getPosition() const
{
    return waterNode->getPosition();
}

void Water::setVisible(bool visible)
{
    waterNode->setVisible(visible);
}
