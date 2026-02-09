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

#include <memory>
#include <vector>

#include "graphics/Types.hpp"
#include "Ship.hpp"
#include "MMGPhysicsModel.hpp"

// Forward declarations
class SimulationModel;
class OwnShipData;
class Terrain;

namespace irr {
    class IrrlichtDevice;
    namespace scene {
        class ISceneManager;
        class ITriangleSelector;
        class IMeshSceneNode;
    }
}

struct ContactPoint
{
        bc::graphics::Vec3 position; // position of the point on the ship's hull/outer surface
        bc::graphics::Vec3 normal;
        bc::graphics::Vec3 internalPosition; // Position within the ship, for use as a starting point for ray intersection checks
        float torqueEffect;                 // From cross product, how much a unit force along the contact vector gives a torque around the vertical axis
        float effectiveArea;                // Contact area represented (in m2)
};

class OwnShip : public Ship
{
public:
        void load(OwnShipData ownShipData, bc::graphics::Vec3i numberOfContactPoints, float minContactPointSpacing, float contactStiffnessFactor, float contactDampingFactor, float frictionCoefficient, float tanhFrictionFactor, irr::scene::ISceneManager *smgr, SimulationModel *model, Terrain *terrain, irr::IrrlichtDevice *dev);
        void update(float deltaTime, float scenarioTime, float tideHeight, float weather, bc::graphics::Vec3 linesForce, bc::graphics::Vec3 linesTorque);
        std::vector<bc::graphics::Vec3> getCameraViews() const;
        std::vector<bool> getCameraIsHighView() const;
        void setViewVisibility(uint32_t view);
        std::string getRadarConfigFile() const;
        float getDepth() const;
        float getAngleCorrection() const;
        bool hasGPS() const;
        bool hasDepthSounder() const;
        bool hasBowThruster() const;
        bool hasSternThruster() const;
        bool hasTurnIndicator() const;
        float getMaxSounderDepth() const;
        void setRudder(float);                 // Set the rudder (-ve is port, +ve is stbd). Clamps to +-30
        void setWheel(float, bool force);      // Set the wheel (-ve is port, +ve is stbd). Clamps to +-30 DEE. Set force to true to apply even if follow up rudder has failed
        void setPortAzimuthAngle(float angle); // Set the azimuth angle, in degrees (-ve is port, +ve is stbd)
        void setStbdAzimuthAngle(float angle); // Set the azimuth angle, in degrees (-ve is port, +ve is stbd)
        // DEE_NOV22 vvvv
        void setCommandedStbdAzimuthAngle(float commandedStbdAzimuthAngle);
        float getCommandedStbdAzimuthAngle();

        // DEE_NOV22 ^^^^
        void setAzimuth1Master(bool isMaster); // Set if azimuth 1 should also control azimuth 2
        void setAzimuth2Master(bool isMaster); // Set if azimuth 2 should also control azimuth 1
        bool getAzimuth1Master() const;
        bool getAzimuth2Master() const;
        void setPortEngine(float);                                 // Set the engine, (-ve astern, +ve ahead), range is +-1. This method limits the range applied
        void setStbdEngine(float);                                 // Set the engine, (-ve astern, +ve ahead), range is +-1. This method limits the range applied
        void setBowThruster(float proportion);                     // Set the bow thruster, (-ve port, +ve stbd), range is +-1. This method limits the range applied
        void setSternThruster(float proportion);                   // Set the bow thruster, (-ve port, +ve stbd), range is +-1. This method limits the range applied
        void setRateOfTurn(float rateOfTurn);                      // Sets the rate of turn (used when controlled as secondary)
        void setBowThrusterRate(float bowThrusterRate);            // Sets the rate of increase of bow thruster, used for joystick button control
        void setSternThrusterRate(float sternThrusterRate);        // Sets the rate of increase of stern thruster, used for joystick button control
        void setRudderPumpState(int whichPump, bool rudderPumpState); // Sets how the rudder is responding. Assumes that whichPump can be 1 or 2
        bool getRudderPumpState(int whichPump) const;
        void setFollowUpRudderWorking(bool followUpRudderWorking); // Sets if the normal (follow up) rudder is working
        bool getFollowUpRudderWorking();                           //  DEE_NOV22 true if in follow up mode false in emergency steering non follow up mode
        float getRateOfTurn() const;
        float getPortEngine() const; //-1 to 1
        float getStbdEngine() const; //-1 to 1
        float getBowThruster() const;
        float getSternThruster() const;
        float getPortEngineRPM() const;
        float getStbdEngineRPM() const;
        float getRudder() const;           //-30 to 30
        float getWheel() const;            // DEE -30 to +30
        float getPortAzimuthAngle() const; // degrees
        float getStbdAzimuthAngle() const; // degrees
        float getPitch() const;
        float getRoll() const;
        float getCOG() const;
        float getSOG() const; // m/s
        float getSpeedThroughWater() const; // m/s
        std::string getBasePath() const;
        bc::graphics::Vec3 getScreenDisplayPosition() const;
        float getScreenDisplaySize() const;
        float getScreenDisplayTilt() const;
        bc::graphics::Vec3 getPortEngineControlPosition() const;
        bc::graphics::Vec3 getStbdEngineControlPosition() const;
        bc::graphics::Vec3 getWheelControlPosition() const;
        float getWheelControlScale() const;
        bool isSingleEngine() const;
        bool isAzimuthDrive() const;
        bool isAzimuthAsternAllowed() const;
        bool isAzimuth1Master() const;
        bool isAzimuth2Master() const;

