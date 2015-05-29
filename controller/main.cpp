#include "irrlicht.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include "PositionDataStruct.hpp"
#include "ShipDataStruct.hpp"
#include "OtherShipDataStruct.hpp"
#include "Network.hpp"
#include "ControllerModel.hpp"
#include "GUI.hpp"
#include "EventReceiver.hpp"

#include "../IniFile.hpp"
#include "../Lang.hpp"

// Irrlicht Namespaces
using namespace irr;

int main (int argc, char ** argv)
{

    std::string iniFilename = "map.ini";
    u32 graphicsWidth = IniFile::iniFileTou32(iniFilename, "graphics_width");
    u32 graphicsHeight = IniFile::iniFileTou32(iniFilename, "graphics_height");
    u32 graphicsDepth = IniFile::iniFileTou32(iniFilename, "graphics_depth");
    bool fullScreen = (IniFile::iniFileTou32(iniFilename, "graphics_mode")==1); //1 for full screen

    IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(graphicsWidth,graphicsHeight),graphicsDepth,fullScreen,false,false,0);
    video::IVideoDriver* driver = device->getVideoDriver();
    //scene::ISceneManager* smgr = device->getSceneManager();

    //load language
    Lang language("languageController.txt");

    //Set font : Todo - make this configurable
    gui::IGUIFont *font = device->getGUIEnvironment()->getFont("media/lucida.xml");
    if (font == 0) {
        std::cout << "Could not load font, using default" << std::endl;
    } else {
        //set skin default font
        device->getGUIEnvironment()->getSkin()->setFont(font);
    }

    //Classes:  Network and Controller share data with shared data structures (passed by ref). Controller then pushes data to the GUI
    //Network class
    Network network;

    //Find world model to use, from the network
    irr::gui::IGUIWindow* patienceWindow = device->getGUIEnvironment()->addWindow(core::rect<s32>(10, 10, driver->getScreenSize().Width-10, driver->getScreenSize().Height-10), false, language.translate("waiting").c_str());
    irr::gui::IGUIStaticText* patienceText = device->getGUIEnvironment()->addStaticText(language.translate("startBC").c_str(), core::rect<s32>(10,40,driver->getScreenSize().Width-30, driver->getScreenSize().Height-40), true, false, patienceWindow);

    std::string worldName = "";
    while (device->run() && worldName.size() == 0) {
        driver->beginScene();
        device->getGUIEnvironment()->drawAll();
        driver->endScene();
        worldName = network.findWorldName();
    }
    patienceText->remove();
    patienceWindow->remove();
    if (worldName.size() == 0) {
        std::cout << "Could not receive world name" << std::endl;
        exit(EXIT_FAILURE);
    }

    //GUI class
    GUIMain guiMain(device, &language);
    //Main model
    ControllerModel controller(device, &guiMain, worldName);

    //Create data structures to hold own ship, other ship and buoy data
    irr::f32 time = 0; //Time since start of day 1 of the scenario
    ShipData ownShipData;
    std::vector<PositionData> buoysData;
    std::vector<OtherShipData> otherShipsData;

    //create event receiver, linked to model
    EventReceiver receiver(device, &controller, &guiMain, &network);
    device->setEventReceiver(&receiver);

    while(device->run()) {

        driver->beginScene();

        //Read in data from network
        network.update(time, ownShipData, otherShipsData, buoysData);

        //Update the internal model, and call the gui
        controller.update(time, ownShipData, otherShipsData, buoysData);

        driver->endScene();
    }

    return(0);
}
