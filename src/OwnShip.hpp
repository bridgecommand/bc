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

#include "irrlicht.h"
#include <vector>
#include "Sail.hpp"
#include "Ship.hpp"
#include "OperatingModeEnum.hpp"
#include "Time.hpp"

// Forward declarations

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
  void load(OwnShipData aOwnShipData, Water *aWater, Tide *aTide, Terrain *aTerrain, irr::IrrlichtDevice *aDev);
  void update(sTime& aTime, irr::f32 tideHeight, irr::f32 weather, Wind *aWind, Solver *aSolver);
  std::vector<irr::core::vector3df> getCameraViews() const;
  std::vector<bool> getCameraIsHighView() const;
  irr::core::vector3df getRadarPosition() const;
  irr::f32 getRadarSize() const;
  irr::f32 getRadarTilt() const;
  std::string getRadarConfigFile() const;
  irr::f32 getAngleCorrection() const;
  bool hasGPS() const;
  bool hasDepthSounder() const;
  bool hasTurnIndicator() const;
  irr::f32 getMaxSounderDepth() const;
  void setRudder(irr::f32);                 // Set the rudder (-ve is port, +ve is stbd). Clamps to +-30
  void setWheel(irr::f32 aWheel);      // Set the wheel (-ve is port, +ve is stbd). Clamps to +-30 DEE. Set force to true to apply even if follow up rudder has failed

  void setPortEngine(irr::f32);                                 // Set the engine, (-ve astern, +ve ahead), range is +-1. This method limits the range applied
  void setStbdEngine(irr::f32);                                 // Set the engine, (-ve astern, +ve ahead), range is +-1. This method limits the range applied
  void setRateOfTurn(irr::f32 rateOfTurn);                      // Sets the rate of turn (used when controlled as secondary)

  irr::f32 getPortEngine() const; //-1 to 1
  irr::f32 getStbdEngine() const; //-1 to 1

  irr::f32 getWheel() const;            // DEE -30 to +30
  irr::f32 getPitch() const;
  irr::f32 getRoll() const;

  std::string getBasePath() const;

  irr::f32 getShipMass() const;
  irr::f32 getScaleFactor() const;
 
protected:
private:
  irr::f32 requiredEngineProportion(irr::f32 speed);
  irr::f32 sign(irr::f32 inValue) const;
  irr::f32 sign(irr::f32 inValue, irr::f32 threshold) const;

  irr::IrrlichtDevice *mDevice;
  std::vector<irr::core::vector3df> views; // The offset of the camera origin from the own ship origin
  std::vector<bool> isHighView;            // Should be the same size as views (todo: Make this into a struct with views)
  std::string radarConfigFile;
  std::string basePath; // The location the model is loaded from

  Terrain *mTerrain;
  Rain *mRain;
  Tide *mTide;
  Water *mWater;
  
  bool mShowDebugData;
  irr::f32 scaleFactor;
  irr::f32 rollPeriod;       // Roll period (s)  DEE this should be dynamically loaded
  irr::f32 rollAngle;        // Roll Angle (deg)
  irr::f32 pitchPeriod;      // Roll period (s)
  irr::f32 pitchAngle;       // Roll Angle (deg)
  irr::f32 buffetPeriod;     // Yaw period (s)
  irr::f32 buffet;           // How much ship is buffeted by waves (undefined units)
  irr::f32 pitch;            //(deg)
  irr::f32 roll;             //(deg)
  irr::f32 portEngine;       //-1 to + 1
  irr::f32 stbdEngine;       //-1 to + 1
  irr::f32 mWheel;             //-30 to + 30
              //-30 to + 30
  bool singleEngine;

  irr::core::vector3df mRadarPos;
  irr::f32 mRadarSize;
  irr::f32 mRadarTilt;
  // Dynamics parameters
  

  irr::f32 turnIndicatorPresent;
  irr::f32 waveHeightFiltered; // 1st order transfer filtered response to waves
  // General settings
  bool gps;
  bool depthSounder;
  irr::f32 maxSounderDepth;

  
  // Debugging
  std::vector<irr::scene::IMeshSceneNode *> contactDebugPoints;
};

#endif // __OWNSHIP_HPP_INCLUDED__
