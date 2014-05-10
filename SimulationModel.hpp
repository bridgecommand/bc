#ifndef __SIMULATIONMODEL_HPP_INCLUDED__
#define __SIMULATIONMODEL_HPP_INCLUDED__

#include "irrlicht.h"

#include "GUIMain.hpp"
#include "Terrain.hpp"
#include "Water.hpp"
#include "Sky.hpp"
#include "Buoys.hpp"
#include "OtherShips.hpp"
#include "OwnShip.hpp"
#include "Camera.hpp"
#include "RadarCalculation.hpp"
#include "RadarScreen.hpp"

class SimulationModel //Start of the 'Model' part of MVC
{

public:

    SimulationModel(irr::IrrlichtDevice* dev, irr::video::IVideoDriver* drv, irr::scene::ISceneManager* scene, GUIMain* gui);

    const irr::f32 longToX(irr::f32 longitude);
    const irr::f32 latToZ(irr::f32 latitude);
    void setSpeed(irr::f32 spd); //Sets the own ship's speed
    void setHeading(irr::f32 hdg); //Sets the own ship's heading
    const irr::f32 getSpeed(); //Gets the own ship's speed
    const irr::f32 getHeading(); //Gets the own ship's heading
    void update();

private:
    irr::IrrlichtDevice* device;
    irr::video::IVideoDriver* driver;
    irr::scene::ISceneManager* smgr;
    Terrain terrain;
    OwnShip ownShip;
    OtherShips otherShips;
    Buoys buoys;
    Camera camera;
    RadarCalculation radarCalculation;
    RadarScreen radarScreen;
    GUIMain* guiMain;

    //Simulation time handling
    irr::u32 currentTime;
    irr::u32 previousTime;
    irr::f32 deltaTime;

};
#endif
