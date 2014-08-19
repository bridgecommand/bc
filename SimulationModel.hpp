#ifndef __SIMULATIONMODEL_HPP_INCLUDED__
#define __SIMULATIONMODEL_HPP_INCLUDED__

#include <iostream> //For debugging
#include <string>

#include "irrlicht.h"

#include "GUIMain.hpp"
#include "Terrain.hpp"
#include "Light.hpp"
#include "Water.hpp"
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

    irr::f32 longToX(irr::f32 longitude) const ;
    irr::f32 latToZ(irr::f32 latitude) const;
    void setSpeed(irr::f32 spd); //Sets the own ship's speed
    void setHeading(irr::f32 hdg); //Sets the own ship's heading
    void setAccelerator(irr::f32 accelerator); //Set simulation time compression
    irr::f32 getSpeed() const; //Gets the own ship's speed
    irr::f32 getHeading() const; //Gets the own ship's heading
    void lookLeft();
    void lookRight();
    void lookAhead();
    void lookAstern();
    void lookPort();
    void lookStbd();
    void changeView();
    void update();

private:
    irr::IrrlichtDevice* device;
    irr::video::IVideoDriver* driver;
    irr::scene::ISceneManager* smgr;
    irr::f32 accelerator;
    irr::f32 tideHeight;
    Terrain terrain;
    Light light;
    OwnShip ownShip;
    OtherShips otherShips;
    Buoys buoys;
    LandObjects landObjects;
    LandLights landLights;
    Camera camera;
    Water water;
    Tide tide;
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

};
#endif
