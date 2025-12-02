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

// Extends from the general 'Ship' class
#include <cstdlib> //For rand()
#include <algorithm>

#include "OwnShip.hpp"
#include "Sail.hpp"
#include "Constants.hpp"
#include "ScenarioDataStructure.hpp"
#include "Terrain.hpp"
#include "IniFile.hpp"
#include "Angles.hpp"
#include "Utilities.hpp"
#include "Solver.hpp"
#include "Collision.hpp"
#include "Wind.hpp"
#include "Water.hpp"
#include "Tide.hpp"


OwnShip::OwnShip()
{


}

OwnShip::~OwnShip()
{

}


void OwnShip::load(OwnShipData aOwnShipData, Water *aWater, Tide *aTide, Terrain *aTerrain, irr::IrrlichtDevice *aDev)
{
  mDevice = aDev;
  mTerrain = aTerrain;
  mTide = aTide;
  mWater = aWater;
  
  irr::scene::ISceneManager* smgr = mDevice->getSceneManager();
  
  // Load from ownShip.ini file
  std::string ownShipName = aOwnShipData.name;

  basePath = "models/Ownship/" + ownShipName + "/";
  std::string userFolder = Utilities::getUserDir();
  // Read model from user dir if it exists there.
  if (Utilities::pathExists(userFolder + basePath))
    {
      basePath = userFolder + basePath;
    }

  // Load from boat.ini file if it exists
  std::string shipIniFilename = basePath + "boat.ini";

  // Construct the radar config file name, to be used later by the radar
  radarConfigFile = basePath + "radar.ini";

  // get the model file
  std::string ownShipFileName = IniFile::iniFileToString(shipIniFilename, "FileName");
  std::string ownShipFullPath = basePath + ownShipFileName;

  double xG = 0, iZ = 0, jZ = 0;

  if(0 == setShipParams("kvlcc2"))
    {
      mM = mRho * mGeoParams.volume;
      mMX = 0.5 * mRho * pow(mGeoParams.lPP, 2) * mGeoParams.d * mAddedMassParams.mpX;
      mMY = 0.5 * mRho * pow(mGeoParams.lPP, 2) * mGeoParams.d * mAddedMassParams.mpY;

      xG = mGeoParams.xG;
      iZ = mM * pow((0.25 * mGeoParams.lPP), 2);
      jZ = 0.5 * mRho * pow(mGeoParams.lPP, 4) * mGeoParams.d * mAddedMassParams.jpZ;

      mMatM << mM+mMX, 0, 0,
        0, mM+mMY, xG*mM,
        0, xG*mM, iZ+(mM*pow(xG, 2))+jZ;

      mInvMatM = mMatM.inverse();
      mMu = mMu0;
      mEta << mTerrain->latToZ(aOwnShipData.initialLat), mTerrain->longToX(aOwnShipData.initialLong), aOwnShipData.initialBearing*PI/180;

      //std::cout << "eta : " << mEta << " - mu : " << mMu;
    }
  
  
  rollPeriod = IniFile::iniFileTof32(shipIniFilename, "RollPeriod");
  rollAngle = 2 * IniFile::iniFileTof32(shipIniFilename, "Swell");  
  pitchPeriod = IniFile::iniFileTof32(shipIniFilename, "PitchPeriod");
  pitchAngle = 0.5 * IniFile::iniFileTof32(shipIniFilename, "Swell");  // Max pitch Angle (deg @weather=1)
  buffet = IniFile::iniFileTof32(shipIniFilename, "Buffet");
  depthSounder = (IniFile::iniFileTou32(shipIniFilename, "HasDepthSounder") == 1);
  maxSounderDepth = IniFile::iniFileTof32(shipIniFilename, "MaxDepth");
  gps = (IniFile::iniFileTou32(shipIniFilename, "HasGPS") == 1);

  // Scale
  scaleFactor = IniFile::iniFileTof32(shipIniFilename, "ScaleFactor");
  irr::f32 yCorrection = IniFile::iniFileTof32(shipIniFilename, "YCorrection");
  mAngleCorrection = IniFile::iniFileTof32(shipIniFilename, "AngleCorrection");
  // DEE_DEC22 vvvv
  angleCorrectionRoll = 0;  // default value
  angleCorrectionPitch = 0; // default value
  angleCorrectionRoll = IniFile::iniFileTof32(shipIniFilename, "AngleCorrectionRoll");
  angleCorrectionPitch = IniFile::iniFileTof32(shipIniFilename, "AngleCorrectionPitch");

  irr::u32 numberOfViews = IniFile::iniFileTof32(shipIniFilename, "Views");
  if (numberOfViews == 0)
    {
      std::cerr << "Own ship: View positions can't be loaded. Please check ini file " << shipIniFilename << std::endl;
      exit(EXIT_FAILURE);
    }
  for (irr::u32 i = 1; i <= numberOfViews; i++)
    {
      irr::f32 camOffsetX = IniFile::iniFileTof32(shipIniFilename, IniFile::enumerate1("ViewX", i));
      irr::f32 camOffsetY = IniFile::iniFileTof32(shipIniFilename, IniFile::enumerate1("ViewY", i));
      irr::f32 camOffsetZ = IniFile::iniFileTof32(shipIniFilename, IniFile::enumerate1("ViewZ", i));
      bool highView = IniFile::iniFileTou32(shipIniFilename, IniFile::enumerate1("ViewHigh", i)) == 1;
      views.push_back(irr::core::vector3df(scaleFactor * camOffsetX, scaleFactor * camOffsetY, scaleFactor * camOffsetZ));
      isHighView.push_back(highView);
    }

  // Radar Screen position, if not set in file, set value to -999 as 'no data' marker
  mRadarPos.X = IniFile::iniFileTof32(shipIniFilename, "RadarScreenX", -999);
  mRadarPos.Y = IniFile::iniFileTof32(shipIniFilename, "RadarScreenY", -999);
  mRadarPos.Z = IniFile::iniFileTof32(shipIniFilename, "RadarScreenZ", -999);
  mRadarSize = IniFile::iniFileTof32(shipIniFilename, "RadarScreenSize");
  mRadarTilt = IniFile::iniFileTof32(shipIniFilename, "RadarScreenTilt");
  // Default position out of view if not set
  if (mRadarPos.X == -999.0 && mRadarPos.Y == -999.0 && mRadarPos.Z == -999.0)
    {
      mRadarPos.X = 0;
      mRadarPos.Y = 0;
      mRadarPos.Y = 500;
    }

  if (mRadarSize <= 0)
    {
      mRadarSize = 1;
    }
  mRadarPos = scaleFactor * mRadarPos;
  mRadarSize = scaleFactor * mRadarSize;
  
  // Load the model
  irr::scene::IMesh *shipMesh;

  // Set mesh vertical correction (world units)
  mHeightCorrection = yCorrection * scaleFactor;

  shipMesh = smgr->getMesh(ownShipFullPath.c_str());

  // Make mesh scene node
  if (shipMesh == 0)
    {
      // Failed to load mesh - load with dummy and continue
      mDevice->getLogger()->log("Failed to load own ship model:");
      mDevice->getLogger()->log(ownShipFullPath.c_str());
      shipMesh = smgr->addSphereMesh("Dummy name");
    }

  // If any part is partially transparent, make it fully transparent (for bridge windows etc!)
  if (IniFile::iniFileTou32(shipIniFilename, "MakeTransparent") == 1)
    {
      for (irr::u32 mb = 0; mb < shipMesh->getMeshBufferCount(); mb++)
	{
	  if (shipMesh->getMeshBuffer(mb)->getMaterial().DiffuseColor.getAlpha() < 255)
	    {
	      // Hide this mesh buffer by scaling to zero size
	      smgr->getMeshManipulator()->scale(shipMesh->getMeshBuffer(mb), irr::core::vector3df(0, 0, 0));
	    }
	}
    }

  mShipScene = smgr->addMeshSceneNode(shipMesh, 0, IDFlag_IsPickable, irr::core::vector3df(0, 0, 0));

  /*Load Sails*/
  mSailsCount = IniFile::iniFileTou32(shipIniFilename, "SailsCount");
  irr::scene::IMesh* sailMesh[4] = {NULL}; //4 sails max for now 
        

  if (mSailsCount > 0)
    {
      //Load sail parameters
      mSails.Open(basePath + "/nc/polar.nc", "TotalSails_X", "TotalSails_Y");
      mSails.Init("STW_kt", "TWS_kt", "TWA_deg");

	  
      mSailsType = IniFile::iniFileToString(shipIniFilename, "SailsType");
      std::string sailsSize = IniFile::iniFileToString(shipIniFilename, "SailsSize");
      std::string meshFile = basePath + "../../Sails/" + mSailsType + "/" + sailsSize + "/" + "sail.obj";

      for (int i = 0; i < mSailsCount; i++)
	{

	  sailMesh[i] = smgr->getMesh(meshFile.c_str());
	  mSailsScene[i] = smgr->addMeshSceneNode(sailMesh[i]);

	  irr::f32 sailPosX = IniFile::iniFileTof32(shipIniFilename, IniFile::enumerate1("SailsX", i+1));
	  irr::f32 sailPosY = IniFile::iniFileTof32(shipIniFilename, IniFile::enumerate1("SailsY", i+1));
	  irr::f32 sailPosZ = IniFile::iniFileTof32(shipIniFilename, IniFile::enumerate1("SailsZ", i+1));

	  mSailsScene[i]->setParent(mShipScene);
	  mSailsScene[i]->setPosition(irr::core::vector3df(sailPosX, sailPosY, sailPosZ)); 
	  mSailsScene[i]->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);

	  if (mSailsScene[i]->getMaterialCount() > 0)
	    {
	      for (irr::u32 mat = 0; mat < mShipScene->getMaterialCount(); mat++)
		{
		  mSailsScene[i]->getMaterial(mat).MaterialType = irr::video::EMT_LIGHTMAP;
		  mSailsScene[i]->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
		}
	    }

	}

    }   


  mShipScene->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
  mShipScene->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); // Normalise normals on scaled meshes, for correct lighting
  // Set lighting to use diffuse and ambient, so lighting of untextured models works
  if (mShipScene->getMaterialCount() > 0)
    {
      for (irr::u32 mat = 0; mat < mShipScene->getMaterialCount(); mat++)
	{
	  mShipScene->getMaterial(mat).MaterialType = irr::video::EMT_LIGHTMAP;
	  mShipScene->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
	}
    }
    

  mShipScene->setName("OwnShip");

  mShipScene->setScale(irr::core::vector3df(scaleFactor, scaleFactor, scaleFactor));
  mShipScene->setPosition(irr::core::vector3df(0, mHeightCorrection, 0));
  mShipScene->updateAbsolutePosition();

  //length = mShipScene->getTransformedBoundingBox().getExtent().Z; // Store length for basic collision calculation
  //breadth = mShipScene->getTransformedBoundingBox().getExtent().X;  // Store length for basic collision calculation

  //draught = -1 * mShipScene->getTransformedBoundingBox().MinEdge.Y;
  mAirDraught = mShipScene->getTransformedBoundingBox().MaxEdge.Y;


  if (rollPeriod == 0)
    {
      rollPeriod = 8; // default to a roll period of 8 seconds if unspecified
    }

  if (pitchPeriod == 0)
    {
      pitchPeriod = 12; // default to a roll periof of 12 seconds if unspecified DEE_DEC22 make a function of
      // weather strength direction and Ixx
    }

  // Default buffet Period DEE_DEC22 to do make this a function of Izz and weather strength perhaps direction too
  buffetPeriod = 8; // Yaw period (s)

  // Default for maxSounderDepth
  if (maxSounderDepth < 1)
    {
      maxSounderDepth = 100;
    } // Default


  mDevice->getLogger()->log("Length, breadth, draught are calculated from bounding box as (m):");
  mDevice->getLogger()->log(irr::core::stringw(mGeoParams.lPP).c_str());
  mDevice->getLogger()->log(irr::core::stringw(mGeoParams.b).c_str());
  mDevice->getLogger()->log(irr::core::stringw(mGeoParams.d).c_str());

  controlMode = MODE_ENGINE;

  // set initial pitch and roll
  pitch = 0;
  roll = 0;
  waveHeightFiltered = 0;

  mWheel = 0;

}



