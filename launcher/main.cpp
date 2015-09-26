//Common launcher program
//This just launches Bridge Command or
//Map Controller executable depending
//on which button the user presses

#include "irrlicht.h"
#include <iostream>
#include "../Lang.hpp"

//headers for execl
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

//Mac OS:
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

// Irrlicht Namespaces
using namespace irr;

const irr::s32 BC_BUTTON = 1;
const irr::s32 MC_BUTTON = 2;

//Event receiver: This does the actual launching
class Receiver : public IEventReceiver
{
public:
    Receiver() { }

    virtual bool OnEvent(const SEvent& event)
    {
        if (event.EventType == EET_GUI_EVENT) {
            if (event.GUIEvent.EventType == gui::EGET_BUTTON_CLICKED ) {
                s32 id = event.GUIEvent.Caller->getID();
                if (id == BC_BUTTON) {
                    #ifdef _WIN32
                        execl("./BridgeCommand.exe", "BridgeCommand.exe", NULL);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("../MacOS/bc.app/Contents/MacOS/bc", "bc", NULL);
                    #else
                        //Other (assumed posix)
                        execl("./bridgecommand", "bridgecommand", NULL);
                    #endif
                    #endif
                }
                if (id == MC_BUTTON) {
                    #ifdef _WIN32
                        execl("./controller.exe", "controller.exe", NULL);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("../MacOS/mc.app/Contents/MacOS/mc", "mc", NULL);
                    #else
                        //Other (assumed posix)
                        execl("./mapController", "mapController", NULL);
                    #endif
                    #endif
                }
            }
        }
        return false;
    }
};

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
    //change up from BridgeCommand.app/Contents/MacOS to ../Resources
    exeFolderPath.append("/../Resources");
    //change to this path now
    chdir(exeFolderPath.c_str());
    //Note, we use this again after the createDevice call
	#endif

    u32 graphicsWidth = 400;
    u32 graphicsHeight = 100;
    u32 graphicsDepth = 32;
    bool fullScreen = false;

    Lang language("languageLauncher.txt");

    IrrlichtDevice* device = createDevice(video::EDT_BURNINGSVIDEO, core::dimension2d<u32>(graphicsWidth,graphicsHeight),graphicsDepth,fullScreen,false,false,0);
    video::IVideoDriver* driver = device->getVideoDriver();

    #ifdef __APPLE__
    //Mac OS - cd back to original dir - seems to be changed during createDevice
    io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        exit(EXIT_FAILURE); //Could not get file system TODO: Message for user
        std::cout << "Could not get filesystem" << std::endl;
    }
    fileSystem->changeWorkingDirectoryTo(exeFolderPath.c_str());
    #endif

    //Set font : Todo - make this configurable
    gui::IGUIFont *font = device->getGUIEnvironment()->getFont("media/lucida.xml");
    if (font == 0) {
        std::cout << "Could not load font, using default" << std::endl;
    } else {
        //set skin default font
        device->getGUIEnvironment()->getSkin()->setFont(font);
    }

    //Add launcher buttons
    irr::gui::IGUIButton* launchBC = device->getGUIEnvironment()->addButton(core::rect<s32>(10,10,190,90),0,BC_BUTTON,language.translate("startBC").c_str()); //i18n
    irr::gui::IGUIButton* launchMC = device->getGUIEnvironment()->addButton(core::rect<s32>(210,10,390,90),0,MC_BUTTON,language.translate("startMC").c_str()); //i18n

    Receiver receiver;
    device->setEventReceiver(&receiver);

    while (device->run()) {
        driver->beginScene();
        device->getGUIEnvironment()->drawAll();
        driver->endScene();
    }
    return(0);
}
