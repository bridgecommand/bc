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

#ifndef __SIMULATIONMODEL_HPP_INCLUDED__
#define __SIMULATIONMODEL_HPP_INCLUDED__

#include <iostream> //For debugging
#include <string>
#include <vector>
#include <stdint.h> //for uint64_t

#include "irrlicht.h"

//Forward declarations
class ScenarioData;
class GUIMain;
class GUIData;
class ISound;

#include "Terrain.hpp"
#include "Light.hpp"
#include "Water.hpp"
#include "Rain.hpp"
#include "Tide.hpp"
#include "Buoys.hpp"
#include "OtherShips.hpp"
#include "LandObjects.hpp"
#include "LandLights.hpp"
#include "OwnShip.hpp"
#include "ManOverboard.hpp"
#include "Camera.hpp"
#include "RadarCalculation.hpp"
#include "RadarScreen.hpp"
#include "ControlVisualiser.hpp"
#include "Lines.hpp"
#include "OperatingModeEnum.hpp"

class SimulationModel //Start of the 'Model' part of MVC
{

public:

    struct ModelParameters{
        OperatingMode::Mode mode;
        bool vrMode;
        float viewAngle;
        float lookAngle;
        float cameraMinDistance;
        float cameraMaxDistance;
        uint32_t disableShaders;
        uint32_t waterSegments;
        irr::core::vector3di numberOfContactPoints;
        float minContactPointSpacing;
        float contactStiffnessFactor;
        float contactDampingFactor;
        float lineStiffnessFactor;
        float lineDampingFactor;
        float frictionCoefficient;
        float tanhFrictionFactor;
        uint32_t limitTerrainResolution;
        bool secondaryControlWheel;
        bool secondaryControlPortEngine;
        bool secondaryControlStbdEngine;
        bool secondaryControlPortSchottel;
        bool secondaryControlStbdSchottel;
        bool secondaryControlPortThrustLever;
        bool secondaryControlStbdThrustLever;
        bool secondaryControlBowThruster;
        bool secondaryControlSternThruster;
        bool debugMode;
    };
    
    SimulationModel(irr::IrrlichtDevice* dev,
                    irr::scene::ISceneManager* scene,
                    GUIMain* gui,
                    ISound* sound,
                    ScenarioData scenarioData,
                    ModelParameters modelParameters);
    ~SimulationModel();
    float longToX(float longitude) const;
    float latToZ(float latitude) const;
    void setSpeed(float spd); //Sets the own ship's speed
    void setHeading(float hdg); //Sets the own ship's heading

    float getRateOfTurn() const;
    void setRateofTurn(float rudder); //Set the rate of turn (-ve is port, +ve is stbd)



    void setRateOfTurn(float rateOfTurn);
    void setPos(float positionX, float positionZ);
    void setRudder(float rudder); //Set the rudder (-ve is port, +ve is stbd)
    void setWheel(float wheel, bool force=false); //Set the wheel (-ve is port, +ve is stbd) DEE. If force is true, the wheel change is applied even if the follow up rudder is failed
    float getRudder() const;
    float getWheel() const; // DEE
    void setAzimuth1Master(bool isMaster); // Set if azimuth 1 should also control azimuth 2
    void setAzimuth2Master(bool isMaster); // Set if azimuth 2 should also control azimuth 1
    bool getAzimuth1Master() const;
    bool getAzimuth2Master() const;
    void setPortAzimuthAngle(float angle); // Set the azimuth angle, in degrees (-ve is port, +ve is stbd)
    void setStbdAzimuthAngle(float angle); // Set the azimuth angle, in degrees (-ve is port, +ve is stbd)

    // DEE_NOV22 vvvv for follow up shcottel and automatic clutch
    void setPortSchottel(float angle); // Set Port Schottel angle
    float getPortSchottel();
    void setStbdSchottel(float angle); // Set Stbd Schottel angle
    float getStbdSchottel();
    bool getPortClutch();
    void setPortClutch(bool);
    bool getStbdClutch();
    void setStbdClutch(bool);
    void engagePortClutch();
    void disengagePortClutch();
    void engageStbdClutch();
    void disengageStbdClutch();
    void setPortAzimuthThrustLever(float);   // sets port thrust lever range is 0..+1 or -1..+1
    float getPortAzimuthThrustLever(); 	 // gets port thrust lever range is 0..+1 or -1..+1
    void setStbdAzimuthThrustLever(float);   // sets starboard thrust lever range is 0..+1 or -1..+1
    float getStbdAzimuthThrustLever(); // gets starboard thrust lever range is 0..+1 or -1..+1

