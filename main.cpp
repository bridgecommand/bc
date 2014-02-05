// main.cpp
// Include the Irrlicht header
#include "irrlicht.h"

#include "GUIMain.hpp"
#include "SimulationModel.hpp"
#include "MyEventReceiver.hpp"

// Irrlicht Namespaces
using namespace irr;

int main()
{

    //create device
    IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(700,525),32,false,false,false,0);

    device->setWindowCaption(L"Bridge Command 5.Alpha - Irrlicht test example");

    video::IVideoDriver* driver = device->getVideoDriver();
    scene::ISceneManager* smgr = device->getSceneManager();

    //create GUI
    GUIMain guiMain(device);

    //Create simulation model
    SimulationModel model (device, driver, smgr, &guiMain); //Add link to gui

    //create event receiver, linked to model
    MyEventReceiver receiver(&model, &guiMain);
    device->setEventReceiver(&receiver);

    //main loop
    while(device->run())
    {

        model.updateModel();

        //Render
        driver->beginScene(true,true,video::SColor(255,100,101,140));
        smgr->drawAll();
        guiMain.drawGUI();
        driver->endScene();
    }

    device->drop();

    return(0);
}