        // DEE_NOV22 vvvv
        // Azimuth Drive code added
        void setPortSchottel(float portAngle); // sets port      azimuth drive angle -ve anticlockwise +ve clockwise
        void setStbdSchottel(float stbdAngle); // sets starboard azimuth drive angle -ve anticlockwise +ve clockwise
        float getPortSchottel() const;         // gets port	  azimuth drive angle -ve anticlockwise +ve clockwise
        float getStbdSchottel() const;         // gets starboard azimuth drive angle -ve anticlockwise +ve clockwise
        void setPortClutch(bool portClutch);      // sets port	  azimuth drive clutch true is clutched in false is clutched out
        void setStbdClutch(bool stbdClutch);      // sets starboard azimuth drive clutch true is clutched in false is clutched out
        bool getPortClutch();                     // gets port clutch perhaps have synonym isPortClutched() todo
        bool getStbdClutch();                     // gets stbd clutch perhaps have synonym isPortClutched() todo
        void engagePortClutch();                  // engages the port clutch
        void disengagePortClutch();               // disengages the port clutch
        void engageStbdClutch();                  // engages the starboard clutch
        void disengageStbdClutch();               // disengages the starboard clutch

        void setPortAzimuthThrustLever(float thrustLever);    // sets port thrust lever 0..1 or -1..+1
        void setStbdAzimuthThrustLever(float thrustLever);    // sets stbd thrust lever 0..1 or -1..+1
        float getPortAzimuthThrustLever();                    // gets position of port thrust lever 0..1 or -1..+1
        float getStbdAzimuthThrustLever();                    // gets position of stbd thrust lever 0..1 or -1..+1
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
        float getLastDeltaTime();                      // gets the delta time for the last cycle
        void setLastDeltaTime(float myDeltaTime);      // sets the delta time for the last cycle
        void setCommandedPortAngle(float myDeltaTime); // sets the delta time for the last cycle

        // DEE_NOV22 ^^^^

        bool isBuoyCollision() const;
        bool isOtherShipCollision() const;
        float getShipMass() const;
        float getScaleFactor() const;

        void enableTriangleSelector(bool selectorEnabled);

protected:
private:
        void collisionDetectAndRespond(float &reaction, float &lateralReaction, float &turnReaction);
        float requiredEngineProportion(float speed);
        float sign(float inValue) const;
        float sign(float inValue, float threshold) const;
        void addContactPointFromRay(bc::graphics::Line3d ray, float contactArea);

