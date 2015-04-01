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

#include "irrlicht.h"

#include "GUIMain.hpp"
#include "Terrain.hpp"
#include "Light.hpp"
#include "Water.hpp"
#include "Rain.hpp"
#include "Tide.hpp"
#include "Sky.hpp"
#include "Buoys.hpp"
#include "OtherShips.hpp"
#include "LandObjects.hpp"
#include "LandLights.hpp"
#include "OwnShip.hpp"
#include "Camera.hpp"
#include "RadarCalculation.hpp"
#include "RadarScreen.hpp"
#include "IniFile.hpp" //For ini handling: confirm if this is needed

class SimulationModel //Start of the 'Model' part of MVC
{

public:

    SimulationModel(irr::IrrlichtDevice* dev, irr::scene::ISceneManager* scene, GUIMain* gui, std::string scenarioName);
    ~SimulationModel();
    irr::f32 longToX(irr::f32 longitude) const;
    irr::f32 latToZ(irr::f32 latitude) const;
    void setSpeed(irr::f32 spd); //Sets the own ship's speed
    void setHeading(irr::f32 hdg); //Sets the own ship's heading
    void setRudder(irr::f32 rudder); //Set the rudder (-ve is port, +ve is stbd)
    irr::f32 getRudder() const;
    void setPortEngine(irr::f32 port); //Set the engine, (-ve astern, +ve ahead)
    void setStbdEngine(irr::f32 stbd); //Set the engine, (-ve astern, +ve ahead)
    irr::f32 getPortEngineRPM() const;
    irr::f32 getStbdEngineRPM() const;
    //irr::f32 getPortEngine() const;
    //irr::f32 getStbdEngine() const;
    void setAccelerator(irr::f32 accelerator); //Set simulation time compression
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
    irr::u64 getTimestamp() const; //The unix timestamp in s
    irr::u64 getTimeOffset() const; //The current 'offset' time, ie the timestamp when last normalised
    irr::f32 getTimeDelta() const; //The change in time (s) since last normalisation

    irr::u32 getNumberOfOtherShips() const;
    irr::u32 getNumberOfBuoys() const;
    std::string getOtherShipName(int number) const;
    irr::f32 getOtherShipPosX(int number) const;
    irr::f32 getOtherShipPosZ(int number) const;
    irr::f32 getOtherShipHeading(int number) const;
    std::vector<Leg> getOtherShipLegs(int number) const;
    irr::f32 getBuoyPosX(int number) const;
    irr::f32 getBuoyPosZ(int number) const;

    void setWeather(irr::f32 weather); //Range 0-12.
    irr::f32 getWeather() const;
    void setRain(irr::f32 rainIntensity); //Range 0-10
    irr::f32 getRain() const;
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
    void setMainCameraActive();
    void setRadarCameraActive();
    void setAspectRatio(irr::f32 aspect);
    irr::u32 getLoopNumber() const;
    void update();

private:
    irr::IrrlichtDevice* device;
    irr::video::IVideoDriver* driver;
    irr::scene::ISceneManager* smgr;
    irr::video::IImage* radarImage;
    //irr::f32 accelerator;
    irr::f32 tideHeight;
    irr::f32 weather; //0-12.0
    irr::f32 rainIntensity; //0-10
    irr::u32 loopNumber; //Todo: check if this is a reasonable size
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

    //Simulation time handling
    irr::u32 currentTime; //Computer clock time
    irr::u32 previousTime; //Computer clock time
    irr::f32 deltaTime;
    irr::f32 scenarioTime; //Simulation internal time, starting at zero at 0000h on start day of simulation
    irr::u64 scenarioOffsetTime; //Simulation day's start time from unix epoch (1 Jan 1970)
    irr::u64 absoluteTime; //Unix timestamp for current time, including start day

    //Offset position handling
    irr::core::vector3d<irr::s64> offsetPosition;//Fixme: check size of this

};
#endif
