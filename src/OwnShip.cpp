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
#include "OwnShip.hpp"
#include "Sail.hpp"
#include "Constants.hpp"
#include "SimulationModel.hpp"
#include "ScenarioDataStructure.hpp"
#include "Terrain.hpp"
#include "IniFile.hpp"
#include "Angles.hpp"
#include "Utilities.hpp"
#include <cstdlib> //For rand()
#include <algorithm>
#include "Solver.hpp"

#ifdef WITH_PROFILING
#include "iprof.hpp"
#else
#define IPROF(a) //intentionally empty placeholder
#endif


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif // M_PI



void OwnShip::load(OwnShipData aOwnShipData, ModelParameters aModelParams, irr::scene::ISceneManager *aSmgr, SimulationModel *aModel, Terrain *aTerrain, irr::IrrlichtDevice *aDev)
{
  
  mTerrain = aTerrain;
  mModel = aModel;
  mDevice = aDev;
  mModelParams = aModelParams;
  mShowDebugData = aModel->debugModeOn();

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
      mEta << mModel->latToZ(aOwnShipData.initialLat), mModel->longToX(aOwnShipData.initialLong), aOwnShipData.initialBearing*M_PI/180;

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
  angleCorrection = IniFile::iniFileTof32(shipIniFilename, "AngleCorrection");
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
  heightCorrection = yCorrection * scaleFactor;

  shipMesh = aSmgr->getMesh(ownShipFullPath.c_str());

  // Make mesh scene node
  if (shipMesh == 0)
    {
      // Failed to load mesh - load with dummy and continue
      mDevice->getLogger()->log("Failed to load own ship model:");
      mDevice->getLogger()->log(ownShipFullPath.c_str());
      shipMesh = aSmgr->addSphereMesh("Dummy name");
    }

  // If any part is partially transparent, make it fully transparent (for bridge windows etc!)
  if (IniFile::iniFileTou32(shipIniFilename, "MakeTransparent") == 1)
    {
      for (irr::u32 mb = 0; mb < shipMesh->getMeshBufferCount(); mb++)
	{
	  if (shipMesh->getMeshBuffer(mb)->getMaterial().DiffuseColor.getAlpha() < 255)
	    {
	      // Hide this mesh buffer by scaling to zero size
	      aSmgr->getMeshManipulator()->scale(shipMesh->getMeshBuffer(mb), irr::core::vector3df(0, 0, 0));
	    }
	}
    }

  ship = aSmgr->addMeshSceneNode(shipMesh, 0, IDFlag_IsPickable, irr::core::vector3df(0, 0, 0));

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

	  sailMesh[i] = aSmgr->getMesh(meshFile.c_str());
	  mSailsScene[i] = aSmgr->addMeshSceneNode(sailMesh[i]);

	  irr::f32 sailPosX = IniFile::iniFileTof32(shipIniFilename, IniFile::enumerate1("SailsX", i+1));
	  irr::f32 sailPosY = IniFile::iniFileTof32(shipIniFilename, IniFile::enumerate1("SailsY", i+1));
	  irr::f32 sailPosZ = IniFile::iniFileTof32(shipIniFilename, IniFile::enumerate1("SailsZ", i+1));

	  mSailsScene[i]->setParent(ship);
	  mSailsScene[i]->setPosition(irr::core::vector3df(sailPosX, sailPosY, sailPosZ)); 
	  mSailsScene[i]->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true);

	  if (mSailsScene[i]->getMaterialCount() > 0)
	    {
	      for (irr::u32 mat = 0; mat < ship->getMaterialCount(); mat++)
		{
		  mSailsScene[i]->getMaterial(mat).MaterialType = irr::video::EMT_LIGHTMAP;
		  mSailsScene[i]->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
		}
	    }

	}

    }   

  // For debugging:
  if (mShowDebugData)
    {
      // ship->setDebugDataVisible(irr::scene::EDS_NORMALS|irr::scene::EDS_BBOX_ALL);
      ship->setDebugDataVisible(irr::scene::EDS_BBOX_ALL);
    }

  ship->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
  ship->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); // Normalise normals on scaled meshes, for correct lighting
  // Set lighting to use diffuse and ambient, so lighting of untextured models works
  if (ship->getMaterialCount() > 0)
    {
      for (irr::u32 mat = 0; mat < ship->getMaterialCount(); mat++)
	{
	  ship->getMaterial(mat).MaterialType = irr::video::EMT_LIGHTMAP;
	  ship->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
	}
    }
    

  ship->setName("OwnShip");

  ship->setScale(irr::core::vector3df(scaleFactor, scaleFactor, scaleFactor));
  ship->setPosition(irr::core::vector3df(0, heightCorrection, 0));
  ship->updateAbsolutePosition();

  //length = ship->getTransformedBoundingBox().getExtent().Z; // Store length for basic collision calculation
  //breadth = ship->getTransformedBoundingBox().getExtent().X;  // Store length for basic collision calculation

  //draught = -1 * ship->getTransformedBoundingBox().MinEdge.Y;
  airDraught = ship->getTransformedBoundingBox().MaxEdge.Y;


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

  loadCollision(aSmgr);

}


