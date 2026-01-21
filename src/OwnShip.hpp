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

#ifndef __OWNSHIP_HPP_INCLUDED__
#define __OWNSHIP_HPP_INCLUDED__

#include <vector>
#include "irrlicht.h"
#include "Sail.hpp"
#include "Ship.hpp"
#include "OperatingModeEnum.hpp"
#include "Time.hpp"

#define MESH_VIEWS_MAX (5)

class OwnShipData;
class Terrain;
class Rain;
class Collision;
class Wind;
class Solver;
class Tide;
class Water;

class OwnShip : public Ship
{
public:

  OwnShip();
  ~OwnShip();
  int Load(OwnShipData aOwnShipData, Water *aWater, Tide *aTide, Terrain *aTerrain, irr::IrrlichtDevice *aDev);
  void Update(sTime& aTime, irr::f32 aTideHeight, irr::f32 aWeather, Wind *aWind, Solver *aSolver, irr::core::vector3d<int64_t> aOffsetMap);
  void InitOwnShipParams(OwnShipData aOwnShipData, Json::Value aJsonRoot);

  std::vector<irr::core::vector3df> getCameraViews() const;
  std::vector<bool> getCameraIsHighView() const;
  irr::core::vector3df getRadarPosition() const;
  irr::f32 getRadarSize() const;
  irr::f32 getRadarTilt() const;
  irr::f32 getAngleCorrection() const;
  void setWheel(irr::f32 aWheel);      // Set the wheel (-ve is port, +ve is stbd). Clamps to +-30 DEE. Set force to true to apply even if follow up rudder has failed
  //void setRudder(irr::f32 aDelta);
  void setPortEngine(irr::f32);                                 // Set the engine, (-ve astern, +ve ahead), range is +-1. This method limits the range applied
  void setStbdEngine(irr::f32);                                 // Set the engine, (-ve astern, +ve ahead), range is +-1. This method limits the range applied
  void setRateOfTurn(irr::f32 rateOfTurn);                      // Sets the rate of turn (used when controlled as secondary)

  irr::f32 getPortEngine() const; //-1 to 1
  irr::f32 getStbdEngine() const; //-1 to 1

  irr::f32 getWheel() const;            // DEE -30 to +30
  irr::f32 getPitch() const;
  irr::f32 getRoll() const;

  //Devices
  bool HasGPS(void) const;
  bool HasDepthSounder(void) const;
  float GetMaxSounderDepth(void) const;
  bool HasRoTIndicator() const ;  

  void PrintDevices(void);
  void PrintMeshInfos(void);
  
  std::string getBasePath() const;

  irr::f32 getShipMass() const;

 
protected:
private:
  irr::f32 requiredEngineProportion(irr::f32 speed);

  irr::IrrlichtDevice *mDevice;
  std::vector<irr::core::vector3df> mViews; 
  std::vector<bool> mIsHighView;
  bool mIsTransparent;
  
  std::string basePath; // The location the model is loaded from

  Terrain *mTerrain;
  Rain *mRain;
  Tide *mTide;
  Water *mWater;
  
  bool mShowDebugData;
  
  irr::f32 mRollPeriod;       // Roll period (s)  DEE this should be dynamically loaded
  irr::f32 mRollAngle;        // Roll Angle (deg)
  irr::f32 mPitchPeriod;      // Roll period (s)
  irr::f32 mPitchAngle;       // Roll Angle (deg)
  irr::f32 mBuffetPeriod;     // Yaw period (s)
  irr::f32 mBuffet;           // How much ship is buffeted by waves (undefined units)
  irr::f32 mPitch;            //(deg)
  irr::f32 mRoll;             //(deg)
  irr::f32 mPortEngine;       //-1 to + 1
  irr::f32 mStbdEngine;       //-1 to + 1
  irr::f32 mWheel;             //-30 to + 30
              //-30 to + 30
  bool mSingleEngine;

  // Dynamics parameters

  //Devices
  bool mHasGps;
  bool mHasDepthSounder;
  float mMaxSounderDepth;
  float mHasRoTIndicator;
  irr::core::vector3df mRadarPos;
  float mRadarSize;
  float mRadarTilt;

  
  float mWaveHeightFiltered; // 1st order transfer filtered response to waves

  
  // Debugging
  std::vector<irr::scene::IMeshSceneNode *> contactDebugPoints;
};

#endif // __OWNSHIP_HPP_INCLUDED__