        irr::IrrlichtDevice *device;
        std::vector<bc::graphics::Vec3> views; // The offset of the camera origin from the own ship origin
        std::vector<bool> isHighView;            // Should be the same size as views (todo: Make this into a struct with views)
        std::string radarConfigFile;
        std::string basePath; // The location the model is loaded from
        Terrain *terrain;
        SimulationModel *model;
        bool is360textureShip;
        bool showDebugData;
        float scaleFactor;
        float rollPeriod;       // Roll period (s)  DEE this should be dynamically loaded
        float rollAngle;        // Roll Angle (deg)
        float pitchPeriod;      // Roll period (s)
        float pitchAngle;       // Roll Angle (deg)
        float buffetPeriod;     // Yaw period (s)
        float buffet;           // How much ship is buffeted by waves (undefined units)
        float pitch;            //(deg)
        float roll;             //(deg)
        float portEngine;       //-1 to + 1
        float stbdEngine;       //-1 to + 1
        float portAzimuthAngle; // in degrees
        float stbdAzimuthAngle; // in degrees
        bool azimuthDrive;
        bool azimuth1Master;            // If azimuth control 1 should also control azimuth control 2
        bool azimuth2Master;            // If azimuth control 2 should also control azimuth control 1
        bool azimuthAsternAllowed;      // If azimuth drives can run in astern as well as ahead (e.g. for outboard motors)
        float azimuthPositionAstern; // How far azimuth drives are astern

        float deltaTime; // stores the last delta time which is good enough for the movement of levers and schottels etc

        // DEE_NOV22 vvvv
        // more Azimuth Drive Code

        // ini file variables

        float azimuthDriveEngineIdleRPM;       // Idling rpm of each engine
        float azimuthDriveClutchEngageRPM;     // for each engine, the rpm which when exceeded the clutch is automatically engaged
        float azimuthDriveClutchDisengageRPM;  // for each engine, the rpm which when reduced to below, the clutch is automatically disengaged
        float schottelMaxDegPerSecond;         // only really relevant to keyboard control to make it playable
                                                  // the shchottel itself can be turned as fast as your wrist can move it
                                                  // so if using physical controls then its not needed
        float azimuthDriveMaxDegPerSecond;     // the maximum number of degrees per second that the azimuth drive
                                                  // can change direction
        float thrustLeverMaxChangePerSecond;   // only really relevant to keyboard control to make it playable
        float engineMaxChangePerSecond;        // how much the engine can change per second todo rpm or 0..1 this is relevant to both keyboard
                                                  // and physical controls so there needs to be a commanded engine  (0..1) variable for each
                                                  // azimuth drive
        bool azimuthDriveSameDirectionAsSchottel; // true if schottel and azidrive turn in the same direction (not always the case eg Shetland Trader)
                                                  // DEE_NOV22 ^^^^
                                                  // DEE_DEC22 vvvv
        float Izz;                             // inertia about temporal axis
        float Ixx;                             // inertia about lateral axis
        float Iyy;                             // inertia about longitudinal axis
        float maxSpeed;                        // vessels max speed ahead as read from ini file new parameter
        float maxSpeed_mps;                    // vessels max speed ahead as read from ini file new parameter
        float cB;                              // Block coefficient
        float aziToLengthRatio;                // 0..1 from stern to bow axial (y axis) location of azidrive
        float aziDriveLateralLeverArm;         // how far astern of amidships are the azi drives , -ve for ahead of midships
                                                  // DEE_NOV22 ^^^^

        float rudderMinAngle;
        float rudderMaxAngle;
        bool singleEngine;
        bool bowThrusterPresent;
        bool sternThrusterPresent;
        bool turnIndicatorPresent;
        float bowThruster;     //-1 to +1
        float sternThruster;   //-1 to +1
        float bowThrusterRate; // Rate of change, for joystick button control
        float sternThrusterRate;
        float wheel;             //-30 to + 30
        float rudderMaxSpeed;    // the angular velocity in degrees per minute that the steering gear can achieve to turn the rudder
                                    // DEE_NOV22 comment ^^^^ on reflection perhaps this would have been better as degrees per second
        bool rudderPump1Working;    // Does rudder pump 1 (of 2) work
        bool rudderPump2Working;    // Does rudder pump 2 (of 2) work
        bool followUpRudderWorking; // Does the normal rudder (follow up mode) work
        float rudder;            //-30 to + 30

        // Dynamics parameters

