#include "irrlicht.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include "PositionDataStruct.hpp"
#include "ShipDataStruct.hpp"
#include "Network.hpp"
#include "ControllerModel.hpp"
#include "GUI.hpp"
#include "EventReceiver.hpp"

#include "../IniFile.hpp"
#include "../Lang.hpp"
#include "../Utilities.hpp"

//Mac OS:
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

// Irrlicht Namespaces
using namespace irr;

//Set up global for ini reader to have access to irrlicht logger if needed.
namespace IniFile {
    irr::ILogger* irrlichtLogger = 0;
}

int main (int argc, char ** argv)
{

    //Mac OS:
    //Find starting folder
	#ifdef __APPLE__
    char exePath[1024];
    uint32_t pathSize = sizeof(exePath);
    std::string exeFolderPath = "";
    if (_NSGetExecutablePath(exePath, &pathSize) == 0) {
        std::string exePathString(exePath);
        size_t pos = exePathString.find_last_of("\\/");
        if (std::string::npos != pos) {
            exeFolderPath = exePathString.substr(0, pos);
        }
    }
	//change up from BridgeCommand.app/Contents/MacOS/mc.app/Contents/MacOS to BridgeCommand.app/Contents/Resources
    exeFolderPath.append("/../../../../Resources");
    //change to this path now, so ini file is read
    chdir(exeFolderPath.c_str());
    //Note, we use this again after the createDevice call
	#endif

    //User read/write location - look in here first and the exe folder second for files
    std::string userFolder = Utilities::getUserDir();

    std::string iniFilename = "repeater.ini";
    //Use local ini file if it exists
    if (Utilities::pathExists(userFolder + iniFilename)) {
        iniFilename = userFolder + iniFilename;
    }
    u32 graphicsWidth = IniFile::iniFileTou32(iniFilename, "graphics_width");
    u32 graphicsHeight = IniFile::iniFileTou32(iniFilename, "graphics_height");
    u32 graphicsDepth = IniFile::iniFileTou32(iniFilename, "graphics_depth");
    bool fullScreen = (IniFile::iniFileTou32(iniFilename, "graphics_mode")==1); //1 for full screen

    if (graphicsWidth == 0 ) {graphicsWidth=640;}
    if (graphicsHeight == 0 ) {graphicsHeight=480;}
    if (graphicsDepth == 0 ) {graphicsDepth=32;}

    //Load UDP network settings
    u32 udpPort = IniFile::iniFileTou32(iniFilename, "udp_send_port");
    if (udpPort == 0) {
        udpPort = 18304;
    }

    IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(graphicsWidth,graphicsHeight),graphicsDepth,fullScreen,false,false,0);
    video::IVideoDriver* driver = device->getVideoDriver();
    //scene::ISceneManager* smgr = device->getSceneManager();


    #ifdef __APPLE__
    //Mac OS - cd back to original dir - seems to be changed during createDevice
    io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        exit(EXIT_FAILURE); //Could not get file system TODO: Message for user
        std::cout << "Could not get filesystem" << std::endl;
    }
    fileSystem->changeWorkingDirectoryTo(exeFolderPath.c_str());
    #endif

    //load language
    //load language
    std::string languageFile = "languageRepeater.txt";
    if (Utilities::pathExists(userFolder + languageFile)) {
        languageFile = userFolder + languageFile;
    }
    Lang language(languageFile);

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
    Network network(udpPort);


    //GUI class
    GUIMain guiMain(device, &language);
    //Main model
    ControllerModel controller(device, &guiMain);

    //Create data structures to hold own ship, other ship and buoy data
    irr::f32 time = 0; //Time since start of day 1 of the scenario
    irr::f32 weather = 0; //(0-12)
    irr::f32 rain = 0; //(0-10)
    irr::f32 visibility = 10.1; //(0.1-10.1)
    ShipData ownShipData;

    //create event receiver, linked to model
    EventReceiver receiver(device, &controller, &guiMain, &network);
    device->setEventReceiver(&receiver);

    u32 timer = device->getTimer()->getRealTime();

    while(device->run()) {

        driver->beginScene(true, false, video::SColor(0,200,200,200)); //Don't need to clear Z buffer

        //Read in data from network
        network.update(time, ownShipData);

        //Update the internal model, and call the gui
        controller.update(time, ownShipData);

        driver->endScene();

        //Pause if faster than 30 fps (33ms)
        u32 newTimer = device->getTimer()->getRealTime();
        if (newTimer-timer<33) {
            device->sleep(33-(newTimer-timer));
        }
        timer = newTimer;
    }

    return(0);
}