void OwnShip::loadCollision(irr::scene::ISceneManager *aSmgr)
{
  buoyCollision = false;
  otherShipCollision = false;

  // Detect sample points for terrain interaction here (think separately about how to do this for 360 models, probably with a separate collision model)
  // Add a triangle selector

  selector = aSmgr->createTriangleSelector(ship->getMesh(), getSceneNode());
  if (selector)
    {
      mDevice->getLogger()->log("Created triangle selector");
      ship->setTriangleSelector(selector);
    }
  triangleSelectorEnabled = true;

  ship->updateAbsolutePosition();

  irr::core::aabbox3df boundingBox = ship->getTransformedBoundingBox();
  irr::f32 minX = boundingBox.MinEdge.X;
  irr::f32 maxX = boundingBox.MaxEdge.X;
  irr::f32 minY = boundingBox.MinEdge.Y;
  irr::f32 maxY = boundingBox.MaxEdge.Y;
  irr::f32 minZ = boundingBox.MinEdge.Z;
  irr::f32 maxZ = boundingBox.MaxEdge.Z;

  mDevice->getLogger()->log("Own bounding box (scaled): ");
  irr::core::stringw boundingBoxInfo;
  boundingBoxInfo.append("X (beam): ");
  boundingBoxInfo.append(irr::core::stringw(minX));
  boundingBoxInfo.append(" to ");
  boundingBoxInfo.append(irr::core::stringw(maxX));

  boundingBoxInfo.append(" Y (height): ");
  boundingBoxInfo.append(irr::core::stringw(minY));
  boundingBoxInfo.append(" to ");
  boundingBoxInfo.append(irr::core::stringw(maxY));

  boundingBoxInfo.append(" Z (length): ");
  boundingBoxInfo.append(irr::core::stringw(minZ));
  boundingBoxInfo.append(" to ");
  boundingBoxInfo.append(irr::core::stringw(maxZ));

  mDevice->getLogger()->log(boundingBoxInfo.c_str());

  boundingBoxInfo = " Draught: ";
  boundingBoxInfo.append(irr::core::stringw(mGeoParams.d));
  boundingBoxInfo.append(" length: ");
  boundingBoxInfo.append(irr::core::stringw(mGeoParams.lPP));
  boundingBoxInfo.append(" breadth: ");
  boundingBoxInfo.append(irr::core::stringw(mGeoParams.b));

  mDevice->getLogger()->log(boundingBoxInfo.c_str());

  // Find if we need more contact points to maintain minContactPointSpacing
  if (mModelParams.minContactPointSpacing > 0)
    {
      mModelParams.numberOfContactPoints.X = std::max(mModelParams.numberOfContactPoints.X, (int)ceil((maxX - minX) / mModelParams.minContactPointSpacing));
      mModelParams.numberOfContactPoints.Y = std::max(mModelParams.numberOfContactPoints.Y, (int)ceil((maxY - minY) / mModelParams.minContactPointSpacing));
      mModelParams.numberOfContactPoints.Z = std::max(mModelParams.numberOfContactPoints.Z, (int)ceil((maxZ - minZ) / mModelParams.minContactPointSpacing));
    }

  // Grid from below looking up
  for (int i = 0; i < mModelParams.numberOfContactPoints.X; i++)
    {
      for (int j = 0; j < mModelParams.numberOfContactPoints.Z; j++)
        {

	  irr::f32 xSpacing = (maxX - minX) / (irr::f32)(mModelParams.numberOfContactPoints.X - 1);
	  irr::f32 zSpacing = (maxZ - minZ) / (irr::f32)(mModelParams.numberOfContactPoints.Z - 1);

	  irr::f32 xTestPos = minX + (irr::f32)i * xSpacing;
	  irr::f32 zTestPos = minZ + (irr::f32)j * zSpacing;

	  irr::core::line3df ray; // Make a ray. This will start outside the mesh, looking in
	  ray.start.X = xTestPos;
	  ray.start.Y = minY - 0.1;
	  ray.start.Z = zTestPos;
	  ray.end = ray.start;
	  ray.end.Y = maxY + 0.1;

	  // Check the ray and add the contact point if it exists
	  addContactPointFromRay(ray, xSpacing * zSpacing);
        }
    }

  // Grid from ahead/astern
  for (int i = 0; i < mModelParams.numberOfContactPoints.X; i++)
    {
      for (int j = 0; j < mModelParams.numberOfContactPoints.Y; j++)
        {

	  irr::f32 xSpacing = (maxX - minX) / (irr::f32)(mModelParams.numberOfContactPoints.X - 1);
	  irr::f32 ySpacing = (maxY - minY) / (irr::f32)(mModelParams.numberOfContactPoints.Y - 1);

	  irr::f32 xTestPos = minX + (irr::f32)i * xSpacing;
	  irr::f32 yTestPos = minY + (irr::f32)j * ySpacing;

	  irr::core::line3df ray; // Make a ray. This will start outside the mesh, looking in
	  ray.start.X = xTestPos;
	  ray.start.Y = yTestPos;
	  ray.start.Z = maxZ + 0.1;
	  ray.end = ray.start;
	  ray.end.Z = minZ - 0.1;

	  // Check the ray and add the contact point if it exists
	  addContactPointFromRay(ray, xSpacing * ySpacing);
	  // swap ray direction and check again
	  ray.start.Z = minZ - 0.1;
	  ray.end.Z = maxZ + 0.1;
	  addContactPointFromRay(ray, xSpacing * ySpacing);
        }
    }

  // Grid from side to side
  for (int i = 0; i < mModelParams.numberOfContactPoints.Z; i++)
    {
      for (int j = 0; j < mModelParams.numberOfContactPoints.Y; j++)
        {

	  irr::f32 zSpacing = (maxZ - minZ) / (irr::f32)(mModelParams.numberOfContactPoints.Z - 1);
	  irr::f32 ySpacing = (maxY - minY) / (irr::f32)(mModelParams.numberOfContactPoints.Y - 1);

	  irr::f32 zTestPos = minZ + (irr::f32)i * zSpacing;
	  irr::f32 yTestPos = minY + (irr::f32)j * ySpacing;

	  irr::core::line3df ray; // Make a ray. This will start outside the mesh, looking in
	  ray.start.X = maxX + 0.1;
	  ray.start.Y = yTestPos;
	  ray.start.Z = zTestPos;
	  ray.end = ray.start;
	  ray.end.X = minX - 0.1;

	  // Check the ray and add the contact point if it exists
	  addContactPointFromRay(ray, ySpacing * zSpacing);
	  // swap ray direction and check again
	  ray.start.X = minX - 0.1;
	  ray.end.X = maxX + 0.1;
	  addContactPointFromRay(ray, ySpacing * zSpacing);
        }
    }

  // We don't want to do further triangle selection with the ship, so set the selector to null
  ship->setTriangleSelector(0);
  triangleSelectorEnabled = false;

  mDevice->getLogger()->log("Own ship points found: ");
  mDevice->getLogger()->log(irr::core::stringw((int)contactPoints.size()).c_str());
}

