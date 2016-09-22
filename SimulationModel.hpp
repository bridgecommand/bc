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
#include "Camera.hpp"
#include "RadarCalculation.hpp"
#include "RadarScreen.hpp"
#include "OperatingModeEnum.hpp"

class SimulationModel //Start of the 'Model' part of MVC
{

public:

    SimulationModel(irr::IrrlichtDevice* dev, irr::scene::ISceneManager* scene, GUIMain* gui, ScenarioData scenarioData, OperatingMode::Mode mode, irr::f32 viewAngle, irr::f32 lookAngle, irr::f32 cameraMinDistance, irr::f32 cameraMaxDistance);
    ~SimulationModel();
    irr::f32 longToX(irr::f32 longitude) const;
    irr::f32 latToZ(irr::f32 latitude) const;
    // DB Extensions
    irr::f32 xToLong(irr::f32 x) const;
    irr::f32 zToLat(irr::f32 z) const;

    void setSpeed(irr::f32 spd); //Sets the own ship's speed
    void setHeading(irr::f32 hdg); //Sets the own ship's heading
    irr::f32 getRateOfTurn() const;
    void setRateOfTurn(irr::f32 rateOfTurn);
    void setPos(irr::f32 positionX, irr::f32 positionZ);
    void setRudder(irr::f32 rudder); //Set the rudder (-ve is port, +ve is stbd)
    irr::f32 getRudder() const;
    void setPortEngine(irr::f32 port); //Set the engine, (-ve astern, +ve ahead)
    void setStbdEngine(irr::f32 stbd); //Set the engine, (-ve astern, +ve ahead)
    irr::f32 getPortEngineRPM() const;
    irr::f32 getStbdEngineRPM() const;
    //irr::f32 getPortEngine() const;
    //irr::f32 getStbdEngine() const;
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

    void setWeather(irr::f32 weather); //Range 0-12.
    irr::f32 getWeather() const;
    void setRain(irr::f32 rainIntensity); //Range 0-10
    irr::f32 getRain() const;
    void setVisibility(irr::f32 visibilityNm);
    irr::f32 getVisibility() const;
    void lookUp();
    void lookDown();
    void lookLeft();
    void lookRight();
    void lookAhead();
    void lookAstern();
    void lookPort();
    void lookStbd();
    void changeView();
    irr::u32 getCameraView() const;
    void increaseRadarRange();
    void decreaseRadarRange();
    void setRadarGain(irr::f32 value);
    void setRadarClutter(irr::f32 value);
    void setRadarRain(irr::f32 value);
    void increaseRadarEBLRange();
    void decreaseRadarEBLRange();
    void increaseRadarEBLBrg();
    void decreaseRadarEBLBrg();
    void setRadarNorthUp();
    void setRadarCourseUp();
    void setRadarHeadUp();
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
    irr::u32 loopNumber; //Todo: check if this is a reasonable size
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
    bool isMouseDown; //Updated by the event receiver, used by radar

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

};
#endif
