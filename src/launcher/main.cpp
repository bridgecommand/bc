//Common launcher program
//This just launches Bridge Command or
//Map Controller executable depending
//on which button the user presses

#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

#include "irrlicht.h"
#include <iostream>
#include <thread>
#include "../IniFile.hpp"
#include "../Lang.hpp"
#include "../Utilities.hpp"
#include "../Constants.hpp"

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
//using namespace irr;

const int FONT_SIZE_DEFAULT = 12;

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
const irr::s32 USER_BUTTON = 11;
const irr::s32 EXIT_BUTTON = 12;

std::string userFolder;

//Event receiver: This does the actual launching
class Receiver : public irr::IEventReceiver
{
public:
    Receiver() { }

    virtual bool OnEvent(const irr::SEvent& event)
    {
        if (event.EventType == irr::EET_GUI_EVENT) {
            if (event.GUIEvent.EventType == irr::gui::EGET_BUTTON_CLICKED ) {
                irr::s32 id = event.GUIEvent.Caller->getID();

                if (id == EXIT_BUTTON) {
                    exit(EXIT_SUCCESS);
                }

                #ifndef _WIN32
                int pid = fork();  // posix only (GNU/Linux, MacOS)
                if (pid > 0) return false;
                #endif

                if (id == BC_BUTTON) {
                    #ifdef _WIN32
                        ShellExecute(NULL, NULL, "bridgecommand-bc.exe", NULL, NULL, SW_SHOW);
                        //_execl("./bridgecommand-bc.exe", "bridgecommand-bc.exe", NULL);
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
                        ShellExecute(NULL, NULL, "bridgecommand-mc.exe", NULL, NULL, SW_SHOW);
                        //_execl("./bridgecommand-mc.exe", "bridgecommand-mc.exe", NULL);
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
                        ShellExecute(NULL, NULL, "bridgecommand-rp.exe", NULL, NULL, SW_SHOW);
                        //_execl("./bridgecommand-rp.exe", "bridgecommand-rp.exe", NULL);
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
                        ShellExecute(NULL, NULL, "bridgecommand-ed.exe", NULL, NULL, SW_SHOW);
                        //_execl("./bridgecommand-ed.exe", "bridgecommand-ed.exe", NULL);
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
                        ShellExecute(NULL, NULL, "bridgecommand-mh.exe", NULL, NULL, SW_SHOW);
                        //_execl("./bridgecommand-mh.exe", "bridgecommand-mh.exe", NULL);
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
                        ShellExecute(NULL, NULL, "bridgecommand-ini.exe", NULL, NULL, SW_SHOW);
                        //_execl("./bridgecommand-ini.exe", "bridgecommand-ini.exe", NULL);
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
                        ShellExecute(NULL, NULL, "bridgecommand-ini.exe", "-M", NULL, SW_SHOW);
                        //_execl("./bridgecommand-ini.exe", "bridgecommand-ini.exe", "-M", NULL);
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
                        ShellExecute(NULL, NULL, "bridgecommand-ini.exe", "-R", NULL, SW_SHOW);
                        //_execl("./bridgecommand-ini.exe", "bridgecommand-ini.exe", "-R", NULL);
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
                        ShellExecute(NULL, NULL, "bridgecommand-ini.exe", "-H", NULL, SW_SHOW);
                        //_execl("./bridgecommand-ini.exe", "bridgecommand-ini.exe", "-H", NULL);
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
                        //Sleep(5000);
                        //exit(EXIT_SUCCESS);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        execl("/usr/bin/open", "open", "../Resources/doc/index.html", NULL);
                    #else
                        //Other (assumed posix)
                        #ifdef FOR_DEB
                            execl("/usr/bin/xdg-open", "xdg-open", "/usr/share/doc/bridgecommand/index.html", NULL);
                            //If execuation gets to this point, it has failed to launch help. Bring up a message to tell user?
                        #else
                            execl("/usr/bin/xdg-open", "xdg-open", "doc/index.html", NULL);
                            //If execuation gets to this point, it has failed to launch help. Bring up a message to tell user?
                        #endif // FOR_DEB
                    #endif
                    #endif
                }
                if (id == USER_BUTTON) {
                    #ifdef _WIN32
                        //CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
                        ShellExecute(NULL, TEXT("open"), TEXT(userFolder.c_str()), NULL, NULL, SW_SHOWNORMAL);
                        //Sleep(5000);
                        //exit(EXIT_SUCCESS);
                    #else
                    #ifdef __APPLE__
                        //APPLE
                        std::cout << userFolder << std::endl;
                        execl("/usr/bin/open", "open", userFolder.c_str(), NULL);
                    #else
                        //Other (assumed posix)
                        execl("/usr/bin/xdg-open", "xdg-open", userFolder.c_str(), NULL);
                    #endif
                    #endif
                }
            }
        }
        if (event.EventType == irr::EET_KEY_INPUT_EVENT) {
            if (event.KeyInput.Key == irr::KEY_ESCAPE ) {
                exit(EXIT_SUCCESS);
            }
        }
        return false;
    }
};

