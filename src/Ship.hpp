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

#include "irrlicht.h"
#include <string>
#include <Eigen/Dense>
#include "ShipGlobalParams.hpp"
#include "Propeller.hpp"
#include "Hull.hpp"
#include "Rudder.hpp"

//Forward declarations
class SimulationModel;

class Ship
{
public:
  Ship();
  virtual ~Ship();

  irr::scene::IMeshSceneNode* getSceneNode() const;
  irr::core::vector3df getRotation() const;
  irr::core::vector3df getPosition() const;
  irr::f32 getLength() const;
  irr::f32 getBreadth() const;
  irr::f32 getHeightCorrection() const;
  irr::f32 getEstimatedDisplacement() const;
  void setHeading(irr::f32 hdg);
  void setSpeed(irr::f32 spd);
  irr::f32 getHeading() const;
  irr::f32 getRateOfTurn() const;
  irr::f32 getSpeed() const; //m/s
  void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);
  void setPosition(irr::f32 xPos, irr::f32 yPos);
  irr::u32 getMMSI() const;
  void setMMSI(irr::u32 mmsi);

  int setShipParams(const std::string& aType);
  double getM(void);
  double getMY(void);
  double getMX(void);
  double getRho(void);
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
  //Wind& getWind(void);
  unsigned char getNumberProp(void);

  
protected:

  irr::scene::IMeshSceneNode* ship; //The scene node for the own ship.
  irr::scene::IMeshSceneNode* mSailsScene[4];
  unsigned int mSailsCount;
  std::string mSailsType;
  irr::f32 airDraught;
  irr::f32 heightCorrection;
  irr::f32 angleCorrection;

  double mM; //Mass
  double mMX; //Mass on Z
  double mMY; //Mass on X
  double mRho; //~1024
  Eigen::Matrix3d mMatM; //Mass matrix
  Eigen::Matrix3d mInvMatM; //Inverse mass matrix
  Eigen::Vector3d mMu0; //Initial speed
  sGeoParams mGeoParams; //
  sAddedMassParams mAddedMassParams;
  //Dynamic params
  Eigen::Vector3d mMu; //mMu[0] : Speed on Z ; mMu[1] :  Rate of turn ; mMu[2] : Speed on X (m/s) 
  Eigen::Vector3d mEta; //mEta[0] : Z position ; mEta[1] : X position ; mEta[2] : Heading
  //Boat parts
  unsigned char mNumberProp;
  Propeller mProp[2];
  Hull mHull;
  Rudder mRudder;
  //Wind mWind;

  
  // DEE_DEC22 vvvv angle corrections about other axis to allow easier import of other cood systems models and to model trim and list
  irr::f32 angleCorrectionRoll;
  irr::f32 angleCorrectionPitch;
  // DEE_DEC22
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
