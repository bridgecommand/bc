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
class Sound;

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
#include "OperatingModeEnum.hpp"

class SimulationModel //Start of the 'Model' part of MVC
{

public:

    SimulationModel(irr::IrrlichtDevice* dev, irr::scene::ISceneManager* scene, GUIMain* gui, Sound* sound, ScenarioData scenarioData, OperatingMode::Mode mode, irr::f32 viewAngle, irr::f32 lookAngle, irr::f32 cameraMinDistance, irr::f32 cameraMaxDistance, irr::u32 disableShaders);
    ~SimulationModel();
    irr::f32 longToX(irr::f32 longitude) const;
    irr::f32 latToZ(irr::f32 latitude) const;
    void setSpeed(irr::f32 spd); //Sets the own ship's speed
    void setHeading(irr::f32 hdg); //Sets the own ship's heading

    irr::f32 getRateOfTurn() const;
    void setRateofTurn(irr::f32 rudder); //Set the rate of turn (-ve is port, +ve is stbd)



    void setRateOfTurn(irr::f32 rateOfTurn);
    void setPos(irr::f32 positionX, irr::f32 positionZ);
    void setRudder(irr::f32 rudder); //Set the rudder (-ve is port, +ve is stbd)
    void setWheel(irr::f32 wheel); //Set the wheel (-ve is port, +ve is stbd) DEE
    irr::f32 getRudder() const;
    irr::f32 getWheel() const; // DEE
    void setPortEngine(irr::f32 port); //Set the engine, (-ve astern, +ve ahead), range is +-1
    void setStbdEngine(irr::f32 stbd); //Set the engine, (-ve astern, +ve ahead), range is +-1
    irr::f32 getPortEngine() const; //Range +-1
    irr::f32 getStbdEngine() const; //Range +-1
    irr::f32 getPortEngineRPM() const;
    irr::f32 getStbdEngineRPM() const;
    void setBowThruster(irr::f32 proportion);
    void setSternThruster(irr::f32 proportion);
    void setAccelerator(irr::f32 accelerator); //Set simulation time compression
    irr::f32 getAccelerator() const;
    irr::f32 getSpeed() const; //Gets the own ship's speed
    irr::f32 getHeading() const; //Gets the own ship's heading

    irr::f32 getLat() const;
    irr::f32 getLong() const;
    irr::f32 getPosX() const;
    irr::f32 getPosZ() const;
    irr::f32 getCOG() const;
    irr::f32 getSOG() const; //In metres/second

    irr::f32 getWaveHeight(irr::f32 posX, irr::f32 posZ) const; //Return wave height (not tide) at the world position specified
    irr::core::vector2df getLocalNormals(irr::f32 relPosX, irr::f32 relPosZ) const;

    irr::core::vector2df getTidalStream(irr::f32 longitude, irr::f32 latitude, uint64_t absoluteTime) const; //Tidal stream in m/s for the specified absolute position

    //void getTime(irr::u8& hour, irr::u8& min, irr::u8& sec) const;
    //void getDate(irr::u8& day, irr::u8& month, irr::u16& year) const;
    uint64_t getTimestamp() const; //The unix timestamp in s
    uint64_t getTimeOffset() const; //The timestamp at the start of the first day of the scenario
    irr::f32 getTimeDelta() const; //The change in time (s) since the start of the start day of the scenario
    void     setTimeDelta(irr::f32 scenarioTime);

    irr::u32 getNumberOfOtherShips() const;
    irr::u32 getNumberOfBuoys() const;
    std::string getOtherShipName(int number) const;
    irr::f32 getOtherShipPosX(int number) const;
    irr::f32 getOtherShipPosZ(int number) const;
    irr::f32 getOtherShipHeading(int number) const;
    irr::f32 getOtherShipSpeed(int number) const; //Speed in m/s
    void setOtherShipHeading(int number, irr::f32 hdg);
    void setOtherShipPos(int number, irr::f32 positionX, irr::f32 positionZ);
    void setOtherShipSpeed(int number, irr::f32 speed); //Speed in m/s
    std::vector<Leg> getOtherShipLegs(int number) const;
    irr::f32 getBuoyPosX(int number) const;
    irr::f32 getBuoyPosZ(int number) const;
    void changeOtherShipLeg(int shipNumber, int legNumber, irr::f32 bearing, irr::f32 speed, irr::f32 distance);
    void addOtherShipLeg(int shipNumber, int afterLegNumber, irr::f32 bearing, irr::f32 speed, irr::f32 distance);
    void deleteOtherShipLeg(int shipNumber, int legNumber);
	std::string getOwnShipEngineSound() const;
	std::string getOwnShipWaveSound() const;
	std::string getOwnShipHornSound() const;


