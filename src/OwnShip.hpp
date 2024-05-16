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

#include "Ship.hpp"

// Forward declarations
class SimulationModel;
class OwnShipData;
class Terrain;

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
        void setPortAzimuthAngle(irr::f32 angle); // Set the azimuth angle, in degrees (-ve is port, +ve is stbd)
        void setStbdAzimuthAngle(irr::f32 angle); // Set the azimuth angle, in degrees (-ve is port, +ve is stbd)
        // DEE_NOV22 vvvv
        void setCommandedStbdAzimuthAngle(irr::f32 commandedStbdAzimuthAngle);
        irr::f32 getCommandedStbdAzimuthAngle();

        // DEE_NOV22 ^^^^
        void setAzimuth1Master(bool isMaster); // Set if azimuth 1 should also control azimuth 2
        void setAzimuth2Master(bool isMaster); // Set if azimuth 2 should also control azimuth 1
        bool getAzimuth1Master() const;
        bool getAzimuth2Master() const;
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
        irr::f32 getPortAzimuthAngle() const; // degrees
        irr::f32 getStbdAzimuthAngle() const; // degrees
        irr::f32 getPitch() const;
        irr::f32 getRoll() const;
        irr::f32 getCOG() const;
        irr::f32 getSOG() const; // m/s
        std::string getBasePath() const;
        irr::core::vector3df getScreenDisplayPosition() const;
        irr::f32 getScreenDisplaySize() const;
        irr::f32 getScreenDisplayTilt() const;
        irr::core::vector3df getPortEngineControlPosition() const;
        irr::core::vector3df getStbdEngineControlPosition() const;
        irr::core::vector3df getWheelControlPosition() const;
        bool isSingleEngine() const;
        bool isAzimuthDrive() const;
        bool isAzimuthAsternAllowed() const;
        bool isAzimuth1Master() const;
        bool isAzimuth2Master() const;

        // DEE_NOV22 vvvv
        // Azimuth Drive code added
        void setPortSchottel(irr::f32 portAngle); // sets port      azimuth drive angle -ve anticlockwise +ve clockwise
        void setStbdSchottel(irr::f32 stbdAngle); // sets starboard azimuth drive angle -ve anticlockwise +ve clockwise
        irr::f32 getPortSchottel() const;         // gets port	  azimuth drive angle -ve anticlockwise +ve clockwise
        irr::f32 getStbdSchottel() const;         // gets starboard azimuth drive angle -ve anticlockwise +ve clockwise
        void setPortClutch(bool portClutch);      // sets port	  azimuth drive clutch true is clutched in false is clutched out
        void setStbdClutch(bool stbdClutch);      // sets starboard azimuth drive clutch true is clutched in false is clutched out
        bool getPortClutch();                     // gets port clutch perhaps have synonym isPortClutched() todo
        bool getStbdClutch();                     // gets stbd clutch perhaps have synonym isPortClutched() todo
        void engagePortClutch();                  // engages the port clutch
        void disengagePortClutch();               // disengages the port clutch
        void engageStbdClutch();                  // engages the starboard clutch
        void disengageStbdClutch();               // disengages the starboard clutch

        void setPortAzimuthThrustLever(irr::f32 thrustLever);    // sets port thrust lever 0..1 or -1..+1
        void setStbdAzimuthThrustLever(irr::f32 thrustLever);    // sets stbd thrust lever 0..1 or -1..+1
        irr::f32 getPortAzimuthThrustLever();                    // gets position of port thrust lever 0..1 or -1..+1
        irr::f32 getStbdAzimuthThrustLever();                    // gets position of stbd thrust lever 0..1 or -1..+1
        void btnIncrementPortSchottel();                  // turn port schottel clockwise in response to a key press D
        void btnDecrementPortSchottel();                  // turn port schottel anticlockwise in response to a key press A
        void btnIncrementStbdSchottel();                  // turn stbd schottel clockwise in response to a key press L
        void btnDecrementStbdSchottel();                  // turn stbd schottel anticlockwise in response to a key press J
        void btnEngagePortClutch();                       // Engage port clutch in response to a key press    TODO youd use this in non follow up
        void btnDisengagePortClutch();                    // Disengage port clutch in response to a key press TODO ditto
        void btnEngageStbdClutch();                       // Engage stbd clutch in response to a key press    TODO ditto
        void btnDisengageStbdClutch();                    // Disengage stbd clutch in response to a key press TODO ditto
        void btnIncrementPortThrustLever();               // Increment port thrust lever
        void btnDecrementPortThrustLever();               // Decrement port thrust lever
        void btnIncrementStbdThrustLever();               // Increment stbd thrust lever
        void btnDecrementStbdThrustLever();               // Decrement stbd thrust lever
        bool isConventionalAzidriveSchottel() const;      // True if azimuth stern drive turns in the same direction as the schottel
        void followupPortAzimuthDrive();                  // Follow up angle of port azimuth drive in response to port schottel control
        void followupStbdAzimuthDrive();                  // Follow up angle of stbd azimuth drive in response to stbd schottel control
        irr::f32 getLastDeltaTime();                      // gets the delta time for the last cycle
        void setLastDeltaTime(irr::f32 myDeltaTime);      // sets the delta time for the last cycle
        void setCommandedPortAngle(irr::f32 myDeltaTime); // sets the delta time for the last cycle

        // DEE_NOV22 ^^^^

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
        irr::f32 portAzimuthAngle; // in degrees
        irr::f32 stbdAzimuthAngle; // in degrees
        bool azimuthDrive;
        bool azimuth1Master;            // If azimuth control 1 should also control azimuth control 2
        bool azimuth2Master;            // If azimuth control 2 should also control azimuth control 1
        bool azimuthAsternAllowed;      // If azimuth drives can run in astern as well as ahead (e.g. for outboard motors)
        irr::f32 azimuthPositionAstern; // How far azimuth drives are astern

        irr::f32 deltaTime; // stores the last delta time which is good enough for the movement of levers and schottels etc

        // DEE_NOV22 vvvv
        // more Azimuth Drive Code

        // ini file variables

        irr::f32 azimuthDriveEngineIdleRPM;       // Idling rpm of each engine
        irr::f32 azimuthDriveClutchEngageRPM;     // for each engine, the rpm which when exceeded the clutch is automatically engaged
        irr::f32 azimuthDriveClutchDisengageRPM;  // for each engine, the rpm which when reduced to below, the clutch is automatically disengaged
        irr::f32 schottelMaxDegPerSecond;         // only really relevant to keyboard control to make it playable
                                                  // the shchottel itself can be turned as fast as your wrist can move it
                                                  // so if using physical controls then its not needed
        irr::f32 azimuthDriveMaxDegPerSecond;     // the maximum number of degrees per second that the azimuth drive
                                                  // can change direction
        irr::f32 thrustLeverMaxChangePerSecond;   // only really relevant to keyboard control to make it playable
        irr::f32 engineMaxChangePerSecond;        // how much the engine can change per second todo rpm or 0..1 this is relevant to both keyboard
                                                  // and physical controls so there needs to be a commanded engine  (0..1) variable for each
                                                  // azimuth drive
        bool azimuthDriveSameDirectionAsSchottel; // true if schottel and azidrive turn in the same direction (not always the case eg Shetland Trader)
                                                  // DEE_NOV22 ^^^^
                                                  // DEE_DEC22 vvvv
        irr::f32 Izz;                             // inertia about temporal axis
        irr::f32 Ixx;                             // inertia about lateral axis
        irr::f32 Iyy;                             // inertia about longitudinal axis
        irr::f32 maxSpeed;                        // vessels max speed ahead as read from ini file new parameter
        irr::f32 maxSpeed_mps;                    // vessels max speed ahead as read from ini file new parameter
        irr::f32 cB;                              // Block coefficient
        irr::f32 aziToLengthRatio;                // 0..1 from stern to bow axial (y axis) location of azidrive
        irr::f32 aziDriveLateralLeverArm;         // how far astern of amidships are the azi drives , -ve for ahead of midships
                                                  // DEE_NOV22 ^^^^

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

        // DEE_NOV22 vvvv
        irr::f32 portLateralThrust;          // Athwartships thrust attributable to port azimuth drive
        irr::f32 stbdLateralThrust;          // Athwartships thrust attributable to stbd azimuth drive
        irr::f32 portAzimuthThrustLever;     // Normally 0 to +1, can be -1 to +1 for things like outboards
        irr::f32 stbdAzimuthThrustLever;     // as above but for starboard
        irr::f32 portSchottel;               // angle of the port Schottel, so basically the commanded azimuth port
        irr::f32 stbdSchottel;               // angle of the stbd Schottel,  ditto for stabd
        bool portClutch;                     // port clutch true for clutch engaged false for clutch disengaged
        bool stbdClutch;                     // stbd clutch ditto for stbd
        irr::f32 maxChangeInEngineThisCycle; // the calculated maximum the engine level can change by in one cycle
        irr::f32 idleEngine;                 // DEE TODO check this
        irr::f32 newPortEngine;              // (0..1) the new port engine level calculated by update
        irr::f32 newStbdEngine;              // (0..1) the new stbd engine level calculated by update
        irr::f32 newPortSchottel;            // (0..360) if outside that range then adjust by 360 degrees
        irr::f32 newStbdSchottel;            // (0..360) if outside that range then adjust by 360 degrees

        irr::f32 commandedPortAngle;                    // whereas this is the same as the schottel angle, some vessels have left and right swapped
        irr::f32 commandedStbdAngle;                    // with the aim of making steering on passage easier for those who can't cope with the
                                                        // concept of tiller steering.
        irr::f32 maxChangeInAzimuthDriveAngleThisCycle; // calculated maximum angular change of an azimuth drive in one cycle
        irr::f32 newPortAzimuthDriveAngle;              // in hindsight dont really need one for port and starboard
        irr::f32 newStbdAzimuthDriveAngle;              // ditto

        // DEE_NOV22 ^^^^

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