void OwnShip::setRateOfTurn(irr::f32 rateOfTurn) // Sets the rate of turn (used when controlled as secondary)
{
  controlMode = MODE_AUTO; // Switch to controlled mode
  this->mMu[1] = rateOfTurn;
}


void OwnShip::setWheel(irr::f32 aWheel)
{
  controlMode = MODE_ENGINE; // Switch to engine and rudder mode
  // Set the wheel (-ve is port, +ve is stbd), unless follow up rudder isn't working (overrideable with 'force')
  mWheel = aWheel;
  if (mWheel < -(mRudder.getDeltaMax())*180/PI)
    {
      mWheel = -(mRudder.getDeltaMax()*180/PI);
    }
  if (mWheel > mRudder.getDeltaMax()*180/PI)
    {
      mWheel = mRudder.getDeltaMax()*180/PI;
    }
}


void OwnShip::setPortEngine(irr::f32 port)
{
  controlMode = MODE_ENGINE; // Switch to engine and rudder mode


  portEngine = port; //+-1
  if (portEngine > 1)
    {
      portEngine = 1;
    }
  if (portEngine < -1)
    {
      portEngine = -1;
    }

} // end setPortEngine

void OwnShip::setStbdEngine(irr::f32 stbd)
{
  controlMode = MODE_ENGINE; // Switch to engine and rudder mode

  stbdEngine = stbd; //+-1
  if (stbdEngine > 1)
    {
      stbdEngine = 1;
    }
  if (stbdEngine < -1)
    {
      stbdEngine = -1;
    }

} // end setStbdEngine


