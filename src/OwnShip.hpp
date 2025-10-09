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

// Forward declarations
class SimulationModel;
class OwnShipData;
class Terrain;
class Rain;

struct ContactPoint
{
  irr::core::vector3df position; // position of the point on the ship's hull/outer surface
  irr::core::vector3df normal;
  irr::core::vector3df internalPosition; // Position within the ship, for use as a starting point for ray intersection checks
  irr::f32 torqueEffect;                 // From cross product, how much a unit force along the contact vector gives a torque around the vertical axis
  irr::f32 effectiveArea;                // Contact area represented (in m2)
};

class OwnShip : public Ship
{
public:
  void load(OwnShipData ownShipData, irr::core::vector3di numberOfContactPoints, irr::f32 minContactPointSpacing, irr::f32 contactStiffnessFactor, irr::f32 contactDampingFactor, irr::f32 frictionCoefficient, irr::f32 tanhFrictionFactor, irr::scene::ISceneManager *smgr, SimulationModel *model, Terrain *terrain, irr::IrrlichtDevice *dev);
  void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::f32 weather, irr::core::vector3df linesForce, irr::core::vector3df linesTorque);
  std::vector<irr::core::vector3df> getCameraViews() const;
  std::vector<bool> getCameraIsHighView() const;
  void setViewVisibility(irr::u32 view);
  std::string getRadarConfigFile() const;
  irr::f32 getDepth() const;
  irr::f32 getAngleCorrection() const;
  bool hasGPS() const;
  bool hasDepthSounder() const;
  bool hasBowThruster() const;
  bool hasSternThruster() const;
  bool hasTurnIndicator() const;
  irr::f32 getMaxSounderDepth() const;
  void setRudder(irr::f32);                 // Set the rudder (-ve is port, +ve is stbd). Clamps to +-30
  void setWheel(irr::f32, bool force);      // Set the wheel (-ve is port, +ve is stbd). Clamps to +-30 DEE. Set force to true to apply even if follow up rudder has failed

  void setPortEngine(irr::f32);                                 // Set the engine, (-ve astern, +ve ahead), range is +-1. This method limits the range applied
  void setStbdEngine(irr::f32);                                 // Set the engine, (-ve astern, +ve ahead), range is +-1. This method limits the range applied
  void setBowThruster(irr::f32 proportion);                     // Set the bow thruster, (-ve port, +ve stbd), range is +-1. This method limits the range applied
  void setSternThruster(irr::f32 proportion);                   // Set the bow thruster, (-ve port, +ve stbd), range is +-1. This method limits the range applied
  void setRateOfTurn(irr::f32 rateOfTurn);                      // Sets the rate of turn (used when controlled as secondary)
  void setBowThrusterRate(irr::f32 bowThrusterRate);            // Sets the rate of increase of bow thruster, used for joystick button control
  void setSternThrusterRate(irr::f32 sternThrusterRate);        // Sets the rate of increase of stern thruster, used for joystick button control
  void setRudderPumpState(int whichPump, bool rudderPumpState); // Sets how the rudder is responding. Assumes that whichPump can be 1 or 2
  bool getRudderPumpState(int whichPump) const;
  void setFollowUpRudderWorking(bool followUpRudderWorking); // Sets if the normal (follow up) rudder is working
  bool getFollowUpRudderWorking();                           //  DEE_NOV22 true if in follow up mode false in emergency steering non follow up mode
  irr::f32 getRateOfTurn() const;
  irr::f32 getPortEngine() const; //-1 to 1
  irr::f32 getStbdEngine() const; //-1 to 1
  irr::f32 getBowThruster() const;
  irr::f32 getSternThruster() const;
  irr::f32 getPortEngineRPM() const;
  irr::f32 getStbdEngineRPM() const;
  irr::f32 getRudder() const;           //-30 to 30
  irr::f32 getWheel() const;            // DEE -30 to +30
  irr::f32 getPitch() const;
  irr::f32 getRoll() const;
  irr::f32 getCOG() const;
  irr::f32 getSOG() const; // m/s
  irr::f32 getSpeedThroughWater() const; // m/s
  irr::f32 getLateralSpeed() const;
  std::string getBasePath() const;
  irr::core::vector3df getScreenDisplayPosition() const;
  irr::f32 getScreenDisplaySize() const;
  irr::f32 getScreenDisplayTilt() const;
  irr::core::vector3df getPortEngineControlPosition() const;
  irr::core::vector3df getStbdEngineControlPosition() const;
  irr::core::vector3df getWheelControlPosition() const;
  irr::f32 getWheelControlScale() const;
  bool isSingleEngine() const;

  irr::f32 getLastDeltaTime();                      // gets the delta time for the last cycle
  void setLastDeltaTime(irr::f32 myDeltaTime);      // sets the delta time for the last cycle

  bool isBuoyCollision() const;
  bool isOtherShipCollision() const;
  irr::f32 getShipMass() const;
  irr::f32 getScaleFactor() const;

  void enableTriangleSelector(bool selectorEnabled);

