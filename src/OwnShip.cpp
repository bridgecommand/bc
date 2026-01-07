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
#include <iostream>
#include <fstream>
#include <filesystem>

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
  mViews.resize(MESH_VIEWS_MAX);
  mIsHighView.resize(MESH_VIEWS_MAX);
  
  mHasGps = false;
  mHasDepthSounder = false;
  mMaxSounderDepth = 0;
  mHasRoTIndicator = false;

  mHasDepthSounder = false;
  mMaxSounderDepth = 0;
  mHasGps = false;
  mHasRoTIndicator = false;

  mRadarConfigFile = "";
  mRadarPos.X = 0;
  mRadarPos.Y = 0;
  mRadarPos.Z = 0;
  mRadarSize = 0;
  mRadarTilt = 0;

  mWheel=0;
  mIsTransparent = false;
    
  mRollPeriod = 0;       
  mRollAngle = 0;        
  mPitchPeriod = 0;      
  mPitchAngle = 0;       
  mBuffetPeriod = 0;     
  mBuffet = 0;           
  mPitch = 0;            
  mRoll = 0;             
  mPortEngine = 0;       
  mStbdEngine = 0;       

  
}

OwnShip::~OwnShip()
{

}

void OwnShip::PrintDevices(void)
{
  std::cout << "::::::Devices Parameters::::::" << std::endl;
  std::cout << "Depth Sounder : " << mHasDepthSounder << " - Max Depth : " << mMaxSounderDepth << std::endl;
  std::cout << "Gps : " << mHasGps << std::endl;
  std::cout << "Turn Indicator : " << mHasRoTIndicator << std::endl;
  std::cout << "Radar : Pos X: " << mRadarPos.X << " Y: " << mRadarPos.Y << " Z: " << mRadarPos.Z << " | Size: " << mRadarSize << " | Tilt: " << mRadarTilt << std::endl;
  std::cout << "::::::::::::" << std::endl;
}

void OwnShip::PrintMeshInfos(void)
{
  std::cout << "::::::Mesh File Infos::::::" << std::endl;
  std::cout << "Name : " << mMeshFileName << std::endl;
  std::cout << "Full path mesh : " << mMeshFullPath << std::endl;
  std::cout << "Scale factor : " << mScaleFactor << std::endl;
  std::cout << "Angle correction : " << mAngleCorrection << std::endl;
  std::cout << "Height correction : " << mHeightCorrection << std::endl; 

  for (unsigned char i=0; i<MESH_VIEWS_MAX; i++)
    {
      std::cout << "View " << i << " :" << std::endl;
      std::cout << "  -> X : " << mViews[i][0] << std::endl;
      std::cout << "  -> Y : " << mViews[i][1] << std::endl;
      std::cout << "  -> Z : " << mViews[i][2] << std::endl;
      std::cout << "  -> isHighView : " << mIsHighView[i]  << std::endl;
      std::cout << "--------" << std::endl;
    }

  std::cout << "::::::::::::" << std::endl;
}