irr::f32 OwnShip::getPortEngine() const
{
  return portEngine;
}

irr::f32 OwnShip::getStbdEngine() const
{
  return stbdEngine;
}


irr::f32 OwnShip::getWheel() const
{
  return mWheel;
}

irr::f32 OwnShip::getPitch() const
{
  return pitch;
}

irr::f32 OwnShip::getRoll() const
{
  return roll;
}

irr::f32 OwnShip::getShipMass() const
{
  return mM;
}

std::string OwnShip::getBasePath() const
{
  return basePath;

}

irr::f32 OwnShip::getScaleFactor() const
{
  return scaleFactor;
}

irr::core::vector3df OwnShip::getRadarPosition() const
{
  return mRadarPos;
}

irr::f32 OwnShip::getRadarSize() const
{
  return mRadarSize;
}

irr::f32 OwnShip::getRadarTilt() const
{
  return mRadarTilt;
}

void OwnShip::update(sTime& aTime, irr::f32 tideHeight, irr::f32 weather, Wind *aWind, Solver *aSolver)
{
  float posZ = getPosition().Z;
  float posX = getPosition().X;
  float deltaTime = aTime.deltaTime;
  
  setEta(aSolver->getEta());
  setMu(aSolver->getMu());
  
  if (controlMode == MODE_ENGINE)
    {
       
      // Find tidal stream, based on our current absolute position
      irr::core::vector2df stream = mTide->getTidalStream(mTerrain->xToLong(posX), mTerrain->zToLat(posZ), aTime.absoluteTime);
      //std::cout << "Tidal stream x:" << stream.X << ", z:" << stream.Y << std::endl;
      irr::f32 streamScaling = fmax(0, fmin(1, getDepth(mTerrain))); // Reduce effect as water gets shallower
      stream *= streamScaling;
      // Convert this into stream axial and lateral speed
      irr::f32 axialStream = stream.X * sin(mEta[2] * irr::core::DEGTORAD) + stream.Y * cos(mEta[2] * irr::core::DEGTORAD); // Stream in ahead direction
      irr::f32 lateralStream = stream.X * cos(mEta[2] * irr::core::DEGTORAD) - stream.Y * sin(mEta[2] * irr::core::DEGTORAD);// Stream in stbd direction

      mSpeedThroughWater = mMu[0] - axialStream;

      //Update sails
      if (mSailsCount > 0)
	{
	  mSails.SetSTW(mSpeedThroughWater);
	  mSails.SetWind(aWind->getTrueSpeed(), aWind->getApparentDir());
	}     

      //Apply engine power 
      if(mNumberProp > 1)
	{
	  irr::f32 portThrust = 0; 
	  irr::f32 stbdThrust = 0;
	  
	  portThrust = portEngine * 20;
	  stbdThrust = stbdEngine * 20;

	  mProp[0].SetRevs(portThrust);
	  mProp[1].SetRevs(stbdThrust);
	}
      else
	{
	  irr::f32 monoThrust = 0;
	  
	  monoThrust = portEngine * 20;	 

	  mProp[0].SetRevs(monoThrust);
	}

      //Apply rudder angle
      mRudder.SetDelta((mWheel*PI)/180, deltaTime);
      
    }
  else // End of engine mode
    {
      // MODE_AUTO
      if (!positionManuallyUpdated)
        {
	  // Apply rate of turn
	  mEta[2] += mMu[1] * deltaTime * irr::core::RADTODEG; // Deg
        }
    }

  irr::f32 timeConstant = 0.5; // Time constant in s; TODO: Make dependent on vessel size
  irr::f32 factor = deltaTime / (timeConstant + deltaTime);
  waveHeightFiltered = (1 - factor) * waveHeightFiltered + factor * mWater->getWaveHeight(mEta[1], mEta[0]); // TODO: Check implementation of simple filter!
  double yPos = tideHeight + mHeightCorrection + waveHeightFiltered;

  // calculate pitch and roll - not linked to water/wave motion
  if (pitchPeriod > 0)
    {
      pitch = weather * pitchAngle * sin(aTime.scenarioTime * 2 * PI / pitchPeriod);
    }
  if (rollPeriod > 0)
    {
      roll = weather * rollAngle * sin(aTime.scenarioTime * 2 * PI / rollPeriod);
    }


  /*Sails dyn*/
  if (mSailsType == "Rotor")
    {
      static float angle = 0.0;

      angle += 20;
      irr::core::vector3df rotation(0, angle, 0);

      for (int i = 0; i < mSailsCount; i++)
        {
	  mSailsScene[i]->setRotation(rotation);

        }
    }
  /******************/

  // Set position & angles
  /*  std::cout << "--> Xeta : " << mEta[0] << std::endl;
  std::cout << "--> Yeta : " << mEta[1] << std::endl;
  std::cout << "--> Hdg : " << mEta[2]*180/PI << std::endl;
  std::cout << "--> Speed X : " << mMu[0] << std::endl;
  std::cout << "--> Speed Y : " << mMu[1] << std::endl;
  std::cout << "--> Speed Z : " << mMu[2] << std::endl;
  std::cout << "--> Revs : " << mProp[0].getRevs() << std::endl;
  std::cout << "--> Rudder : " << mRudder.getDelta() << std::endl;
  std::cout << "--> Wheel : " << mWheel << std::endl;
  std::cout << "--> Forward Rotation : " << mProp[0].getForwardRotDir() << std::endl;
  std::cout << "--> Current Rotation : " << mProp[0].getCurrentRotDir() << std::endl;
  std::cout << "**************" << std::endl;*/
  mShipScene->setPosition(irr::core::vector3df(mEta[1], yPos, mEta[0]));
  mShipScene->setRotation(Angles::irrAnglesFromYawPitchRoll(mEta[2]*180/PI, pitch, roll));
  
}

