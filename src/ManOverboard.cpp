/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2017 James Packer

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

#include <iostream>
#include "ManOverboard.hpp"
#include "IniFile.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"
#include "Water.hpp"
#include "Wind.hpp"
#include "Tide.hpp"
#include "Terrain.hpp"

//using namespace irr;
ManOverboard::ManOverboard()
{
  
}

ManOverboard::~ManOverboard()
{
  
}

void ManOverboard::load(const irr::core::vector3df& aLocation, Terrain* aTerrain, Water* aWater, Wind* aWind, Tide* aTide, irr::IrrlichtDevice* aDev)
{
  
  mTerrain=aTerrain;
  mWater=aWater;
  mWind=aWind;
  mTide=aTide;

  irr::scene::ISceneManager* smgr = aDev->getSceneManager();

  
  std::string basePath = "models/ManOverboard/";
  std::string userFolder = Utilities::getUserDir();
  //Read model from user dir if it exists there.
  if (Utilities::pathExists(userFolder + basePath)) {
    basePath = userFolder + basePath;
  }

  //Load from individual ManOverboard.ini file if it exists
  std::string mobIniFilename = basePath + "ManOverboard.ini";

  //get filename from ini file (or empty string if file doesn't exist)
  std::string mobFileName = IniFile::iniFileToString(mobIniFilename,"FileName", "ManOverboard.x");

  //get scale factor from ini file (or zero if not set - assume 1)
  irr::f32 mobScale = IniFile::iniFileTof32(mobIniFilename,"Scalefactor", 1.f);

  //The path to the actual model file
  std::string mobFullPath = basePath + mobFileName;

  //Load the mesh
  irr::scene::IMesh* mobMesh = smgr->getMesh(mobFullPath.c_str());

  //add to scene node
  if (mobMesh==0) {
    //Failed to load mesh - load with dummy and continue
    aDev->getLogger()->log("Failed to load man overboard model:");
    aDev->getLogger()->log(mobFullPath.c_str());
    man = smgr->addCubeSceneNode(0.1);
  } else {
    man = smgr->addMeshSceneNode( mobMesh, 0, -1, aLocation );
  }

  //Set lighting to use diffuse and ambient, so lighting of untextured models works
  if(man->getMaterialCount()>0) {
    for(irr::u32 mat=0;mat<man->getMaterialCount();mat++) {
      man->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
    }
  }

  man->setScale(irr::core::vector3df(mobScale,mobScale,mobScale));
  man->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
  man->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting

}


irr::core::vector3df ManOverboard::getPosition() const
{
  man->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
  return man->getAbsolutePosition();
}

irr::scene::ISceneNode* ManOverboard::getSceneNode() const
{
  return (irr::scene::ISceneNode*)man;
}

void ManOverboard::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
  irr::core::vector3df currentPos = man->getPosition();
  irr::f32 newPosX = currentPos.X + deltaX;
  irr::f32 newPosY = currentPos.Y + deltaY;
  irr::f32 newPosZ = currentPos.Z + deltaZ;

  man->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
}

void ManOverboard::update(sTime& aTime, irr::f32 tideHeight)
{
  float deltaTime = aTime.deltaTime;
  float absoluteTime = aTime.absoluteTime;
  
  //Move with tide and waves
  irr::core::vector3df pos=getPosition();
  pos.Y = tideHeight + mWater->getWaveHeight(pos.X,pos.Z);

  //Move with tidal stream (if not aground)
  irr::f32 depth = -1*mTerrain->getHeight(pos.X,pos.Z)+pos.Y;
  irr::core::vector2df mobVector = mTide->getTidalStream(mTerrain->xToLong(pos.X),mTerrain->zToLat(pos.Z), absoluteTime);
    
  // Add component from wind
  irr::f32 windSpeed = mWind->getTrueSpeed() * KTS_TO_MPS;
  irr::f32 windDirection = mWind->getTrueDirection();
  // Convert this into wind axial speed and wind lateral speed
  irr::f32 windFlowDirection = windDirection + 180; // Wind direction is where the wind is from. We want where it is flowing towards
  irr::f32 windX = windSpeed * sin(windFlowDirection * irr::core::DEGTORAD);
  irr::f32 windZ = windSpeed * cos(windFlowDirection * irr::core::DEGTORAD);
  // Assume that the MoB moves at 1/10 of the wind speed
  mobVector.X += windX * 0.1;
  mobVector.Y += windZ * 0.1;

  // Apply movement vector
  if (depth > 0) {
    irr::f32 streamScaling = fmin(1,depth); //Reduce effect as water gets shallower
    pos.X += mobVector.X*deltaTime*streamScaling;
    pos.Z += mobVector.Y*deltaTime*streamScaling;
  }

  man->setPosition(pos);
}

void ManOverboard::releaseManOverboard(irr::core::vector3df aOwnShipPos, float aBreadth, float aHeading)
{
  //Only release/update if not already released
  if(!man->isVisible())
    {
      man->setVisible(true);
      irr::core::vector3df relativePosition;
      relativePosition.Y = 0;
      //Put randomly on port or starboard side of the ship
      if (rand() > RAND_MAX/2) {
	relativePosition.X = aBreadth *  0.6 * cos(aHeading*irr::core::DEGTORAD);
	relativePosition.Z = aBreadth * -0.6 * sin(aHeading*irr::core::DEGTORAD);

      } else {
	relativePosition.X = aBreadth * -0.6 * cos(aHeading*irr::core::DEGTORAD);
	relativePosition.Z = aBreadth *  0.6 * sin(aHeading*irr::core::DEGTORAD);

      }
      man->setPosition(aOwnShipPos + relativePosition);
    }
}

void ManOverboard::setVisible(bool aVisible){man->setVisible(aVisible);}
void ManOverboard::setPos(float aPosX, float aPosZ){man->setPosition(irr::core::vector3df(aPosX,0,aPosZ));}
void ManOverboard::retrieveManOverboard(void){man->setVisible(false);}
bool ManOverboard::getVisible(void) const {return man->isVisible();}
float ManOverboard::getPosX(void) const {return man->getPosition().X;}
float ManOverboard::getPosZ(void) const {return man->getPosition().Z;}


