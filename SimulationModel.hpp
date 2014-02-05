#ifndef __SIMULATIONMODEL_HPP_INCLUDED__
#define __SIMULATIONMODEL_HPP_INCLUDED__

#include "irrlicht.h"

#include "GUIMain.hpp"

using namespace irr;

class SimulationModel //Start of the 'Model' part of MVC
{

public:

    SimulationModel(IrrlichtDevice* dev, video::IVideoDriver* drv, scene::ISceneManager* scene, GUIMain* gui);

    void setSpeed(f32 spd);
    void setHeading(f32 hdg);
    void updateModel();

private:
    IrrlichtDevice* device;
    scene::IMeshSceneNode* ownShipNode;
    video::IVideoDriver* driver;
    scene::ISceneManager* smgr;
    scene::ICameraSceneNode* camera;
    GUIMain* guiMain;

    //Ship movement
    f32 heading;
    f32 xPos;
    f32 yPos;
    f32 zPos;
    f32 speed;
    u32 currentTime;
    u32 previousTime;
    f32 deltaTime;

    void setPosition(f32 x, f32 y, f32 z);

    void setRotation(f32 rx, f32 ry, f32 rz);

};
#endif