irr::f32 OwnShip::getAngleCorrection() const
{
  return mAngleCorrection;
}

bool OwnShip::hasGPS() const
{
  return gps;
}

bool OwnShip::hasDepthSounder() const
{
  return depthSounder;
}

bool OwnShip::hasTurnIndicator() const
{
  return turnIndicatorPresent;
}

irr::f32 OwnShip::getMaxSounderDepth() const
{
  return maxSounderDepth;
}

std::vector<irr::core::vector3df> OwnShip::getCameraViews() const
{
  return views;
}

std::vector<bool> OwnShip::getCameraIsHighView() const
{
  return isHighView;
}

std::string OwnShip::getRadarConfigFile() const
{
  return radarConfigFile;
}

irr::f32 OwnShip::sign(irr::f32 inValue) const
{
  if (inValue > 0)
    {
      return 1.0;
    }
  if (inValue < 0)
    {
      return -1.0;
    }
  return 0.0;
}

irr::f32 OwnShip::sign(irr::f32 inValue, irr::f32 threshold) const
{
  if (threshold <= 0)
    {
      return sign(inValue);
    }

  if (inValue > threshold)
    {
      return 1.0;
    }
  if (inValue < -1 * threshold)
    {
      return -1.0;
    }
  return inValue / threshold;
}