int main (int argc, char ** argv)
{

    if ((argc>1)&&(strcmp(argv[1],"--version")==0)) {
        std::cout << LONGVERSION << std::endl;
        exit(EXIT_SUCCESS);
    }
    
    #ifdef FOR_DEB
    chdir("/usr/share/bridgecommand");
    #endif // FOR_DEB

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

    //User read/write location - look in here first and the exe folder second for files
    userFolder = Utilities::getUserDir();

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

    int fontSize = FONT_SIZE_DEFAULT;
    float fontScale = IniFile::iniFileTof32(iniFilename, "font_scale");
    if (fontScale > 1) {
        fontSize = (int)(fontSize * fontScale + 0.5);
    } else {
	    fontScale = 1.0;
    }

    irr::u32 graphicsWidth = fontSize * 16;
    irr::u32 graphicsHeight = fontSize * 34;
    irr::u32 graphicsDepth = 32;
    bool fullScreen = false;

    irr::IrrlichtDevice* device = irr::createDevice(irr::video::EDT_OPENGL, irr::core::dimension2d<irr::u32>(graphicsWidth,graphicsHeight),graphicsDepth,fullScreen,false,false,0);
    irr::video::IVideoDriver* driver = device->getVideoDriver();

    #ifdef __APPLE__
    //Mac OS - cd back to original dir - seems to be changed during createDevice
    irr::io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        exit(EXIT_FAILURE); //Could not get file system TODO: Message for user
        std::cout << "Could not get filesystem" << std::endl;
    }
    fileSystem->changeWorkingDirectoryTo(exeFolderPath.c_str());
    #endif

    std::string fontName = IniFile::iniFileToString(iniFilename, "font");
    std::string fontPath = "media/fonts/" + fontName + "/" + fontName + "-" + std::to_string(fontSize) + ".xml";
    irr::gui::IGUIFont *font = device->getGUIEnvironment()->getFont(fontPath.c_str());
    if (font == NULL) {
        std::cout << "Could not load font, using fallback" << std::endl;
    } else {
        //set skin default font
        device->getGUIEnvironment()->getSkin()->setFont(font);
    }

    //Add launcher buttons with layout in viewport due to scaling

    short bC = graphicsWidth / 20;       // padding ...
    short bR = bC / 3 * 2 + 1;

    short bW = graphicsWidth - (2 * bC); // size ...
    short bH = 20 * (fontSize / (FONT_SIZE_DEFAULT * 1.0)) + 1;

    short x1 = bC;                       // location ...
    short x2 = x1 + bW;
    short y1, y2;

    y1 =        bR; y2 = y1 + 2*bH; irr::gui::IGUIButton* launchBC    = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,BC_BUTTON,language.translate("startBC").c_str()); //i18n

    y1 = y2 + 3*bR; y2 = y1 +   bH; irr::gui::IGUIButton* launchED    = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,ED_BUTTON,language.translate("startED").c_str()); //i18n
    y1 = y2 +   bR; y2 = y1 +   bH; irr::gui::IGUIButton* launchMC    = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,MC_BUTTON,language.translate("startMC").c_str()); //i18n
    y1 = y2 +   bR; y2 = y1 +   bH; irr::gui::IGUIButton* launchRP    = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,RP_BUTTON,language.translate("startRP").c_str()); //i18n
    y1 = y2 +   bR; y2 = y1 +   bH; irr::gui::IGUIButton* launchMH    = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,MH_BUTTON,language.translate("startMH").c_str()); //i18n

    y1 = y2 + 3*bR; y2 = y1 +   bH; irr::gui::IGUIButton* launchINIBC = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,INI_BC_BUTTON,language.translate("startINIBC").c_str()); //i18n
    y1 = y2 +   bR; y2 = y1 +   bH; irr::gui::IGUIButton* launchINIMC = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,INI_MC_BUTTON,language.translate("startINIMC").c_str()); //i18n
    y1 = y2 +   bR; y2 = y1 +   bH; irr::gui::IGUIButton* launchINIRP = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,INI_RP_BUTTON,language.translate("startINIRP").c_str()); //i18n
    y1 = y2 +   bR; y2 = y1 +   bH; irr::gui::IGUIButton* launchINIMH = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,INI_MH_BUTTON,language.translate("startINIMH").c_str()); //i18n

    y1 = y2 + 3*bR; y2 = y1 +   bH; irr::gui::IGUIButton* launchDOC   = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,DOC_BUTTON,language.translate("startDOC").c_str()); //i18n
    y1 = y2 +   bR; y2 = y1 +   bH; irr::gui::IGUIButton* launchFOLDER= device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,USER_BUTTON,language.translate("user").c_str()); //i18n
    y1 = y2 +   bR; y2 = y1 +   bH; irr::gui::IGUIButton* leave       = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(x1,y1,x2,y2),0,EXIT_BUTTON,language.translate("leave").c_str()); //i18n
    
    device->getGUIEnvironment()->setFocus(launchBC);

    Receiver receiver;
    device->setEventReceiver(&receiver);

    #ifdef FOR_DEB
    chdir("/usr/bin");
    #endif // FOR_DEB

    while (device->run()) {
        driver->beginScene();
        device->getGUIEnvironment()->drawAll();
        driver->endScene();
        device->sleep(100);
    }

    return EXIT_FAILURE;
}
