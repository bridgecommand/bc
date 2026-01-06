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
#include "IniFile.hpp"

//using namespace irr;

Ship::Ship()
{
  //Default to run on defined spd and hdg
  mControlMode = MODE_AUTO;
  positionManuallyUpdated = false; //Used to track if position has been manually updated, and shouldn't have position update applied this loop
  mMsi = 0;

  mMu0 << 0, 0, 0;
  mM = 0; 
  mMX = 0; 
  mMY = 0; 
  mMatM << 0, 0, 0, 0, 0, 0, 0, 0, 0; 
  mInvMatM << 0, 0, 0, 0, 0, 0, 0, 0, 0;  
  mMu << 0, 0, 0; 
  mEta << 0, 0, 0;
  mSpeedThroughWater = 0;

  mGeoParams = {0}; 
  mAddedMassParams = {0};

}

Ship::~Ship()
{
  //dtor
}

void Ship::PrintGeoParams(void)
{
  std::cout << "::::::Geometric Ship Parameters::::::" << std::endl;
  std::cout << "Ship length : " << mGeoParams.lPP << std::endl;
  std::cout << "Ship breadth : " << mGeoParams.b << std::endl;
  std::cout << "Ship draught : " << mGeoParams.d << std::endl;
  std::cout << "Subwater volume : " << mGeoParams.volume << std::endl;
  std::cout << "Longitudinal coordinate of center of gravity of ship : " << mGeoParams.xG << std::endl;
  std::cout << "Coefficient Block : " << mGeoParams.cB << std::endl;
  std::cout << "::::::::::::" << std::endl;
}

void Ship::PrintAddedMassParams(void)
{
  std::cout << "::::::Added Mass Parameters::::::" << std::endl;
  std::cout << "Added masses of x axis direction : " << mAddedMassParams.mpX << std::endl;
  std::cout << "Added masses of y axis direction : " << mAddedMassParams.mpY << std::endl;
  std::cout << "Added moment of inertia : " << mAddedMassParams.jpZ << std::endl;
  std::cout << "::::::::::::" << std::endl;
}