void OwnShip::addContactPointFromRay(irr::core::line3d<irr::f32> ray, irr::f32 contactArea)
{
  irr::core::vector3df intersection;
  irr::core::triangle3df hitTriangle;

  irr::scene::ISceneNode *selectedSceneNode =
    mDevice->getSceneManager()->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
												 ray,
												 intersection,      // This will be the position of the collision
												 hitTriangle,       // This will be the triangle hit in the collision
												 IDFlag_IsPickable, // (bitmask)
												 0);                // Check all nodes

  if (selectedSceneNode)
    {
      ContactPoint contactPoint;
      contactPoint.position = intersection;
      contactPoint.normal = hitTriangle.getNormal().normalize();
      contactPoint.position.Y -= heightCorrection; // Adjust for height correction

      // Check if the normal is pointing 'towards' the incoming ray used to find the contact point, i.e. if it is pointing in roughly the right direction
      // 0.707 is approximately cos(45deg), so should be within +- 45 degrees of the incoming ray.
      if (contactPoint.normal.dotProduct(ray.getVector().normalize()) < -0.707)
        {

	  // Find an internal node position, i.e. a point at which a ray check for internal intersection can start
	  ray.start = contactPoint.position;
	  // leave ray.end as the same as before
	  // Check for the internal node
	  selectedSceneNode =
	    mDevice->getSceneManager()->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
													 ray,
													 intersection,      // This will be the position of the collision
													 hitTriangle,       // This will be the triangle hit in the collision
													 IDFlag_IsPickable, // (bitmask)
													 0);                // Check all nodes

	  if (selectedSceneNode)
            {
	      contactPoint.internalPosition = intersection;
	      contactPoint.internalPosition.Y -= heightCorrection; // Adjust for height correction

	      // Adjust internal position, so it's only 1/2 way to the opposite boundary of the model
	      contactPoint.internalPosition = 0.5 * contactPoint.internalPosition + 0.5 * contactPoint.position;

	      // Find cross product, for torque component
	      irr::core::vector3df crossProduct = contactPoint.position.crossProduct(contactPoint.normal);
	      contactPoint.torqueEffect = crossProduct.Y;

	      // Store effective area represented by the contact
	      contactPoint.effectiveArea = contactArea;

	      // Store the contact point that we have found
	      contactPoints.push_back(contactPoint); // Store
            }
        }
    }
}

