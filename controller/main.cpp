#include "irrlicht.h"
#include <iostream>
#include <cstdio>

#include "Network.hpp"
#include "ControllerModel.hpp"

// Irrlicht Namespaces
using namespace irr;

int main (int argc, char ** argv)
{

    IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(800,600),32,false,false,false,0); //Fixme: Hardcoded size, depth and full screen
    video::IVideoDriver* driver = device->getVideoDriver();
    scene::ISceneManager* smgr = device->getSceneManager();

    ControllerModel controller;
    Network network(&controller);


    /* Wait up to 100 milliseconds for an event. */
    while(device->run()) {
        network.update();
    }

    return(0);
}