int Ship::InitShipParams(const Json::Value& aJsonRoot)
{
  int ret = 0;
  
  if(aJsonRoot.isNull())
    {
      ret = -1;
    }
  else
    {
      //Geo Params
      mGeoParams.lPP = aJsonRoot["geoParams"]["length"].asFloat();
      mGeoParams.b = aJsonRoot["geoParams"]["breadth"].asFloat();
      mGeoParams.d = aJsonRoot["geoParams"]["draugth"].asFloat();
      mGeoParams.volume = aJsonRoot["geoParams"]["subwaterVolume"].asFloat();
      mGeoParams.xG = aJsonRoot["geoParams"]["longGravityCenter"].asFloat();
      mGeoParams.cB = aJsonRoot["geoParams"]["blockCoef"].asFloat();
      PrintGeoParams();

      //Added-Mass Params
      mAddedMassParams.mpX = aJsonRoot["addedMass"]["pX"].asFloat();
      mAddedMassParams.mpY = aJsonRoot["addedMass"]["pY"].asFloat();
      mAddedMassParams.jpZ = aJsonRoot["addedMass"]["pZ"].asFloat();
      PrintAddedMassParams();
      
      //Hull
      mHull.Init(aJsonRoot["hull"]["xp0"].asFloat(),
		 aJsonRoot["hull"]["xpVV"].asFloat(),
		 aJsonRoot["hull"]["xpVR"].asFloat(),
		 aJsonRoot["hull"]["xpRR"].asFloat(),
		 aJsonRoot["hull"]["xpVVVV"].asFloat(),
		 aJsonRoot["hull"]["ypV"].asFloat(),
		 aJsonRoot["hull"]["ypR"].asFloat(),
		 aJsonRoot["hull"]["ypVVV"].asFloat(),
		 aJsonRoot["hull"]["ypVVR"].asFloat(),
		 aJsonRoot["hull"]["ypVRR"].asFloat(),
		 aJsonRoot["hull"]["ypRRR"].asFloat(),
		 aJsonRoot["hull"]["npV"].asFloat(),
		 aJsonRoot["hull"]["npR"].asFloat(),
		 aJsonRoot["hull"]["npVVV"].asFloat(),
		 aJsonRoot["hull"]["npVVR"].asFloat(),
		 aJsonRoot["hull"]["npVRR"].asFloat(),
		 aJsonRoot["hull"]["npRRR"].asFloat()
		 );
      mHull.PrintParams();

      //Propeller
      mNumberProp = aJsonRoot["propeller"]["number"].asInt();
      for(unsigned char i=0;i<mNumberProp;i++)
	{
	  mProp[i].Init(aJsonRoot["propeller"]["diameter"].asFloat(),
			aJsonRoot["propeller"]["thrustFactor"].asFloat(),
			aJsonRoot["propeller"]["longPosition"].asFloat(),
			aJsonRoot["propeller"]["nominalWake"].asFloat(),
			aJsonRoot["propeller"]["k0"].asFloat(),
			aJsonRoot["propeller"]["k1"].asFloat(),
			aJsonRoot["propeller"]["k2"].asFloat(),
			aJsonRoot["propeller"]["forwardRotDir"].asString(),
			aJsonRoot["propeller"]["backwardEff"].asFloat()
			);
	  mProp[i].PrintParams();

	  //Engine 
	  mEngine[i].Init(aJsonRoot["engine"]["brand"].asString(),
			  aJsonRoot["engine"]["type"].asString(),
			  aJsonRoot["engine"]["power"].asFloat(),
			  aJsonRoot["engine"]["rpmMax"].asFloat(),
			  aJsonRoot["engine"]["fuelCons"].asFloat()		  
			  );
	  mEngine[i].PrintParams();
	}

      //Rudder
      mRudder.Init(aJsonRoot["rudder"]["spanLength"].asFloat(),
		   aJsonRoot["rudder"]["areaMobPart"].asFloat(),
		   aJsonRoot["rudder"]["longCoordinateRatio"].asFloat(),
		   aJsonRoot["rudder"]["forceIncreaseFactor"].asFloat(),
		   aJsonRoot["rudder"]["steeringResistanceFactor"].asFloat(),
		   aJsonRoot["rudder"]["longLateralForce"].asFloat(),
		   aJsonRoot["rudder"]["wakeFractionRatio"].asFloat(),
		   aJsonRoot["rudder"]["expConstUr"].asFloat(),
		   aJsonRoot["rudder"]["longCoordinatePos"].asFloat(),
		   aJsonRoot["rudder"]["aspectRatio"].asFloat(),
		   {aJsonRoot["rudder"]["flowCoef"][0].asFloat(), aJsonRoot["rudder"]["flowCoef"][1].asFloat()},
		   aJsonRoot["rudder"]["maxSpeed"].asFloat(),
		   aJsonRoot["rudder"]["maxAngle"].asFloat()
		   );
      mRudder.PrintParams();

      //Sail
      int sailNumber = aJsonRoot["sail"]["number"].asInt();
      float sailPos[SAILS_MAX][3] = {0};
      
      for(unsigned char i=0;i<sailNumber;i++)
	{
	  sailPos[i][0] = aJsonRoot["sail"]["pos"][i][0].asFloat();
	  sailPos[i][1] = aJsonRoot["sail"]["pos"][i][1].asFloat();
	  sailPos[i][2] = aJsonRoot["sail"]["pos"][i][2].asFloat();
	}
	    
      mSails.Init(sailNumber,
		  aJsonRoot["sail"]["type"].asString(),
		  aJsonRoot["sail"]["size"].asString(),
		  sailPos
		  );
      
      mSails.PrintParams();
    }

  return ret;
}

int Ship::InitShipParams(const std::string& aType)
{
  int ret = 0;

  /*TODO : Get datas from parameters file*/
  if("kvlcc2" == aType)
    {
      mGeoParams = {320, 58, 20.8, 312622, 11.1, 0.81};
      mHull.Init(0.022,-0.04, 0.002, 0.011, 0.771, -0.315, 0.083, -1.607, 0.379, -0.391, 0.008, -0.137, -0.049, -0.03, -0.294, 0.055, -0.013);
      mAddedMassParams = {0.022, 0.223, 0.011};
      mNumberProp=1;
      mProp[0].Init(9.86, 0.22, -0.48, 0.35, 0.293, -0.275, -0.139, "right", 0.67);
      mRudder.Init(15.8, 112.5, -0.5, 0.312, 0.387, -0.464, 1.09, 0.5, -0.71, 1.827, {0.395, 0.64}, 0.0407, 0.61);
      // mShipWindParams = {4910, 1624, 750, 375, 160, 0, 1.225, 0 * 15.5 * 0.514, 90};
      mEngine[0].Init("Caterpillar", "9M32C", 4500, 170, 177);
    }
  else
    {
      ret = -1;
    }

  return ret;
}