    void btnIncrementPortThrustLever(); // increments the port thrust lever
    void btnDecrementPortThrustLever(); // decrements the port thrust lever
    void btnIncrementStbdThrustLever(); // increments the stbd thrust lever
    void btnDecrementStbdThrustLever(); // decrements the stbd thrust lever

    void btnIncrementPortSchottel(); // clockwise turn of the port schottel in response to a key press
    void btnDecrementPortSchottel(); // anticlockwise turn of the port schottel in response to a key press
    void btnIncrementStbdSchottel(); // clockwise turn of the starboard schottel in response to a key press
    void btnDecrementStbdSchottel(); // anticlockwise turn of the starboard schottel in response to a key press

    // DEE_NOV22 ^^^^

    void setPortEngine(float port); //Set the engine, (-ve astern, +ve ahead), range is +-1
    void setStbdEngine(float stbd); //Set the engine, (-ve astern, +ve ahead), range is +-1
    float getPortEngine() const; //Range +-1
    float getStbdEngine() const; //Range +-1
    float getPortEngineRPM() const;
    float getStbdEngineRPM() const;
    void setBowThruster(float proportion);
    void setSternThruster(float proportion);
    void setBowThrusterRate(float bowThrusterRate); //Sets rate of change, for joystick button control
    void setSternThrusterRate(float sternThrusterRate); //Sets rate of change, for joystick button control
    float getBowThruster() const;
    float getSternThruster() const;
    void setRudderPumpState(int whichPump, bool rudderPumpState); //Sets how the rudder is responding. Assumed that whichPump can be 1 or 2
    bool getRudderPumpState(int whichPump) const;
    void setFollowUpRudderWorking(bool followUpRudderWorking); //Sets if the normal (follow up) rudder is working
    void setAccelerator(float accelerator); //Set simulation time compression
    float getAccelerator() const;
    float getHeading() const; //Gets the own ship's heading

    float getLat() const;
    float getLong() const;
    float getPosX() const;
    float getPosZ() const;
    float getCOG() const;
    float getSOG() const; //In metres/second
    float getDepth() const;

    float getWaveHeight(float posX, float posZ) const; //Return wave height (not tide) at the world position specified
    bc::graphics::Vec2 getLocalNormals(float relPosX, float relPosZ) const;

    bc::graphics::Vec2 getTidalStream(float longitude, float latitude, uint64_t requestTime) const; //Tidal stream in m/s for the specified absolute position

    //void getTime(uint8_t& hour, uint8_t& min, uint8_t& sec) const;
    //void getDate(uint8_t& day, uint8_t& month, uint16_t& year) const;
    uint64_t getTimestamp() const; //The unix timestamp in s
    uint64_t getTimeOffset() const; //The timestamp at the start of the first day of the scenario
    float getTimeDelta() const; //The change in time (s) since the start of the start day of the scenario
    void     setTimeDelta(float scenarioTime);

    uint32_t getNumberOfOtherShips() const;
    uint32_t getNumberOfBuoys() const;
    std::string getOtherShipName(int number) const;
    float getOtherShipPosX(int number) const;
    float getOtherShipPosZ(int number) const;
    float getOtherShipLat(int number) const;
    float getOtherShipLong(int number) const;
    float getOtherShipHeading(int number) const;
    float getOtherShipSpeed(int number) const; //Speed in m/s
    uint32_t getOtherShipMMSI(int number) const;
    void setOtherShipHeading(int number, float hdg);
    void setOtherShipPos(int number, float positionX, float positionZ);
    void setOtherShipRateOfTurn(int number, float rateOfTurn);
    void setOtherShipSpeed(int number, float speed); //Speed in m/s
    void setOtherShipMMSI(int number, uint32_t mmsi);
    std::vector<Leg> getOtherShipLegs(int number) const;
    float getBuoyPosX(int number) const;
    float getBuoyPosZ(int number) const;
    void changeOtherShipLeg(int shipNumber, int legNumber, float bearing, float speed, float distance);
    void addOtherShipLeg(int shipNumber, int afterLegNumber, float bearing, float speed, float distance);
    void deleteOtherShipLeg(int shipNumber, int legNumber);
    void resetOtherShipLegs(int shipNumber, float course, float speedKts, float distanceNm);
	std::string getOwnShipEngineSound() const;
	std::string getOwnShipWaveSound() const;
	std::string getOwnShipHornSound() const;
    std::string getOwnShipAlarmSound() const;