void OwnShip::setRateOfTurn(irr::f32 rateOfTurn) // Sets the rate of turn (used when controlled as secondary)
{
  controlMode = MODE_AUTO; // Switch to controlled mode
  this->mMu[1] = rateOfTurn;
}

irr::f32 OwnShip::getRateOfTurn() const
{
  return this->mMu[1];
}


void OwnShip::setWheel(irr::f32 aWheel)
{
  controlMode = MODE_ENGINE; // Switch to engine and rudder mode
  // Set the wheel (-ve is port, +ve is stbd), unless follow up rudder isn't working (overrideable with 'force')
  mWheel = aWheel;
  if (mWheel < -(mRudder.getDeltaMax())*180/M_PI)
    {
      mWheel = -(mRudder.getDeltaMax()*180/M_PI);
    }
  if (mWheel > mRudder.getDeltaMax()*180/M_PI)
    {
      mWheel = mRudder.getDeltaMax()*180/M_PI;
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

irr::f32 OwnShip::getCOG() const
{
  return mEta[2];
}

irr::f32 OwnShip::getSOG() const
{
  return mMu[0];
}

bool OwnShip::isSingleEngine() const
{
  return singleEngine;
}

irr::f32 OwnShip::getShipMass() const
{
  return mM;
}

std::string OwnShip::getBasePath() const
{
  return basePath;
}

bool OwnShip::isBuoyCollision() const
{
  return buoyCollision;
}

bool OwnShip::isOtherShipCollision() const
{
  return otherShipCollision;
}

void OwnShip::enableTriangleSelector(bool selectorEnabled)
{

  // Only re-set if we need to change the state

  if (selectorEnabled && !triangleSelectorEnabled)
    {
      ship->setTriangleSelector(selector);
      triangleSelectorEnabled = true;
    }

  if (!selectorEnabled && triangleSelectorEnabled)
    {
      ship->setTriangleSelector(0);
      triangleSelectorEnabled = false;
    }
}

irr::f32 OwnShip::getScaleFactor() const
{
  return scaleFactor;
}


irr::f32 OwnShip::getLastDeltaTime()
{
  return deltaTime;
}

void OwnShip::setLastDeltaTime(irr::f32 myDeltaTime)
{
  deltaTime = myDeltaTime;
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

void OwnShip::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::f32 weather, irr::core::vector3df linesForce, irr::core::vector3df linesTorque)
{

  setLastDeltaTime(deltaTime);

  if (controlMode == MODE_ENGINE)
    {
      // Check depth and update collision response forces and torque
      irr::f32 groundingAxialDrag = 0;
      irr::f32 groundingLateralDrag = 0;
      irr::f32 groundingTurnDrag = 0;
      collisionDetectAndRespond(groundingAxialDrag, groundingLateralDrag, groundingTurnDrag); // The drag values will get modified by this call

      // Add in response from mooring lines here
      groundingAxialDrag -= linesForce.Z;
      groundingLateralDrag -= linesForce.X;
      groundingTurnDrag -= linesTorque.Y;

      // std::cout << "Collision forces (Time/axial/lateral/turn)," << scenarioTime << "," << groundingAxialDrag << "," << groundingLateralDrag << "," << groundingTurnDrag << std::endl;

      // Add drag from wind and stream
      irr::f32 windSpeed = mModel->getWindSpeed() * KTS_TO_MPS;
      irr::f32 windDirection = mModel->getWindDirection();
      // Convert this into wind axial speed and wind lateral speed
      irr::f32 windFlowDirection = windDirection + 180; // Wind direction is where the wind is from. We want where it is flowing towards
      irr::f32 relativeWindFlowDirection = windFlowDirection - mEta[2];
      irr::f32 axialWind = windSpeed * cos(relativeWindFlowDirection * irr::core::DEGTORAD);
      irr::f32 lateralWind = windSpeed * sin(relativeWindFlowDirection * irr::core::DEGTORAD);

      irr::f32 relWindAxial_mps = (axialWind - mMu[0]) * KTS_TO_MPS;
      irr::f32 relWindLateral_mps = (lateralWind - mMu[2]) * KTS_TO_MPS;
      irr::f32 frontalArea = mGeoParams.b * airDraught;
      irr::f32 sideArea = mGeoParams.lPP * airDraught;

      irr::f32 axialWindDrag = -1 * pow(relWindAxial_mps, 2) * sign(relWindAxial_mps) * 0.5 * RHO_AIR * frontalArea;
      irr::f32 lateralWindDrag = -1 * pow(relWindLateral_mps, 2) * sign(relWindLateral_mps) * 0.5 * RHO_AIR * sideArea;

      // Find tidal stream, based on our current absolute position
      irr::core::vector2df stream = mModel->getTidalStream(mModel->getLong(), mModel->getLat(), mModel->getTimestamp());
      //std::cout << "Tidal stream x:" << stream.X << ", z:" << stream.Y << std::endl;
      irr::f32 streamScaling = fmax(0, fmin(1, getDepth())); // Reduce effect as water gets shallower
      stream *= streamScaling;
      // Convert this into stream axial and lateral speed
      irr::f32 axialStream = stream.X * sin(mEta[2] * irr::core::DEGTORAD) + stream.Y * cos(mEta[2] * irr::core::DEGTORAD); // Stream in ahead direction
      irr::f32 lateralStream = stream.X * cos(mEta[2] * irr::core::DEGTORAD) - stream.Y * sin(mEta[2] * irr::core::DEGTORAD);// Stream in stbd direction

      mSpeedThroughWater = mMu[0] - axialStream;

      irr::f32 alpha = (windDirection - mEta[2]);
      alpha = alpha * irr::core::DEGTORAD;

      irr::f32 apparentWindSpd = sqrt(pow(mSpeedThroughWater, 2) + pow((windSpeed * MPS_TO_KTS), 2) + (2 * mSpeedThroughWater * (windSpeed * MPS_TO_KTS) * cos(alpha)));
      //irr::f32 apparentWindDir = acos((speedThroughWater + ((windSpeed * MPS_TO_KTS) * cos(alpha))) / apparentWindSpd);

      irr::f32 apparentWindDir = atan2(windSpeed * MPS_TO_KTS * sin(alpha), mSpeedThroughWater + windSpeed * MPS_TO_KTS * cos(alpha));

      mModel->setApparentWindDir(apparentWindDir);
      mModel->setApparentWindSpd(apparentWindSpd);

      float sailsForceX = 0, sailsForceY = 0;
      if(windDirection > 180)
	windDirection = 180-(windDirection-180);

      if (mSailsCount > 0)
	{
	  sailsForceX = mSails.GetForce('X', mSpeedThroughWater, windSpeed * MPS_TO_KTS, (apparentWindDir * irr::core::RADTODEG));
	  sailsForceY = mSails.GetForce('Y', mSpeedThroughWater, windSpeed * MPS_TO_KTS, (apparentWindDir * irr::core::RADTODEG));
	  //std::cout << "Sail force X = " << sailsForceX << std::endl;
	  //std::cout << "Sail force Y = " << sailsForceY << std::endl;
	}

    
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


      
      mRudder.SetDelta((mWheel*M_PI)/180, deltaTime);
      
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
  waveHeightFiltered = (1 - factor) * waveHeightFiltered + factor * mModel->getWaveHeight(mEta[1], mEta[0]); // TODO: Check implementation of simple filter!
  double yPos = tideHeight + heightCorrection + waveHeightFiltered;

  // calculate pitch and roll - not linked to water/wave motion
  if (pitchPeriod > 0)
    {
      pitch = weather * pitchAngle * sin(scenarioTime * 2 * PI / pitchPeriod);
    }
  if (rollPeriod > 0)
    {
      roll = weather * rollAngle * sin(scenarioTime * 2 * PI / rollPeriod);
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
  //ship->setPosition(irr::core::vector3df(xPos, yPos, zPos));
  std::cout << "--> Xeta : " << mEta[0] << std::endl;
  std::cout << "--> Yeta : " << mEta[1] << std::endl;
  std::cout << "--> Hdg : " << mEta[2]*180/M_PI << std::endl;
  std::cout << "--> Speed X : " << mMu[0] << std::endl;
  std::cout << "--> Speed Y : " << mMu[1] << std::endl;
  std::cout << "--> Speed Z : " << mMu[2] << std::endl;
  std::cout << "--> Revs : " << mProp.getRevs() << std::endl;
  std::cout << "--> Rudder : " << mRudder.getDelta() << std::endl;
  std::cout << "--> Wheel : " << mWheel << std::endl;
  std::cout << "--> Forward Rotation : " << mProp.getForwardRotDir() << std::endl;
  std::cout << "--> Current Rotation : " << mProp.getCurrentRotDir() << std::endl;
  std::cout << "**************" << std::endl;
  
  //std::cout << "--> Xpos : " << xPos << std::endl;
  //std::cout << "--> Y : " << mEta[1] << std::endl;
  ship->setPosition(irr::core::vector3df(mEta[1], yPos, mEta[0]));
  // DEE_DEC22 vvvv the original remains however this could be a replacement
  //    ship->setRotation(Angles::irrAnglesFromYawPitchRoll(hdg+angleCorrection,angleCorrectionPitch+pitch,angleCorrectionRoll+roll)); // attempt 1
  //    ship->setRotation(irr::core::vector3df(angleCorrectionPitch+pitch, hdg+angleCorrection,angleCorrectionRoll+roll));
  //ship->setRotation(Angles::irrAnglesFromYawPitchRoll(hdg + angleCorrection, pitch, roll)); // this is the original
  ship->setRotation(Angles::irrAnglesFromYawPitchRoll(mEta[2]*180/M_PI, pitch, roll));
  // DEE_DEC22 ^^^^
}

irr::f32 OwnShip::getSpeedThroughWater() const
{
  return mSpeedThroughWater; // m/s
}

irr::f32 OwnShip::getLateralSpeed() const
{
  return mMu[2]; 
}

irr::f32 OwnShip::getDepth() const
{
  return -1 * mTerrain->getHeight(mEta[1], mEta[0]) + getPosition().Y;
}

void OwnShip::collisionDetectAndRespond(irr::f32 &reaction, irr::f32 &lateralReaction, irr::f32 &turnReaction)
{

#ifdef WITH_PROFILING
  IPROF_FUNC;
#endif

  reaction = 0;
  lateralReaction = 0;
  turnReaction = 0;

  buoyCollision = false;
  otherShipCollision = false;

  // Normal ship model
  ship->updateAbsolutePosition();
  irr::core::matrix4 rot;
  rot.setRotationDegrees(ship->getRotation());
  irr::core::vector3df shipAbsolutePosition = ship->getAbsolutePosition();

  for (int i = 0; i < contactPoints.size(); i++)
    {
      irr::core::vector3df pointPosition = contactPoints.at(i).position;
      irr::core::vector3df pointPositionForNormal = pointPosition + contactPoints.at(i).normal;
      irr::core::vector3df internalPointPosition = contactPoints.at(i).internalPosition;

      // Rotate with own ship
      rot.transformVect(pointPosition);
      rot.transformVect(pointPositionForNormal);
      rot.transformVect(internalPointPosition);

      pointPosition += shipAbsolutePosition;
      pointPositionForNormal += shipAbsolutePosition;
      internalPointPosition += shipAbsolutePosition;

      irr::f32 localIntersection = 0; // Ready to use

      // Find depth below the contact point
      irr::f32 localDepth = -1 * mTerrain->getHeight(pointPosition.X, pointPosition.Z) + pointPosition.Y;

      // Contact model (proof of principle!)
      if (localDepth < 0)
	{
	  localIntersection = -1 * localDepth * std::abs(contactPoints.at(i).normal.Y); // Projected based on normal, so we get an estimate of the intersection normal to the contact point. Ideally this vertical component of the normal would react to the ship's motion, but probably not too important
	}

      irr::f32 remotePointAxialSpeed = 0;
      irr::f32 remotePointLateralSpeed = 0;

      // Also check contact with pickable scenery elements here (or other ships?)
      irr::core::line3d<irr::f32> ray(internalPointPosition, pointPosition);
      irr::core::vector3df intersection;
      irr::core::triangle3df hitTriangle;
      irr::scene::ISceneNode *selectedSceneNode =
	mDevice->getSceneManager()->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
												     ray,
												     intersection,      // This will be the position of the collision
												     hitTriangle,       // This will be the triangle hit in the collision
												     IDFlag_IsPickable, // (bitmask)
												     0);                // Check all nodes

      // Check normal directions of contact triangle  - if they are pointing in the same direction, then we are on the 'free' side of the contact, and can ignore it
      if (selectedSceneNode)
	{
	  // First find the normal of the contact point (on the ship) in world coordinates
	  irr::core::vector3df worldCoordsShipNormal = pointPositionForNormal - pointPosition;
	  if (hitTriangle.getNormal().dotProduct(worldCoordsShipNormal) > 0)
	    {
	      // Ignore this contact
	      selectedSceneNode = 0;
	    }
	}

      // If this returns something, we must be in contact, so find distance between intersection and pointPosition
      if (selectedSceneNode && std::string(selectedSceneNode->getName()).find("LandObject") == 0)
	{

	  irr::f32 collisionDistance = pointPosition.getDistanceFrom(intersection);

	  // If we're more collided with an object than the terrain, use this
	  if (collisionDistance > localIntersection)
	    {
	      localIntersection = collisionDistance;
	    }
	}

      // Also check for buoy collision
      if (selectedSceneNode && std::string(selectedSceneNode->getName()).find("Buoy") == 0)
	{
	  buoyCollision = true;
	}

      // And for other ship collision
      if (selectedSceneNode && std::string(selectedSceneNode->getName()).find("OtherShip") == 0)
	{
	  otherShipCollision = true;

	  irr::s32 otherShipID = -1;
	  // Find other ship ID from name (should be OtherShip_#)
	  std::vector<std::string> splitName = Utilities::split(std::string(selectedSceneNode->getName()), '_');
	  if (splitName.size() == 2)
	    {
	      otherShipID = Utilities::lexical_cast<irr::s32>(splitName.at(1));
	    }
	  // std::cout << "In contact with " << std::string(selectedSceneNode->getName()) << " Length of split: " << splitName.size() << std::endl;

	  // Testing: behave as if other ship is solid. In multiplayer, the other ship (if another 'player') should also respond
	  irr::f32 collisionDistance = pointPosition.getDistanceFrom(intersection);
	  // If we're more collided with an object than the terrain, use this
	  if (collisionDistance > localIntersection)
	    {
	      localIntersection = collisionDistance;

	      // Calculate velocity of other ship, in our reference frame
	      if (otherShipID >= 0)
		{
		  irr::f32 otherShipHeading = mModel->getOtherShipHeading(otherShipID);
		  irr::f32 otherShipSpeed = mModel->getOtherShipSpeed(otherShipID);
		  irr::f32 otherShipRelativeHeading = otherShipHeading - mEta[2];

		  // TODO: Initially ignore rate of turn of other ship, but should be included
		  remotePointAxialSpeed = otherShipSpeed * cos(irr::core::DEGTORAD * otherShipRelativeHeading);
		  remotePointLateralSpeed = otherShipSpeed * sin(irr::core::DEGTORAD * otherShipRelativeHeading);
		}
	    }
	}

      // Contact model (proof of principle!)
      if (localIntersection > 5)
	{
	  localIntersection = 5; // Limit to 5m intersection
	}

      if (localIntersection > 0)
	{
	  // Simple 'proof of principle' values initially
	  // reaction += localIntersection*100*maxForce * sign(axialSpd,0.1);
	  // lateralReaction += localIntersection*100*maxForce * sign(lateralSpd,0.1);
	  // turnReaction += localIntersection*100*maxForce * sign(rateOfTurn,0.1);

	  // Find effective area of contact point
	  irr::f32 contactArea = contactPoints.at(i).effectiveArea;

	  // Define stiffness & damping
	  irr::f32 contactStiffness = mModelParams.contactStiffnessFactor * contactArea;                         // N/m per m2 * area
	  irr::f32 contactDamping = mModelParams.contactDampingFactor * 2.0 * sqrt(contactStiffness * mM); // Critical damping, assuming that only one point is in contact, and that mass of own ship is the smaller in two body contact...

	  // Local speed at this point (TODO, include y component from pitch and roll?)
	  //  Relative to the speed of the point we're in contact with
	  irr::core::vector3df localSpeedVector;
	  localSpeedVector.X = mMu[2] + mMu[1] * contactPoints.at(i).position.Z - remotePointLateralSpeed;
	  localSpeedVector.Y = 0;
	  localSpeedVector.Z = mMu[0] - mMu[1] * contactPoints.at(i).position.X - remotePointAxialSpeed;

	  // Find the speed component, tangential to the contact plane (for friction)
	  irr::core::vector3df tangentialSpeedComponent;
	  // Find this here, by subtracting the part normal to the contact plane
	  // part normal to the contact plane is speedVector.normal * normal (normal is already normalised length)
	  tangentialSpeedComponent = localSpeedVector - localSpeedVector.dotProduct(contactPoints.at(i).normal) * contactPoints.at(i).normal;

	  irr::f32 tangentialSpeedAmplitude = tangentialSpeedComponent.getLength();
	  irr::core::vector3df normalisedTangentialSpeedComponent = tangentialSpeedComponent; // Normalised, so we just have the direction
	  normalisedTangentialSpeedComponent.normalize();

	  // Simple 'stiffness' based response
	  irr::f32 reactionForce = localIntersection * contactStiffness;
	  // Damping: Project localSpeedVector onto contact normal. Damping reaction force is proportional to this, and can be applied like the main reaction force
	  irr::f32 normalSpeed = localSpeedVector.dotProduct(contactPoints.at(i).normal);
	  irr::f32 dampingForce = normalSpeed * contactDamping;

	  // Find combined stiffness and damping effect. Only allow to be positive, so no 'sticking'
	  irr::f32 combinedStiffnessDamping = reactionForce + dampingForce;
	  if (combinedStiffnessDamping < 0.0)
	    {
	      combinedStiffnessDamping = 0.0;
	    }

	  // Apply this force
	  turnReaction += combinedStiffnessDamping * contactPoints.at(i).torqueEffect;
	  reaction += combinedStiffnessDamping * contactPoints.at(i).normal.Z;
	  lateralReaction += combinedStiffnessDamping * contactPoints.at(i).normal.X;

	  // Friction response. Use tanh function for better stability at low speed
	  irr::f32 frictionTorqueFactor = (contactPoints.at(i).position.crossProduct(normalisedTangentialSpeedComponent)).Y; // Effect of unit friction force on ship's turning. TODO: Check this, I think it's correct
	  irr::f32 frictionCoeff = mModelParams.frictionCoefficient * tanh(mModelParams.tanhFrictionFactor * tangentialSpeedAmplitude);
	  turnReaction += combinedStiffnessDamping * frictionCoeff * frictionTorqueFactor;
	  reaction += combinedStiffnessDamping * frictionCoeff * normalisedTangentialSpeedComponent.Z;
	  lateralReaction += combinedStiffnessDamping * frictionCoeff * normalisedTangentialSpeedComponent.X;

	  // std::cout << "remotePointAxialSpeed: " << remotePointAxialSpeed << std::endl;

	  if (mShowDebugData)
	    {
	      // Show points in contact in red
	      irr::core::position2d<irr::s32> contactPoint2d = mDevice->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(
																			  pointPosition, mDevice->getSceneManager()->getActiveCamera(), false);
	      irr::core::position2d<irr::s32> contactPoint2dNormal = mDevice->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(
																				pointPositionForNormal, mDevice->getSceneManager()->getActiveCamera(), false);
	      mDevice->getVideoDriver()->draw2DPolygon(contactPoint2d, 10, irr::video::SColor(100, 255, 0, 0));
	      mDevice->getVideoDriver()->draw2DLine(contactPoint2d, contactPoint2dNormal, irr::video::SColor(100, 255, 0, 0));
	    }
	}
      else
	{
	  // Not in contact at this point
	  if (mShowDebugData)
	    {
	      // Show points not in contact
	      irr::video::SColor pointColour;
	      if (contactPoints.at(i).torqueEffect > 0)
		{
		  pointColour = irr::video::SColor(100, 0, 255, 0);
		}
	      else
		{
		  pointColour = irr::video::SColor(100, 0, 0, 255);
		}

	      irr::core::position2d<irr::s32> contactPoint2d = mDevice->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(
																			  pointPosition, mDevice->getSceneManager()->getActiveCamera(), false);
	      irr::core::position2d<irr::s32> contactPoint2dNormal = mDevice->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(
																				pointPositionForNormal, mDevice->getSceneManager()->getActiveCamera(), false);
	      mDevice->getVideoDriver()->draw2DPolygon(contactPoint2d, 10, pointColour);
	      mDevice->getVideoDriver()->draw2DLine(contactPoint2d, contactPoint2dNormal, pointColour);
	    }
	}
      // contactDebugPoints.at(i*2)->setPosition(internalPointPosition);
      // contactDebugPoints.at(i*2 + 1)->setPosition(internalPointPosition);
    }

  // If showing debug data, draw a big circle series for the model centre
  if (mShowDebugData)
    {
      irr::core::position2d<irr::s32> centrePosition2d = mDevice->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(
																		    ship->getAbsolutePosition(), mDevice->getSceneManager()->getActiveCamera(), false);
      mDevice->getVideoDriver()->draw2DPolygon(centrePosition2d, 5, irr::video::SColor(100, 0, 255, 0));
      mDevice->getVideoDriver()->draw2DPolygon(centrePosition2d, 10, irr::video::SColor(100, 0, 255, 0));
      mDevice->getVideoDriver()->draw2DPolygon(centrePosition2d, 15, irr::video::SColor(100, 0, 255, 0));
      mDevice->getVideoDriver()->draw2DPolygon(centrePosition2d, 20, irr::video::SColor(100, 0, 255, 0));
      mDevice->getVideoDriver()->draw2DPolygon(centrePosition2d, 25, irr::video::SColor(100, 0, 255, 0));
    }
}

irr::f32 OwnShip::getAngleCorrection() const
{
  return angleCorrection;
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