Sail& Ship::getSail(void)
{
  return mSails;
}

irr::scene::IMeshSceneNode* Ship::getSceneNode() const
{
  return (irr::scene::IMeshSceneNode*)mShipScene;
}

irr::core::vector3df Ship::getRotation() const
{
  return mShipScene->getRotation();
}

irr::core::vector3df Ship::getPosition() const
{
  mShipScene->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
  return mShipScene->getAbsolutePosition();
}

irr::f32 Ship::getLength() const
{
  return mGeoParams.lPP;
}

irr::f32 Ship::getBreadth() const
{
  return mGeoParams.b;
}

irr::f32 Ship::getHeightCorrection() const
{
  return mHeightCorrection;
}

irr::f32 Ship::getDepth(Terrain *aTerrain) const
{
  if(NULL != aTerrain)
    return -1 * aTerrain->getHeight(mEta[1], mEta[0]) + getPosition().Y;
  else
    return -1;
}

irr::f32 Ship::getSpeedThroughWater() const
{
  return mSpeedThroughWater; // m/s
}

irr::f32 Ship::getLateralSpeed() const
{
  return mMu[2]; 
}


irr::f32 Ship::getEstimatedDisplacement() const
{
  irr::f32 seawaterDensity = 1024; // define seawater density in kg / m^3 could parametarise this for dockwater and freshwater
  irr::f32 typicalBlockCoefficient = 0.87; // 0.87 is typical block coefficient
  return mGeoParams.d * mGeoParams.b * mGeoParams.lPP * seawaterDensity * typicalBlockCoefficient;
}

void Ship::setPosition(irr::f32 xPos, irr::f32 zPos)
{
  //Update the position used, ready for next update. Doesn't actually move the mesh at this point
  mEta[1] = xPos;
  mEta[0] = zPos;
  positionManuallyUpdated = true;
}

void Ship::setHeading(irr::f32 hdg)
{
  mEta[2] = hdg;
  mControlMode = MODE_AUTO; //Switch to auto mode
}

void Ship::setSpeed(irr::f32 spd)
{
  mMu[0] = spd;
  mControlMode = MODE_AUTO; //Switch to auto mode
}

irr::f32 Ship::getRateOfTurn() const
{
  return mMu[1];
}

irr::f32 Ship::getHeading() const
{
  return mEta[2];
}

irr::f32 Ship::getSpeed() const
{
  return mMu[0];
}

irr::u32 Ship::getMMSI() const
{
  return mMsi;
}

void Ship::setMMSI(irr::u32 aMmsi)
{
  mMsi = aMmsi;
}

void Ship::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
  mEta[1] += deltaX;
  double yPos = deltaY;
  mEta[0] += deltaZ;
  mShipScene->setPosition(irr::core::vector3df(mEta[1],yPos,mEta[0]));
}


void Ship::setEta(Eigen::Vector3d aEta)
{
  mEta = aEta; 
}

void Ship::setMu(Eigen::Vector3d aMu)
{
  mMu = aMu;
}


Propeller& Ship::getPropeller(std::string aNProp)
{
  if(aNProp == "mono" || aNProp == "port")
    return mProp[0];
  else
    return mProp[1];
}


Hull& Ship::getHull(void){return mHull;}
Rudder& Ship::getRudder(void) {return mRudder;}

sGeoParams& Ship::getGeoParams(void){return mGeoParams;}
double Ship::getM(void){return mM;}
double Ship::getMX(void){return mMX;}
double Ship::getMY(void){return mMY;}
Eigen::Vector3d Ship::getMu0(void){return mMu0;}
Eigen::Vector3d Ship::getMu(void){return mMu;}
Eigen::Vector3d Ship::getEta(void){return mEta;}
Eigen::Matrix3d& Ship::getInvMatM(void){return mInvMatM;}
unsigned char Ship::getNumberProp(void){return mNumberProp;}