    void setWeather(irr::f32 weather); //Range 0-12.
    irr::f32 getWeather() const;
    void setRain(irr::f32 rainIntensity); //Range 0-10
    irr::f32 getRain() const;
    void setVisibility(irr::f32 visibilityNm);
    irr::f32 getVisibility() const;
    void setWaterVisible(bool visible);
    void lookUp();
    void lookDown();
    void lookLeft();
    void lookRight();
    void lookStepLeft();
    void lookStepRight();
    void lookAhead();
    void lookAstern();
    void lookPort();
    void lookStbd();
    void changeView();
    void setView(irr::u32 view);
    irr::u32 getCameraView() const;
    void increaseRadarRange();
    void decreaseRadarRange();
    void setRadarGain(irr::f32 value);
    void setRadarClutter(irr::f32 value);
    void setRadarRain(irr::f32 value);
    void setPIData(irr::s32 PIid, irr::f32 PIbearing, irr::f32 PIrange);
    irr::f32 getPIbearing(irr::s32 PIid) const;
    irr::f32 getPIrange(irr::s32 PIid) const;
    void increaseRadarEBLRange();
    void decreaseRadarEBLRange();
    void increaseRadarEBLBrg();
    void decreaseRadarEBLBrg();
    void setRadarNorthUp();
    void setRadarCourseUp();
    void setRadarHeadUp();
    void setArpaOn(bool on);
    void setRadarARPARel();
    void setRadarARPATrue();
    void setRadarARPAVectors(irr::f32 vectorMinutes);
    void setRadarDisplayRadius(irr::u32 radiusPx);
    void setMainCameraActive();
    void setRadarCameraActive();
    void updateViewport(irr::f32 aspect);
    void setMouseDown(bool isMouseDown);
    void setZoom(bool zoomOn);
    irr::u32 getLoopNumber() const;
    std::string getSerialisedScenario() const;
    std::string getScenarioName() const;
    std::string getWorldName() const;
    void releaseManOverboard();
    void retrieveManOverboard();
    bool getManOverboardVisible() const;
    irr::f32 getManOverboardPosX() const;
    irr::f32 getManOverboardPosZ() const;
    void setManOverboardVisible(bool visible); //To be used directly, eg when in secondary display mode only
    void setManOverboardPos(irr::f32 positionX, irr::f32 positionZ);   //To be used directly, eg when in secondary display mode only
    bool hasGPS() const;
    bool isSingleEngine() const;
    bool hasDepthSounder() const;
    irr::f32 getMaxSounderDepth() const;
    bool hasBowThruster() const;
    bool hasSternThruster() const;
    bool hasTurnIndicator() const;

	void startHorn();
	void endHorn();

    void update();

private:
    irr::IrrlichtDevice* device;
    irr::video::IVideoDriver* driver;
    irr::scene::ISceneManager* smgr;
    OperatingMode::Mode mode; //What mode are we in
    irr::f32 viewAngle;
    irr::video::IImage* radarImage; //Basic radar image
    irr::video::IImage* radarImageOverlaid; //WIth any 2d overlay
    //irr::f32 accelerator;
    irr::f32 tideHeight;
    irr::f32 weather; //0-12.0
    irr::f32 rainIntensity; //0-10
    irr::f32 visibilityRange; //Nm
    irr::u32 loopNumber; //u32 should be up to 4,294,967,295, so over 2 years at 60 fps
    irr::f32 zoom;
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
    RadarCalculation radarCalculation;
    RadarScreen radarScreen;
    GUIMain* guiMain;
	Sound* sound;
    bool isMouseDown; //Updated by the event receiver, used by radar
    ManOverboard manOverboard;

    //Simulation time handling
    irr::u32 currentTime; //Computer clock time
    irr::u32 previousTime; //Computer clock time
    irr::f32 deltaTime;
    irr::f32 scenarioTime; //Simulation internal time, starting at zero at 0000h on start day of simulation
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

    //Structure to pass data to gui
    GUIData* guiData;

};
#endif