    void setWeather(float weather); //Range 0-12.
    float getWeather() const;
    void setRain(float rainIntensity); //Range 0-10
    float getRain() const;
    void setVisibility(float visibilityNm);
    float getVisibility() const;
    void setWindDirection(float windDirection); //Range 0-360.
    float getWindDirection() const;
    void setWindSpeed(float windSpeed); //Nm/h
    float getWindSpeed() const;
    void setStreamOverrideDirection(float streamDirection); //Range 0-360.
    float getStreamOverrideDirection() const;
    void setStreamOverrideSpeed(float streamSpeed); //Nm/h
    float getStreamOverrideSpeed() const;
    void setStreamOverride(bool streamOverride);
    bool getStreamOverride() const;
    void setWaterVisible(bool visible);
    void lookUp();
    void lookDown();
    void lookLeft();
    void lookRight();
    void setPanSpeed(float horizontalPanSpeed);
    void setVerticalPanSpeed(float verticalPanSpeed);
    void changeLookPx(int32_t deltaX, int32_t deltaY);
    void lookStepLeft();
    void lookStepRight();
    void moveCameraForwards();
    void moveCameraBackwards();
    void lookAhead();
    void lookAstern();
    void lookPort();
    void lookStbd();
    void changeView();
    void setView(uint32_t view);
    uint32_t getCameraView() const;
    bc::graphics::Vec3 getCameraBasePosition() const;
    bc::graphics::Matrix4 getCameraBaseRotation() const;
    void setFrozenCamera(bool frozen);
    void toggleFrozenCamera();
	void setAlarm(bool alarmState);
    void toggleRadarOn();
    bool isRadarOn() const;
    irr::video::SColor getRadarSurroundColour() const;
	void increaseRadarRange();
    void decreaseRadarRange();
    void setRadarGain(float value);
    void setRadarClutter(float value);
    void setRadarRain(float value);
    void increaseRadarGain(float value);
    void decreaseRadarGain(float value);
    void increaseRadarClutter(float value);
    void decreaseRadarClutter(float value);
    void increaseRadarRain(float value);
    void decreaseRadarRain(float value);
    void setPIData(int32_t PIid, float PIbearing, float PIrange);
    float getPIbearing(int32_t PIid) const;
    float getPIrange(int32_t PIid) const;
    void increaseRadarEBLRange();
    void decreaseRadarEBLRange();
    void increaseRadarEBLBrg();
    void decreaseRadarEBLBrg();
    void increaseRadarXCursor();
    void decreaseRadarXCursor();
    void increaseRadarYCursor();
    void decreaseRadarYCursor();
    void setRadarNorthUp();
    void setRadarCourseUp();
    void setRadarHeadUp();
    void changeRadarColourChoice();
    int getArpaMode() const;
    void setArpaMode(int mode);
    void setArpaListSelection(int32_t selection);
    void setRadarARPARel();
    void setRadarARPATrue();
    void setRadarARPAVectors(float vectorMinutes);
    void setRadarDisplayRadius(uint32_t radiusPx);
    void addManualPoint(bool newContact);
    void clearManualPoints();
    void trackTargetFromCursor();
    void clearTargetFromCursor();
    uint32_t getARPATracksSize() const;
    ARPAContact getARPAContactFromTrackIndex(uint32_t index) const;
    void setMainCameraActive();
    void setRadarCameraActive();
    void updateViewport(float aspect);
    void setMouseDown(bool isMouseDown);
    void setZoom(bool zoomOn);
    void setZoom(bool zoomOn, float zoomLevel);
    void setViewAngle(float viewAngle);
    uint32_t getLoopNumber() const;
    std::string getSerialisedScenario() const;
    std::string getScenarioName() const;
    std::string getWorldName() const;
    std::string getWorldReadme() const;
    void releaseManOverboard();
    void retrieveManOverboard();
    bool getManOverboardVisible() const;
    float getManOverboardPosX() const;
    float getManOverboardPosZ() const;
    void setManOverboardVisible(bool visible); //To be used directly, eg when in secondary display mode only
    void setManOverboardPos(float positionX, float positionZ);   //To be used directly, eg when in secondary display mode only
    bool hasGPS() const;
    bool isSingleEngine() const;
    bool isAzimuthDrive() const;
    bool isAzimuthAsternAllowed() const;
    float inputToAzimuthEngineMapping(float inputAngle) const;
    float azimuthToInputEngineMapping(float inputEngine) const;
    bool hasDepthSounder() const;
    float getMaxSounderDepth() const;
    bool hasBowThruster() const;
    bool hasSternThruster() const;
    bool hasTurnIndicator() const;
    bool debugModeOn() const;
    float getOwnShipMass() const;
    float getOwnShipMassEstimate() const;
    float getOtherShipMassEstimate(int number) const;

