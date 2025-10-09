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

#include "Constants.hpp"
#include "SimulationModel.hpp"
#include "IniFile.hpp"

//using namespace irr;

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

int Ship::setShipParams(const std::string& aType)
{
  int ret = 0;

  /*TODO : Get datas from parameters file*/
  if("kvlcc2" == aType)
    {
      mRho = 1025;
      mMu0 << 0, 0, 0;
      mGeoParams = {320, 58, 20.8, 312622, 11.1, 0.81};
      //mHullDerParams = {0.022,-0.04, 0.002, 0.011, 0.771, -0.315, 0.083, -1.607, 0.379, -0.391, 0.008, -0.137, -0.049, -0.03, -0.294, 0.055, -0.013};
      //mAddedMassParams = {0.022, 0.223, 0.011};
      mProp.Init(9.86, 0.22, -0.48, 0.35, 0.293, -0.275, -0.139 /*,1.53*/);
      //mRudderParams = {15.8, 112.5, -0.5, 0.312, 0.387, -0.464, 1.09, 0.5, -0.71, 1.827, {0.395, 0.64}, 0.0407};
      // mShipWindParams = {4910, 1624, 750, 375, 160, 0, 1.225, 0 * 15.5 * 0.514, 90};
    }
  else
    {
      ret = -1;
    }

  return ret;
}


irr::scene::IMeshSceneNode* Ship::getSceneNode() const
{
    return (irr::scene::IMeshSceneNode*)ship;
}

irr::core::vector3df Ship::getRotation() const
{
    return ship->getRotation();
}

irr::core::vector3df Ship::getPosition() const
{
    ship->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return ship->getAbsolutePosition();
}

irr::f32 Ship::getLength() const
{
    return length;
}

irr::f32 Ship::getBreadth() const
{
    return breadth;
}

irr::f32 Ship::getHeightCorrection() const
{
    return heightCorrection;
}

irr::f32 Ship::getEstimatedDisplacement() const
{
    irr::f32 seawaterDensity = 1024; // define seawater density in kg / m^3 could parametarise this for dockwater and freshwater
    irr::f32 typicalBlockCoefficient = 0.87; // 0.87 is typical block coefficient
    return draught * breadth * length * seawaterDensity * typicalBlockCoefficient; 
}

void Ship::setPosition(irr::f32 xPos, irr::f32 zPos)
{
     //Update the position used, ready for next update. Doesn't actually move the mesh at this point
     this->xPos = xPos;
     this->zPos = zPos;
     positionManuallyUpdated = true;
}

void Ship::setHeading(irr::f32 hdg)
{
    this->hdg = hdg;
    controlMode = MODE_AUTO; //Switch to auto mode
}

void Ship::setSpeed(irr::f32 spd)
{
    this->axialSpd = spd;
    controlMode = MODE_AUTO; //Switch to auto mode
}

irr::f32 Ship::getHeading() const
{
    return hdg;
}

irr::f32 Ship::getSpeed() const
{
    return axialSpd;
}

irr::u32 Ship::getMMSI() const
{
    return mmsi;
}

void Ship::setMMSI(irr::u32 mmsi)
{
    this->mmsi = mmsi;
}

void Ship::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    xPos += deltaX;
    yPos += deltaY;
    zPos += deltaZ;
    ship->setPosition(irr::core::vector3df(xPos,yPos,zPos));
}



Propeller& Ship::getProp(void){return mProp;}
//Hull& Ship::getHull(void){return mHull;}
//Rudder& Ship::getRudd(void){return mRudd;}
//Wind& Ship::getWind(void){return mWind;}

sGeoParams& Ship::getGeoParams(void){return mGeoParams;}

double Ship::getRho(void){return mRho;}
double Ship::getM(void){return mM;}
double Ship::getMX(void){return mMX;}
double Ship::getMY(void){return mMY;}
Eigen::Vector3d& Ship::getMu0(void){return mMu0;}
Eigen::Vector3d& Ship::getMu(void){return mMu;}
//Eigen::Vector3d& Ship::getEta(void){return mEta;}
Eigen::Matrix3d& Ship::getInvMatM(void){return mInvMatM;}