        // DEE_NOV22 vvvv
        float portLateralThrust;          // Athwartships thrust attributable to port azimuth drive
        float stbdLateralThrust;          // Athwartships thrust attributable to stbd azimuth drive
        float portAzimuthThrustLever;     // Normally 0 to +1, can be -1 to +1 for things like outboards
        float stbdAzimuthThrustLever;     // as above but for starboard
        float portSchottel;               // angle of the port Schottel, so basically the commanded azimuth port
        float stbdSchottel;               // angle of the stbd Schottel,  ditto for stabd
        bool portClutch;                     // port clutch true for clutch engaged false for clutch disengaged
        bool stbdClutch;                     // stbd clutch ditto for stbd
        float maxChangeInEngineThisCycle; // the calculated maximum the engine level can change by in one cycle
        float idleEngine;                 // DEE TODO check this
        float newPortEngine;              // (0..1) the new port engine level calculated by update
        float newStbdEngine;              // (0..1) the new stbd engine level calculated by update
        float newPortSchottel;            // (0..360) if outside that range then adjust by 360 degrees
        float newStbdSchottel;            // (0..360) if outside that range then adjust by 360 degrees

        float commandedPortAngle;                    // whereas this is the same as the schottel angle, some vessels have left and right swapped
        float commandedStbdAngle;                    // with the aim of making steering on passage easier for those who can't cope with the
                                                        // concept of tiller steering.
        float maxChangeInAzimuthDriveAngleThisCycle; // calculated maximum angular change of an azimuth drive in one cycle
        float newPortAzimuthDriveAngle;              // in hindsight dont really need one for port and starboard
        float newStbdAzimuthDriveAngle;              // ditto

        // DEE_NOV22 ^^^^

        float shipMass;
        float inertia;
        float maxEngineRevs;
        float dynamicsSpeedA;
        float dynamicsSpeedB;
        float dynamicsTurnDragA;
        float dynamicsTurnDragB;
        float dynamicsLateralDragA;
        float dynamicsLateralDragB;
        float rudderA;
        float rudderB;
        float rudderBAstern;
        float maxForce;
        float bowThrusterMaxForce;
        float sternThrusterMaxForce;
        float bowThrusterDistance;
        float sternThrusterDistance;
        float propellorSpacing;
        float asternEfficiency;
        float propWalkAhead;
        float propWalkAstern;
        // Dynamics variables
        // float portThrust;
        // float stbdThrust;
        // float drag;
        // float acceleration;
        float maxSpeedAhead;
        float maxSpeedAstern;
        float rateOfTurn;
        float dragTorque;
        float rudderTorque;
        float engineTorque;
        float propWalkTorque;
        float lateralSpd;
        float speedThroughWater;

        // DEE_DEC22 vvvv
        float seawaterDensity; // 1024 kg/m^3 however can be less in dockwater and freshwater
        // DEE_DEC22 ^^^^

        float cog; // course over ground
        float sog; // m/s speed over ground

        float waveHeightFiltered; // 1st order transfer filtered response to waves
        // General settings
        bool gps;
        bool depthSounder;
        float maxSounderDepth;

        bc::graphics::Vec3 screenDisplayPosition;
        float screenDisplaySize;
        float screenDisplayTilt;

        bc::graphics::Vec3 portThrottlePosition;
        bc::graphics::Vec3 stbdThrottlePosition;
        bc::graphics::Vec3 wheelControlPosition;
        float wheelControlScale;

        // MMG physics model (optional, enabled by MMGMode=1 in boat.ini)
        bool useMMG;
        std::unique_ptr<MMGPhysicsModel> mmgModel;
        float physicsAccumulator;

        bool buoyCollision;
        bool otherShipCollision;

        std::vector<ContactPoint> contactPoints;

        float contactStiffnessFactor;
        float contactDampingFactor;
        float frictionCoefficient;
        float tanhFrictionFactor;

        irr::scene::ITriangleSelector *selector; // Irrlicht collision detection
        bool triangleSelectorEnabled; // Keep track of this so we don't keep re-setting the selector

        // Debugging
        std::vector<irr::scene::IMeshSceneNode *> contactDebugPoints;
};

#endif // __OWNSHIP_HPP_INCLUDED__
