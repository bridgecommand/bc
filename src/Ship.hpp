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

#ifndef __SHIP_HPP_INCLUDED__
#define __SHIP_HPP_INCLUDED__


#include <string>
#include <Eigen/Dense>
#include <json/json.h>
#include "irrlicht.h"
#include "ShipGlobalParams.hpp"
#include "Propeller.hpp"
#include "Hull.hpp"
#include "Rudder.hpp"
#include "Terrain.hpp"
#include "Sail.hpp"
#include "Engine.hpp"

class Ship
{
public:
  Ship();
  virtual ~Ship();

  irr::scene::IMeshSceneNode* getSceneNode() const;
  irr::core::vector3df getRotation() const;
  irr::core::vector3df getPosition() const;
  float GetScaleFactor() const;


  float getLength() const;
  float getBreadth() const;
  float getHeightCorrection() const;
  float getEstimatedDisplacement() const;
  void setHeading(float hdg);
  void setSpeed(float spd);
  float getSpeedThroughWater() const; // m/s
  float getLateralSpeed() const;
  float getHeading() const;
  float getRateOfTurn() const;
  float getSpeed() const; //m/s
  void moveNode(float deltaX, float deltaY, float deltaZ);
  void setPosition(float xPos, float yPos);
  irr::u32 getMMSI() const;
  void setMMSI(irr::u32 mmsi);
  float getDepth(Terrain *aTerrain) const;
  int InitShipParams(const std::string& aType);
  int InitShipParams(const Json::Value& aJsonRoot);
  double getM(void);
  double getMY(void);
  double getMX(void);

  Eigen::Vector3d getMu0(void);
  Eigen::Matrix3d& getInvMatM(void);
  sGeoParams& getGeoParams(void);
  Eigen::Vector3d getEta(void);
  Eigen::Vector3d getMu(void);
  void setEta(Eigen::Vector3d aEta);
  void setMu(Eigen::Vector3d aMu);

  //Boat parts
  Propeller& getPropeller(std::string aNProp = "mono");
  Hull& getHull(void);
  Rudder& getRudder(void);
  unsigned char getNumberProp(void);
  Sail& getSail(void);

  //Print Ship params
  void PrintGeoParams(void);
  void PrintAddedMassParams(void);
  
protected:

  //Mesh Boat
  irr::scene::IMeshSceneNode* mShipScene; 

  //Mesh Correction
  float mHeightCorrection;
  float mAngleCorrection;
  float mScaleFactor;
  
  //Global Ship params
  sGeoParams mGeoParams; 
  sAddedMassParams mAddedMassParams;

  //Computed values
  double mM; //Mass
  double mMX; //Mass on Z
  double mMY; //Mass on X
  Eigen::Matrix3d mMatM; //Mass matrix
  Eigen::Matrix3d mInvMatM; //Inverse mass matrix
  Eigen::Vector3d mMu0; //Initial speed
  //Dynamic params
  Eigen::Vector3d mMu; //mMu[0] : Speed on Z ; mMu[1] :  Rate of turn ; mMu[2] : Speed on X (m/s) 
  Eigen::Vector3d mEta; //mEta[0] : Z position ; mEta[1] : X position ; mEta[2] : Heading
  double mSpeedThroughWater;

  //Boat parts
  unsigned char mNumberProp;
  Propeller mProp[2];
  Hull mHull;
  Rudder mRudder;
  Sail mSails;
  Engine mEngine[2];

  
  float angleCorrectionRoll;
  float angleCorrectionPitch;

  int controlMode;
  bool positionManuallyUpdated; //If position has been updated, and shouldn't be updated again this loop
  irr::u32 mmsi;
  enum CONTROL_MODE
    {
      MODE_AUTO = 0,
      MODE_ENGINE = 1
    };
};

#endif // __SHIP_HPP_INCLUDED__