    bool getMoveViewWithPrimary() const;
    void setMoveViewWithPrimary(bool moveView);

    ModelParameters getModelParameters() const;

    // TODO: Most of these can be replaced with getModelParameters()
    bool getIsSecondaryControlWheel() const;
    bool getIsSecondaryControlPortEngine() const;
    bool getIsSecondaryControlStbdEngine() const;
    bool getIsSecondaryControlPortSchottel() const;
    bool getIsSecondaryControlStbdSchottel() const;
    bool getIsSecondaryControlPortThrustLever() const;
    bool getIsSecondaryControlStbdThrustLever() const;
    bool getIsSecondaryControlBowThruster() const;
    bool getIsSecondaryControlSternThruster() const;

    float getLineStiffnessFactor() const;
    float getLineDampingFactor() const;

	void startHorn();
	void endHorn();

    irr::scene::ISceneNode* getContactFromRay(irr::core::line3d<float> ray, int32_t linesMode);
    
    irr::scene::ISceneNode* getOwnShipSceneNode();
    irr::scene::ISceneNode* getOtherShipSceneNode(int number);
    irr::scene::ISceneNode* getBuoySceneNode(int number);
    irr::scene::ISceneNode* getLandObjectSceneNode(int number);
    irr::scene::ISceneNode* getTerrainSceneNode(int number);

    Terrain* getTerrain();

    float getTerrainHeight(float posX, float posZ) const;

    void addLine(); // Add a line, which will be undefined
    
    Lines* getLines(); // Get pointer to lines object

    void updateCameraVRPos(bc::graphics::Quaternion quat, bc::graphics::Vec3 pos, bc::graphics::Vec2 lensShift);

    void update();
  
private:
    irr::IrrlichtDevice* device;
    irr::video::IVideoDriver* driver;
    irr::scene::ISceneManager* smgr;

    ModelParameters modelParameters;
    
    irr::video::IImage* radarImage; //Basic radar image
    irr::video::IImage* radarImageOverlaid; //WIth any 2d overlay
    irr::video::IImage* radarImageLarge; //Basic radar image, for full screen display
    irr::video::IImage* radarImageOverlaidLarge; //WIth any 2d overlay, for full screen display
    irr::video::IImage* radarImageChosen; //Should point to one of radarImage or radarImageLarge
    irr::video::IImage* radarImageOverlaidChosen; //Should point to one of radarImageOverlaid or radarImageOverlaidLarge
    //float accelerator;
    float tideHeight;
    float weather; //0-12.0
    float rainIntensity; //0-10
    float visibilityRange; //Nm
    float windDirection; //0-360
    float windSpeed; //Nm
    float streamOverrideDirection; //0-360
    float streamOverrideSpeed; //Nm
    bool streamOverride;
    uint32_t loopNumber; //u32 should be up to 4,294,967,295, so over 2 years at 60 fps
    float currentZoom; // Zoom currently in use
    float zoomLevel; // Zoom level that should be used if binos are on
    Terrain terrain;
    Light light;
    OwnShip ownShip;
    OtherShips otherShips;
    Buoys buoys;
    LandObjects landObjects;
    LandLights landLights;
    Camera camera;
    Camera radarCamera;
    Water water;
    Tide tide;
    Rain rain;
    Lines lines;
    RadarCalculation radarCalculation;
    RadarScreen radarScreen;
    ControlVisualiser portEngineVisual;
    ControlVisualiser stbdEngineVisual;
    ControlVisualiser portAzimuthThrottleVisual;
    ControlVisualiser stbdAzimuthThrottleVisual;
    ControlVisualiser wheelVisual;
    GUIMain* guiMain;
	ISound* sound;
    bool isMouseDown; //Updated by the event receiver, used by radar
    bool moveViewWithPrimary;
    ManOverboard manOverboard;

    //Simulation time handling
    uint32_t currentTime; //Computer clock time
    uint32_t previousTime; //Computer clock time
    float deltaTime;
    float scenarioTime; //Simulation internal time, starting at zero at 0000h on start day of simulation
    uint64_t scenarioOffsetTime; //Simulation day's start time from unix epoch (1 Jan 1970)
    uint64_t absoluteTime; //Unix timestamp for current time, including start day. Calculated from scenarioTime and scenarioOffsetTime

    //utility function to check for collision
    bool checkOwnShipCollision();

    //Offset position handling
    irr::core::vector3d<int64_t> offsetPosition;

    //store useful information
    std::string scenarioName;
    std::string worldName;
    std::string serialisedScenarioData;
    std::string worldModelReadmeText;

    //Structure to pass data to gui
    GUIData* guiData;
};
#endif
