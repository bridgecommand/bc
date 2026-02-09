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

//Parent class for own and other ships - not used un-extended
#include "Ship.hpp"

#include "irrlicht.h"
#include "Constants.hpp"
#include "SimulationModel.hpp"
#include "IniFile.hpp"

//using namespace irr;

namespace {
    inline bc::graphics::Vec3 fromIrrVec(const irr::core::vector3df& v) { return {v.X, v.Y, v.Z}; }
}

Ship::Ship()
{
    //Default to run on defined spd and hdg
    controlMode = MODE_AUTO;
    positionManuallyUpdated = false; //Used to track if position has been manually updated, and shouldn't have position update applied this loop
    mmsi = 0;
}

Ship::~Ship()
{
    //dtor
}

irr::scene::IMeshSceneNode* Ship::getSceneNode() const
{
    return (irr::scene::IMeshSceneNode*)ship;
}

bc::graphics::Vec3 Ship::getRotation() const
{
    return fromIrrVec(ship->getRotation());
}

bc::graphics::Vec3 Ship::getPosition() const
{
    ship->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return fromIrrVec(ship->getAbsolutePosition());
}

float Ship::getLength() const
{
    return length;
}

float Ship::getBreadth() const
{
    return breadth;
}

float Ship::getHeightCorrection() const
{
    return heightCorrection;
}

float Ship::getEstimatedDisplacement() const
{
    float seawaterDensity = 1024; // define seawater density in kg / m^3 could parametarise this for dockwater and freshwater
    float typicalBlockCoefficient = 0.87; // 0.87 is typical block coefficient
    return draught * breadth * length * seawaterDensity * typicalBlockCoefficient; 
}

void Ship::setPosition(float xPos, float zPos)
{
     //Update the position used, ready for next update. Doesn't actually move the mesh at this point
     this->xPos = xPos;
     this->zPos = zPos;
     positionManuallyUpdated = true;
}

void Ship::setHeading(float hdg)
{
    this->hdg = hdg;
    controlMode = MODE_AUTO; //Switch to auto mode
}

void Ship::setSpeed(float spd)
{
    this->axialSpd = spd;
    controlMode = MODE_AUTO; //Switch to auto mode
}

float Ship::getHeading() const
{
    return hdg;
}

float Ship::getSpeed() const
{
    return axialSpd;
}

uint32_t Ship::getMMSI() const
{
    return mmsi;
}

void Ship::setMMSI(uint32_t mmsi)
{
    this->mmsi = mmsi;
}

void Ship::moveNode(float deltaX, float deltaY, float deltaZ)
{
    xPos += deltaX;
    yPos += deltaY;
    zPos += deltaZ;
    ship->setPosition(irr::core::vector3df(xPos,yPos,zPos));
}