protected:
private:
  void collisionDetectAndRespond(irr::f32 &reaction, irr::f32 &lateralReaction, irr::f32 &turnReaction);
  irr::f32 requiredEngineProportion(irr::f32 speed);
  irr::f32 sign(irr::f32 inValue) const;
  irr::f32 sign(irr::f32 inValue, irr::f32 threshold) const;
  void addContactPointFromRay(irr::core::line3d<irr::f32> ray, irr::f32 contactArea);

  irr::IrrlichtDevice *device;
  std::vector<irr::core::vector3df> views; // The offset of the camera origin from the own ship origin
  std::vector<bool> isHighView;            // Should be the same size as views (todo: Make this into a struct with views)
  std::string radarConfigFile;
  std::string basePath; // The location the model is loaded from
  Terrain *terrain;
  Rain *rain;
  SimulationModel *model;
  bool is360textureShip;
  bool showDebugData;
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

  irr::f32 deltaTime;
  // DEE_DEC22 vvvv
  irr::f32 Izz;                             // inertia about temporal axis
  irr::f32 Ixx;                             // inertia about lateral axis
  irr::f32 Iyy;                             // inertia about longitudinal axis
  irr::f32 maxSpeed;                        // vessels max speed ahead as read from ini file new parameter
  irr::f32 maxSpeed_mps;                    // vessels max speed ahead as read from ini file new parameter
  irr::f32 cB;                              // Block coefficient

  irr::f32 rudderMinAngle;
  irr::f32 rudderMaxAngle;
  bool singleEngine;
  bool bowThrusterPresent;
  bool sternThrusterPresent;
  bool turnIndicatorPresent;
  irr::f32 bowThruster;     //-1 to +1
  irr::f32 sternThruster;   //-1 to +1
  irr::f32 bowThrusterRate; // Rate of change, for joystick button control
  irr::f32 sternThrusterRate;
  irr::f32 wheel;             //-30 to + 30
  irr::f32 rudderMaxSpeed;    // the angular velocity in degrees per minute that the steering gear can achieve to turn the rudder
  // DEE_NOV22 comment ^^^^ on reflection perhaps this would have been better as degrees per second
  bool rudderPump1Working;    // Does rudder pump 1 (of 2) work
  bool rudderPump2Working;    // Does rudder pump 2 (of 2) work
  bool followUpRudderWorking; // Does the normal rudder (follow up mode) work
  irr::f32 rudder;            //-30 to + 30

  // Dynamics parameters
  irr::f32 shipMass;
  irr::f32 inertia;
  irr::f32 maxEngineRevs;
  irr::f32 dynamicsSpeedA;
  irr::f32 dynamicsSpeedB;
  irr::f32 dynamicsTurnDragA;
  irr::f32 dynamicsTurnDragB;
  irr::f32 dynamicsLateralDragA;
  irr::f32 dynamicsLateralDragB;
  irr::f32 rudderA;
  irr::f32 rudderB;
  irr::f32 rudderBAstern;
  irr::f32 maxForce;
  irr::f32 bowThrusterMaxForce;
  irr::f32 sternThrusterMaxForce;
  irr::f32 bowThrusterDistance;
  irr::f32 sternThrusterDistance;
  irr::f32 propellorSpacing;
  irr::f32 asternEfficiency;
  irr::f32 propWalkAhead;
  irr::f32 propWalkAstern;
  Sail mSails;

  // Dynamics variables
  // irr::f32 portThrust;
  // irr::f32 stbdThrust;
  // irr::f32 drag;
  // irr::f32 acceleration;
  irr::f32 maxSpeedAhead;
  irr::f32 maxSpeedAstern;
  irr::f32 rateOfTurn;
  irr::f32 dragTorque;
  irr::f32 rudderTorque;
  irr::f32 engineTorque;
  irr::f32 propWalkTorque;
  irr::f32 axialSpd; // DEE_DEC22
  irr::f32 lateralSpd;
  irr::f32 speedThroughWater;

  // DEE_DEC22 vvvv
  irr::f32 seawaterDensity; // 1024 kg/m^3 however can be less in dockwater and freshwater
  // DEE_DEC22 ^^^^

  irr::f32 cog; // course over ground
  irr::f32 sog; // m/s speed over ground

  irr::f32 waveHeightFiltered; // 1st order transfer filtered response to waves
  // General settings
  bool gps;
  bool depthSounder;
  irr::f32 maxSounderDepth;

  irr::core::vector3df screenDisplayPosition;
  irr::f32 screenDisplaySize;
  irr::f32 screenDisplayTilt;

  irr::core::vector3df portThrottlePosition;
  irr::core::vector3df stbdThrottlePosition;
  irr::core::vector3df wheelControlPosition;
  irr::f32 wheelControlScale;

  bool buoyCollision;
  bool otherShipCollision;

  std::vector<ContactPoint> contactPoints;

  irr::f32 contactStiffnessFactor;
  irr::f32 contactDampingFactor;
  irr::f32 frictionCoefficient;
  irr::f32 tanhFrictionFactor;

  irr::scene::ITriangleSelector *selector;
  bool triangleSelectorEnabled; // Keep track of this so we don't keep re-setting the selector

  // Debugging
  std::vector<irr::scene::IMeshSceneNode *> contactDebugPoints;
};

#endif // __OWNSHIP_HPP_INCLUDED__
