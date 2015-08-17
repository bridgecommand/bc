//Common launcher program - WINDOWS ONLY
//This just launches BridgeCommand.exe or
//controller.exe depending on which button
//the user presses

#include "irrlicht.h"
#include <iostream>
#include <process.h>

#include "../Lang.hpp"

// Irrlicht Namespaces
using namespace irr;

const irr::s32 BC_BUTTON = 1;
const irr::s32 MC_BUTTON = 2;

//TODO: add event receiver

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
                    _execl("BridgeCommand.exe", "BridgeCommand.exe", NULL); //Windows only
                }
                if (id == MC_BUTTON) {
                    _execl("controller.exe", "controller.exe", NULL); //Windows only
                }
            }
        }
        return false;
    }
};

int main (int argc, char ** argv)
{

    u32 graphicsWidth = 400;
    u32 graphicsHeight = 100;
    u32 graphicsDepth = 32;
    bool fullScreen = false;

    Lang language("languageLauncher.txt");

    IrrlichtDevice* device = createDevice(video::EDT_BURNINGSVIDEO, core::dimension2d<u32>(graphicsWidth,graphicsHeight),graphicsDepth,fullScreen,false,false,0);
    video::IVideoDriver* driver = device->getVideoDriver();

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