void OwnShip::InitOwnShipParams(OwnShipData aOwnShipData, Json::Value aJsonRoot)
{
  double xG = 0, iZ = 0, jZ = 0;

  //Init Speed
  mMu0 << aJsonRoot["initialSpeed"][0].asFloat(), aJsonRoot["initialSpeed"][1].asFloat(), aJsonRoot["initialSpeed"][2].asFloat();
  
  mM = RHO_SW * mGeoParams.volume;
  mMX = 0.5 * RHO_SW * pow(mGeoParams.lPP, 2) * mGeoParams.d * mAddedMassParams.mpX;
  mMY = 0.5 * RHO_SW * pow(mGeoParams.lPP, 2) * mGeoParams.d * mAddedMassParams.mpY;

  xG = mGeoParams.xG;
  iZ = mM * pow((0.25 * mGeoParams.lPP), 2);
  jZ = 0.5 * RHO_SW * pow(mGeoParams.lPP, 4) * mGeoParams.d * mAddedMassParams.jpZ;

  mMatM << mM+mMX, 0, 0,
    0, mM+mMY, xG*mM,
    0, xG*mM, iZ+(mM*pow(xG, 2))+jZ;

  mInvMatM = mMatM.inverse();
  mMu = mMu0;

  mEta << mTerrain->latToZ(aOwnShipData.initialLat), mTerrain->longToX(aOwnShipData.initialLong), aOwnShipData.initialBearing*PI/180;

  //Mesh
  if(1 == aJsonRoot["mesh"]["makeTransparent"].asInt())
    mIsTransparent = true;
  else
    mIsTransparent = false;
  
  mScaleFactor = aJsonRoot["mesh"]["scaleFactor"].asFloat();
  float yCorrection = aJsonRoot["mesh"]["yCorrection"].asFloat();
  mAngleCorrection = aJsonRoot["mesh"]["angleCorrection"].asFloat();
  mHeightCorrection = yCorrection * mScaleFactor;

  //Views
  for (unsigned char i=0; i<MESH_VIEWS_MAX; i++)
    {
      mViews[i][0] = mScaleFactor * aJsonRoot["mesh"]["views"][i][0].asFloat();
      mViews[i][1] = mScaleFactor * aJsonRoot["mesh"]["views"][i][1].asFloat();
      mViews[i][2] = mScaleFactor * aJsonRoot["mesh"]["views"][i][2].asFloat();
      mIsHighView[i] = mScaleFactor * aJsonRoot["mesh"]["views"][i][3].asFloat();
    }
  
  //Devices
  //DepthSounder
  if(1 == aJsonRoot["depthSounder"]["number"].asInt())
    mHasDepthSounder = true;
  else
    mHasDepthSounder = false;

  if(mHasDepthSounder)
    mMaxSounderDepth = aJsonRoot["depthSounder"]["maxDepth"].asFloat();
  //GPS
  if(1 == aJsonRoot["gps"]["number"].asInt())
    mHasGps = true;
  else
    mHasGps = false;
  if(1 == aJsonRoot["rotIndicator"]["number"].asInt())
    mHasRoTIndicator = true;
  else
    mHasRoTIndicator = false;
  // Radar 
  mRadarPos.X = aJsonRoot["radar"]["pos"][0].asFloat();
  mRadarPos.Y = aJsonRoot["radar"]["pos"][1].asFloat();
  mRadarPos.Z = aJsonRoot["radar"]["pos"][2].asFloat();
  mRadarSize = aJsonRoot["radar"]["size"].asFloat();
  mRadarTilt = aJsonRoot["radar"]["tilt"].asFloat();
  mRadarPos = mScaleFactor * mRadarPos;
  mRadarSize = mScaleFactor * mRadarSize;

  if(0 == mRadarSize)
    mRadarSize = 1;

  PrintDevices();
  PrintMeshInfos();
}

