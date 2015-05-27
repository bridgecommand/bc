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

// main.cpp
// Include the Irrlicht header
#include "irrlicht.h"

#include "GUIMain.hpp"
#include "SimulationModel.hpp"
#include "ScenarioChoice.hpp"
#include "MyEventReceiver.hpp"
#include "Network.hpp"
#include "IniFile.hpp"
#include "Constants.hpp"
#include "Lang.hpp"
#include "NMEA.hpp"

#include <cstdlib> //For rand(), srand()
#include <vector>
#include <sstream>

// Irrlicht Namespaces
using namespace irr;

int main()
{

    //Read basic ini settings
    std::string iniFilename = "bc5.ini";
    u32 graphicsWidth = IniFile::iniFileTou32(iniFilename, "graphics_width");
    u32 graphicsHeight = IniFile::iniFileTou32(iniFilename, "graphics_height");
    u32 graphicsDepth = IniFile::iniFileTou32(iniFilename, "graphics_depth");
    bool fullScreen = (IniFile::iniFileTou32(iniFilename, "graphics_mode")==1); //1 for full screen

    //Load joystick settings, subtract 1 as first axis is 0 internally (not 1)
    u32 portJoystickAxis = IniFile::iniFileTou32(iniFilename, "port_throttle_channel")-1;
    u32 stbdJoystickAxis = IniFile::iniFileTou32(iniFilename, "stbd_throttle_channel")-1;
    u32 rudderJoystickAxis = IniFile::iniFileTou32(iniFilename, "rudder_channel")-1;

    //Load NMEA settings
    std::string serialPortName = IniFile::iniFileToString(iniFilename, "NMEA_ComPort");

    //Sensible defaults if not set
    if (graphicsWidth==0) {graphicsWidth=800;}
    if (graphicsHeight==0) {graphicsHeight=600;}
    if (graphicsDepth==0) {graphicsDepth=32;}

    //set size of camera window
    u32 graphicsWidth3d = graphicsWidth;
    u32 graphicsHeight3d = graphicsHeight*0.6;
    f32 aspect = (f32)graphicsWidth/(f32)graphicsHeight;
    f32 aspect3d = (f32)graphicsWidth3d/(f32)graphicsHeight3d;

    //create device
    IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(graphicsWidth,graphicsHeight),graphicsDepth,fullScreen,false,false,0);
    device->setWindowCaption(core::stringw(LONGNAME.c_str()).c_str()); //Fixme - odd conversion from char* to wchar*!

    video::IVideoDriver* driver = device->getVideoDriver();
    scene::ISceneManager* smgr = device->getSceneManager();

    //load language
    Lang language("language.txt");

    //Set font : Todo - make this configurable
    gui::IGUIFont *font = device->getGUIEnvironment()->getFont("media/lucida.xml");
    if (font == 0) {
        std::cout << "Could not load font, using default" << std::endl;
    } else {
        //set skin default font
        device->getGUIEnvironment()->getSkin()->setFont(font);
    }

    //Choose scenario
    std::string scenarioName = "";
    std::string hostname = "";
    ScenarioChoice scenarioChoice(device,&language);
    scenarioChoice.chooseScenario(scenarioName, hostname);

    //std::cout << "Chosen " << scenarioName << " with " << hostname << std::endl;

    //seed random number generator
    std::srand(device->getTimer()->getTime());

    //create GUI
    GUIMain guiMain(device, &language);

    //Create simulation model
    SimulationModel model(device, smgr, &guiMain, scenarioName);

    //load realistic water
    //RealisticWaterSceneNode* realisticWater = new RealisticWaterSceneNode(smgr, 4000, 4000, "./",irr::core::dimension2du(512, 512),smgr->getRootSceneNode());

    //create event receiver, linked to model
    MyEventReceiver receiver(device, &model, &guiMain, portJoystickAxis, stbdJoystickAxis, rudderJoystickAxis);
    device->setEventReceiver(&receiver);

    //Create networking, linked to model
    Network network(&model);
    network.connectToServer(hostname);

    //create NMEA serial port, linked to model
    NMEA nmea(&model, serialPortName);

    //set up timing for NMEA: FIXME: Make this a defined constant
    u32 nextNMEATime = device->getTimer()->getTime()+250;

    //main loop
    while(device->run())
    {

        network.update();

        //Check if time has elapsed, so we send data once per second.
        if (device->getTimer()->getTime() >= nextNMEATime) {
            nmea.updateNMEA();
            nmea.sendNMEASerial();
            nextNMEATime = device->getTimer()->getTime()+250; //Fixme: Make this a defined constant.
        }

        model.update();

        //Set up
        driver->setViewPort(core::rect<s32>(0,0,graphicsWidth,graphicsHeight)); //Full screen before beginScene
        driver->beginScene(true,true,video::SColor(255,100,101,140));

        //3d view portion
        if (guiMain.getShowInterface()) {
            driver->setViewPort(core::rect<s32>(0,0,graphicsWidth3d,graphicsHeight3d));
            model.setAspectRatio(aspect3d);
        } else {
            driver->setViewPort(core::rect<s32>(0,0,graphicsWidth,graphicsHeight));
            model.setAspectRatio(aspect);
        }
        model.setMainCameraActive();

        smgr->drawAll();

        //radar view portion
        if (graphicsHeight>graphicsHeight3d && guiMain.getShowInterface()) {
            //Fixme: May want to re-introduce this
            //realisticWater->setVisible(false); //Hide the reflecting water, as this updates itself on drawAll()
            driver->setViewPort(core::rect<s32>(graphicsWidth-(graphicsHeight-graphicsHeight3d),graphicsHeight3d,graphicsWidth,graphicsHeight));
            model.setRadarCameraActive();
            smgr->drawAll();
            //realisticWater->setVisible(true);
        }

        //gui
        driver->setViewPort(core::rect<s32>(0,0,graphicsWidth,graphicsHeight)); //Full screen for gui
        guiMain.drawGUI();
        driver->endScene();

    }

    device->drop();
    //networking should be stopped (presumably with destructor when it goes out of scope?)


    return(0);
}
