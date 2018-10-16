//Common launcher program
//This just launches Bridge Command or
//Map Controller executable depending
//on which button the user presses

#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

#include "irrlicht.h"
#include <iostream>
#include "../IniFile.hpp"
#include "../Lang.hpp"
#include "../Utilities.hpp"

//headers for execl
#ifdef _WIN32
#include <windows.h>
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

//Global definition for ini logger
namespace IniFile {
    irr::ILogger* irrlichtLogger = 0;
}

const irr::s32 BC_BUTTON = 1;
const irr::s32 MC_BUTTON = 2;
const irr::s32 RP_BUTTON = 3;
const irr::s32 ED_BUTTON = 4;
const irr::s32 MH_BUTTON = 5;
const irr::s32 INI_BC_BUTTON = 6;
const irr::s32 INI_MC_BUTTON = 7;
const irr::s32 INI_RP_BUTTON = 8;
const irr::s32 INI_MH_BUTTON = 9;
const irr::s32 DOC_BUTTON = 10;

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
                        _execl("./bridgecommand-bc.exe", "bridgecommand-bc.exe", NULL);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("../MacOS/bc.app/Contents/MacOS/bc", "bc", NULL);
                    #else
                        //Other (assumed posix)
                        execl("./bridgecommand-bc", "bridgecommand-bc", NULL);
                    #endif
                    #endif
                }
                if (id == MC_BUTTON) {
                    #ifdef _WIN32
                        _execl("./bridgecommand-mc.exe", "bridgecommand-mc.exe", NULL);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("../MacOS/mc.app/Contents/MacOS/mc", "mc", NULL);
                    #else
                        //Other (assumed posix)
                        execl("./bridgecommand-mc", "bridgecommand-mc", NULL);
                    #endif
                    #endif
                }
                if (id == RP_BUTTON) {
                    #ifdef _WIN32
                        _execl("./bridgecommand-rp.exe", "bridgecommand-rp.exe", NULL);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("../MacOS/rp.app/Contents/MacOS/rp", "rp", NULL);
                    #else
                        //Other (assumed posix)
                        execl("./bridgecommand-rp", "bridgecommand-rp", NULL);
                    #endif
                    #endif
                }
                if (id == ED_BUTTON) {
                    #ifdef _WIN32
                        _execl("./bridgecommand-ed.exe", "bridgecommand-ed.exe", NULL);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("../MacOS/ed.app/Contents/MacOS/ed", "ed", NULL);
                    #else
                        //Other (assumed posix)
                        execl("./bridgecommand-ed", "bridgecommand-ed", NULL);
                    #endif
                    #endif
                }
                if (id == MH_BUTTON) {
                    #ifdef _WIN32
                        _execl("./bridgecommand-mh.exe", "bridgecommand-mh.exe", NULL);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("../MacOS/mh.app/Contents/MacOS/mh", "mh", NULL);
                    #else
                        //Other (assumed posix)
                        execl("./bridgecommand-mh", "bridgecommand-mh", NULL);
                    #endif
                    #endif
                }

                if (id == INI_BC_BUTTON) {
                    #ifdef _WIN32
                        _execl("./bridgecommand-ini.exe", "bridgecommand-ini.exe", NULL);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("../MacOS/ini.app/Contents/MacOS/ini", "ini", NULL);
                    #else
                        //Other (assumed posix)
                        execl("./bridgecommand-ini", "bridgecommand-ini", NULL);
                    #endif
                    #endif
                }
                if (id == INI_MC_BUTTON) {
                    #ifdef _WIN32
                        _execl("./bridgecommand-ini.exe", "bridgecommand-ini.exe", "-M", NULL);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("../MacOS/ini.app/Contents/MacOS/ini", "ini", "-M", NULL);
                    #else
                        //Other (assumed posix)
                        execl("./bridgecommand-ini", "bridgecommand-ini", "-M", NULL);
                    #endif
                    #endif
                }
                if (id == INI_RP_BUTTON) {
                    #ifdef _WIN32
                        _execl("./bridgecommand-ini.exe", "bridgecommand-ini.exe", "-R", NULL);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("../MacOS/ini.app/Contents/MacOS/ini", "ini", "-R", NULL);
                    #else
                        //Other (assumed posix)
                        execl("./bridgecommand-ini", "bridgecommand-ini", "-R", NULL);
                    #endif
                    #endif
                }
                if (id == INI_MH_BUTTON) {
                    #ifdef _WIN32
                        _execl("./bridgecommand-ini.exe", "bridgecommand-ini.exe", "-H", NULL);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("../MacOS/ini.app/Contents/MacOS/ini", "ini", "-H", NULL);
                    #else
                        //Other (assumed posix)
                        execl("./bridgecommand-ini", "bridgecommand-ini", "-H", NULL);
                    #endif
                    #endif
                }
                if (id == DOC_BUTTON) {
                    #ifdef _WIN32
                        //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
                        ShellExecute(NULL, TEXT("open"), TEXT("doc\\index.html"), NULL, NULL, SW_SHOWNORMAL);
                        Sleep(5000);
                        exit(EXIT_SUCCESS);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("/usr/bin/open", "open", "../Resources/doc/index.html", NULL);
                    #else
                        //Other (assumed posix)
                        execl("/usr/bin/xdg-open", "xdg-open", "doc/index.html", NULL);
                        //If execuation gets to this point, it has failed to launch help. Bring up a message to tell user?

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

    u32 graphicsWidth = 200;
    u32 graphicsHeight = 465;
    u32 graphicsDepth = 32;
    bool fullScreen = false;

    //User read/write location - look in here first and the exe folder second for files
    std::string userFolder = Utilities::getUserDir();

    //Read basic ini settings
    std::string iniFilename = "bc5.ini";
    //Use local ini file if it exists
    if (Utilities::pathExists(userFolder + iniFilename)) {
        iniFilename = userFolder + iniFilename;
    }


    std::string modifier = IniFile::iniFileToString(iniFilename, "lang");
    if (modifier.length()==0) {
        modifier = "en"; //Default
    }
    std::string languageFile = "languageLauncher-";
    languageFile.append(modifier);
    languageFile.append(".txt");
    if (Utilities::pathExists(userFolder + languageFile)) {
        languageFile = userFolder + languageFile;
    }

    Lang language(languageFile);

    IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(graphicsWidth,graphicsHeight),graphicsDepth,fullScreen,false,false,0);
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
    irr::gui::IGUIButton* launchBC = device->getGUIEnvironment()->addButton(core::rect<s32>(10,10,190,75),0,BC_BUTTON,language.translate("startBC").c_str()); //i18n
    irr::gui::IGUIButton* launchED = device->getGUIEnvironment()->addButton(core::rect<s32>(10,100,190,130),0,ED_BUTTON,language.translate("startED").c_str()); //i18n
    irr::gui::IGUIButton* launchMC = device->getGUIEnvironment()->addButton(core::rect<s32>(10,140,190,170),0,MC_BUTTON,language.translate("startMC").c_str()); //i18n
    irr::gui::IGUIButton* launchRP = device->getGUIEnvironment()->addButton(core::rect<s32>(10,180,190,210),0,RP_BUTTON,language.translate("startRP").c_str()); //i18n
    irr::gui::IGUIButton* launchMH = device->getGUIEnvironment()->addButton(core::rect<s32>(10,220,190,250),0,MH_BUTTON,language.translate("startMH").c_str()); //i18n

    irr::gui::IGUIButton* launchINIBC = device->getGUIEnvironment()->addButton(core::rect<s32>(10,275,190,300),0,INI_BC_BUTTON,language.translate("startINIBC").c_str()); //i18n
    irr::gui::IGUIButton* launchINIMC = device->getGUIEnvironment()->addButton(core::rect<s32>(10,310,190,335),0,INI_MC_BUTTON,language.translate("startINIMC").c_str()); //i18n
    irr::gui::IGUIButton* launchINIRP = device->getGUIEnvironment()->addButton(core::rect<s32>(10,345,190,370),0,INI_RP_BUTTON,language.translate("startINIRP").c_str()); //i18n
    irr::gui::IGUIButton* launchINIMH = device->getGUIEnvironment()->addButton(core::rect<s32>(10,380,190,405),0,INI_MH_BUTTON,language.translate("startINIMH").c_str()); //i18n

    irr::gui::IGUIButton* launchDOC = device->getGUIEnvironment()->addButton(core::rect<s32>(10,430,190,455),0,DOC_BUTTON,language.translate("startDOC").c_str()); //i18n

    Receiver receiver;
    device->setEventReceiver(&receiver);

    while (device->run()) {
        driver->beginScene();
        device->getGUIEnvironment()->drawAll();
        driver->endScene();
    }
    return(0);
}