void OwnShip::Load(OwnShipData aOwnShipData, Water *aWater, Tide *aTide, Terrain *aTerrain, irr::IrrlichtDevice *aDev)
{
  int retShipPrms = -1;
  Json::Value rootJson;
  irr::scene::IMesh *shipMesh;
  
  mDevice = aDev;
  mTerrain = aTerrain;
  mTide = aTide;
  mWater = aWater;
  
  irr::scene::ISceneManager* smgr = mDevice->getSceneManager();
  std::string ownShipName = aOwnShipData.name;

  basePath = "models/Ownship/" + ownShipName + "/";
  std::string userFolder = Utilities::getUserDir();

  if(std::filesystem::exists(userFolder + basePath))
      basePath = userFolder + basePath;

  // Load from boat.ini file if it exists
  std::string shipJsonFilename = basePath + "boat.json";
   
  angleCorrectionRoll = 0;  // default value
  angleCorrectionPitch = 0; // default value

  std::filesystem::path boatJson = shipJsonFilename;  
  if(std::filesystem::exists(boatJson))
    {
      std::ifstream streamJson(boatJson);                
      streamJson >> rootJson;
      retShipPrms = InitShipParams(rootJson);
      
      // get the model file
      mMeshFileName = rootJson["mesh"]["name"].asString();
      mMeshFullPath = basePath + mMeshFileName;
      streamJson.close();
      // Load the model
      shipMesh = smgr->getMesh(mMeshFullPath.c_str());
      mShipScene = smgr->addMeshSceneNode(shipMesh, 0, IDFlag_IsPickable, irr::core::vector3df(0, 0, 0));
    }
  else
    return;
  
  if(0 == retShipPrms)
      InitOwnShipParams(aOwnShipData, rootJson);

  
  /*Load Sails*/
  if(mSails.GetCount() > 0)
    { 
      irr::scene::IMesh* sailMesh[SAILS_MAX] = {NULL}; //4 sails max for now 
      //Load sail parameters
      mSails.OpenPolar(basePath + "/nc/polar.nc", "TotalSails_X", "TotalSails_Y");
      mSails.InitPolar("STW_kt", "TWS_kt", "TWA_deg");
      std::string meshFile = basePath + "../../Sails/" + mSails.GetType() + "/" + mSails.GetSize() + "/" + "sail.obj";

      for (int i = 0; i < mSails.GetCount(); i++)
	{
	  sailMesh[i] = smgr->getMesh(meshFile.c_str());
	  mSails.SetMeshScene(smgr->addMeshSceneNode(sailMesh[i]));
	          
	  mSails.GetMeshScene(i)->setParent(mShipScene);
	  mSails.GetMeshScene(i)->setPosition(irr::core::vector3df(mSails.GetPos()[i][0], mSails.GetPos()[i][1], mSails.GetPos()[i][2])); 
	  mSails.GetMeshScene(i)->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);

	  if(mSails.GetMeshScene(i)->getMaterialCount() > 0)
	    {
	      for (irr::u32 mat = 0; mat < mShipScene->getMaterialCount(); mat++)
		{
		  mSails.GetMeshScene(i)->getMaterial(mat).MaterialType = irr::video::EMT_LIGHTMAP;
		  mSails.GetMeshScene(i)->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
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

  if(mIsTransparent)
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
  
  mShipScene->setName("OwnShip");

  mShipScene->setScale(irr::core::vector3df(mScaleFactor, mScaleFactor, mScaleFactor));
  mShipScene->setPosition(irr::core::vector3df(0, mHeightCorrection, 0));
  mShipScene->updateAbsolutePosition();
  mRollAngle = 0.1;
  mBuffet = 0.3;

  if(mRollPeriod == 0)
    {
      mRollPeriod = 8; // default to a roll period of 8 seconds if unspecified
    }

  if(mPitchPeriod == 0)
    {
      mPitchPeriod = 12; // default to a roll periof of 12 seconds if unspecified DEE_DEC22 make a function of
      // weather strength direction and Ixx
    }

  // Default buffet Period DEE_DEC22 to do make this a function of Izz and weather strength perhaps direction too
  mBuffetPeriod = 8; // Yaw period (s)

  mControlMode = MODE_ENGINE;

}

void OwnShip::Update(sTime& aTime, irr::f32 aTideHeight, irr::f32 aWeather, Wind *aWind, Solver *aSolver)
{
  float posZ = getPosition().Z;
  float posX = getPosition().X;
  float deltaTime = aTime.deltaTime;
  
  setEta(aSolver->getEta());
  setMu(aSolver->getMu());
  
  if (mControlMode == MODE_ENGINE)
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
      if (mSails.GetCount() > 0)
	{
	  mSails.SetSTW(mSpeedThroughWater);
	  mSails.SetWind(aWind->getTrueSpeed(), aWind->getApparentDir());
	}     

      //Apply engine power 
      if(mNumberProp > 1)
	{
	  irr::f32 portThrust = 0; 
	  irr::f32 stbdThrust = 0;
	  
	  portThrust = mPortEngine*mEngine[0].getRpmMax()/60;
	  stbdThrust = mStbdEngine*mEngine[1].getRpmMax()/60;

	  mProp[0].SetRevs(portThrust);
	  mProp[1].SetRevs(stbdThrust);
	}
      else
	{
	  irr::f32 monoThrust = 0;
	  
	  monoThrust = mPortEngine*mEngine[0].getRpmMax()/60;	 
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
  mWaveHeightFiltered = (1 - factor) * mWaveHeightFiltered + factor * mWater->getWaveHeight(mEta[1], mEta[0]); // TODO: Check implementation of simple filter!
  double yPos = aTideHeight + mHeightCorrection + mWaveHeightFiltered;

  // calculate pitch and roll - not linked to water/wave motion
  if(mPitchPeriod > 0)
    {
      mPitch = aWeather * mPitchAngle * sin(aTime.scenarioTime * 2 * PI / mPitchPeriod);
    }
  if(mRollPeriod > 0)
    {
      mRoll = aWeather * mRollAngle * sin(aTime.scenarioTime * 2 * PI / mRollPeriod);
    }


  /*Sails dyn*/
  mSails.UpdateMesh();

  mShipScene->setPosition(irr::core::vector3df(mEta[1], yPos, mEta[0]));
  mShipScene->setRotation(Angles::irrAnglesFromYawPitchRoll(mEta[2]*180/PI, mPitch, mRoll));
  
}


void OwnShip::setRateOfTurn(irr::f32 rateOfTurn) // Sets the rate of turn (used when controlled as secondary)
{
  mControlMode = MODE_AUTO; // Switch to controlled mode
  this->mMu[1] = rateOfTurn;
}


void OwnShip::setWheel(irr::f32 aWheel)
{
  mControlMode = MODE_ENGINE; // Switch to engine and rudder mode
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

/*void OwnShip::setRudder(irr::f32 aDelta)
  {
  controlMode = MODE_ENGINE;
  mRudder.SetDelta(aDelta);
  }*/


void OwnShip::setPortEngine(irr::f32 aPort)
{
  mControlMode = MODE_ENGINE; // Switch to engine and rudder mode


  mPortEngine = aPort; //+-1
  if (mPortEngine > 1)
    {
      mPortEngine = 1;
    }
  if (mPortEngine < -1)
    {
      mPortEngine = -1;
    }

} // end setPortEngine

void OwnShip::setStbdEngine(irr::f32 aStbd)
{
  mControlMode = MODE_ENGINE; // Switch to engine and rudder mode

  mStbdEngine = aStbd; //+-1
  if (mStbdEngine > 1)
    {
      mStbdEngine = 1;
    }
  if (mStbdEngine < -1)
    {
      mStbdEngine = -1;
    }

} // end setStbdEngine


irr::f32 OwnShip::getPortEngine() const
{
  return mPortEngine;
}

irr::f32 OwnShip::getStbdEngine() const
{
  return mStbdEngine;
}


irr::f32 OwnShip::getWheel() const
{
  return mWheel;
}

irr::f32 OwnShip::getPitch() const
{
  return mPitch;
}

irr::f32 OwnShip::getRoll() const
{
  return mRoll;
}

irr::f32 OwnShip::getShipMass() const
{
  return mM;
}

std::string OwnShip::getBasePath() const
{
  return basePath;

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


irr::f32 OwnShip::getAngleCorrection() const
{
  return mAngleCorrection;
}

std::vector<irr::core::vector3df> OwnShip::getCameraViews() const
{
  return mViews;
}

std::vector<bool> OwnShip::getCameraIsHighView() const
{
  return mIsHighView;
}

std::string OwnShip::getRadarConfigFile() const
{
  return mRadarConfigFile;
}

bool OwnShip::HasGPS() const {return mHasGps;}
bool OwnShip::HasDepthSounder() const {return mHasDepthSounder;}
float OwnShip::GetMaxSounderDepth() const {return mMaxSounderDepth;}
bool OwnShip::HasRoTIndicator() const {return mHasRoTIndicator;}
