#include "irrlicht.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include "PositionDataStruct.hpp"
#include "ShipDataStruct.hpp"
#include "OtherShipDataStruct.hpp"
//#include "Network.hpp"
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

//To do: Utility function to find scenario list
void getDirectoryList(IrrlichtDevice* device, std::vector<std::string>&dirList, std::string path) {

    io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        exit(EXIT_FAILURE); //Could not get file system TODO: Message for user
    }
    //store current dir
    io::path cwd = fileSystem->getWorkingDirectory();

    //change to dir
    if (!fileSystem->changeWorkingDirectoryTo(path.c_str())) {
        exit(EXIT_FAILURE); //Couldn't change to dir
    }

    io::IFileList* fileList = fileSystem->createFileList();
    if (fileList==0) {
        exit(EXIT_FAILURE); //Could not get file list for scenarios TODO: Message for user
    }

    //List here
    for (u32 i=0;i<fileList->getFileCount();i++) {
        if (fileList->isDirectory(i)) {
            const io::path& fileName = fileList->getFileName(i);
            if (fileName.findFirst('.')!=0) { //Check it doesn't start with '.' (., .., or hidden)
                //std::cout << fileName.c_str() << std::endl;
                dirList.push_back(fileName.c_str());
            }
        }
    }

    //change back
    if (!fileSystem->changeWorkingDirectoryTo(cwd)) {
        exit(EXIT_FAILURE); //Couldn't change dir back
    }
    fileList->drop();
}

void findWhatToLoad(IrrlichtDevice* device, std::string& worldName, std::string scenarioName, Lang* language, std::string userFolder)
//Will fill one of worldName of scenarioName, depending on user's selection.
{

    video::IVideoDriver* driver = device->getVideoDriver();

    //Get screen width
    u32 su = driver->getScreenSize().Width;
    u32 sh = driver->getScreenSize().Height;

    //Find list of scenarios, and list of available world models
    std::string scenarioPath = "Scenarios/";
    if (Utilities::pathExists(userFolder + scenarioPath)) {
        scenarioPath = userFolder + scenarioPath;
    }
    std::vector<std::string> scenarioDirList;
    getDirectoryList(device,scenarioDirList,scenarioPath); //Populates scenarioDirList

    std::string worldPath = "World/";
    if (Utilities::pathExists(userFolder + worldPath)) {
        worldPath = userFolder + worldPath;
    }
    std::vector<std::string> worldDirList;
    getDirectoryList(device,worldDirList,worldPath); //Populates worldDirList

    irr::gui::IGUIWindow* scnWorldChoiceWindow = device->getGUIEnvironment()->addWindow(core::rect<s32>(0.01*su, 0.01*sh, 0.99*su, 0.99*sh), false);
    irr::gui::IGUIListBox* scenarioListBox = device->getGUIEnvironment()->addListBox(core::rect<s32>(0.01*su,0.100*sh,0.485*su,0.80*sh),scnWorldChoiceWindow,-1); //TODO: Set ID so we can use event receiver
    irr::gui::IGUIListBox* worldListBox =    device->getGUIEnvironment()->addListBox(core::rect<s32>(0.495*su,0.100*sh,0.970*su,0.80*sh),scnWorldChoiceWindow,-1); //TODO: Set ID so we can use event receiver
    irr::gui::IGUIStaticText* scenarioText = device->getGUIEnvironment()->addStaticText(language->translate("selectScenario").c_str(),core::rect<s32>(0.01*su,0.050*sh,0.485*su,0.090*sh),false,true,scnWorldChoiceWindow);
    irr::gui::IGUIStaticText* worldText = device->getGUIEnvironment()->addStaticText(language->translate("selectWorld").c_str(),core::rect<s32>(0.495*su,0.050*sh,0.970*su,0.090*sh),false,true,scnWorldChoiceWindow);
    scnWorldChoiceWindow->getCloseButton()->setVisible(false);

    //Add scenarios to list box
    for (std::vector<std::string>::iterator it = scenarioDirList.begin(); it != scenarioDirList.end(); ++it) {
        scenarioListBox->addItem(core::stringw(it->c_str()).c_str()); //Fixme - odd conversion from char* to wchar*!
    }
    //select first one if possible
    if (scenarioListBox->getItemCount()>0) {
        scenarioListBox->setSelected(0);
    }
    //Add world to list box
    for (std::vector<std::string>::iterator it = worldDirList.begin(); it != worldDirList.end(); ++it) {
        worldListBox->addItem(core::stringw(it->c_str()).c_str()); //Fixme - odd conversion from char* to wchar*!
    }
    //select first one if possible
    if (worldListBox->getItemCount()>0) {
        worldListBox->setSelected(0);
    }

    //set focus on first box
    device->getGUIEnvironment()->setFocus(scenarioListBox);

    while (device->run() && worldName.size() == 0 && scenarioName.size() == 0) {
        driver->beginScene();
        device->getGUIEnvironment()->drawAll();
        driver->endScene();
        //worldName = "SimpleEstuary"; //Todo: make this a user selection.
    }
    //Clean up
    scenarioText->remove();
    worldText->remove();
    scenarioListBox->remove();
    worldListBox->remove();
    scnWorldChoiceWindow->remove();
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
    //change up from BridgeCommand.app/Contents/MacOS to ../Resources
    exeFolderPath.append("/../Resources");
    //change to this path now, so ini file is read
    chdir(exeFolderPath.c_str());
    //Note, we use this again after the createDevice call
	#endif

    //User read/write location - look in here first and the exe folder second for files
    std::string userFolder = Utilities::getUserDir();

    std::string iniFilename = "map.ini";
    //Use local ini file if it exists
    if (Utilities::pathExists(userFolder + iniFilename)) {
        iniFilename = userFolder + iniFilename;
    }
    u32 graphicsWidth = IniFile::iniFileTou32(iniFilename, "graphics_width");
    u32 graphicsHeight = IniFile::iniFileTou32(iniFilename, "graphics_height");
    u32 graphicsDepth = IniFile::iniFileTou32(iniFilename, "graphics_depth");
    bool fullScreen = (IniFile::iniFileTou32(iniFilename, "graphics_mode")==1); //1 for full screen

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
    std::string languageFile = "languageController.txt"; //TODO: Update when needed
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

    std::string worldName; //Todo: make this a user selection.
    std::string scenarioName; //Todo: make this a user selection.
    findWhatToLoad(device, worldName, scenarioName, &language, userFolder);

    //GUI class
    GUIMain guiMain(device, &language);
    //Main model
    ControllerModel controller(device, &guiMain, worldName);

    //Classes:  Data structures created in main, and shared with controller by reference. Controller then pushes data to the GUI

    //Create data structures to hold own ship, other ship and buoy data
    irr::f32 time = 0; //Time since start of day 1 of the scenario
    ShipData ownShipData;
    std::vector<PositionData> buoysData;
    std::vector<OtherShipData> otherShipsData;

    //TODO: If an existing scenario, load data into this


    //create event receiver, linked to model
    EventReceiver receiver(device, &controller, &guiMain/*, &network*/);
    device->setEventReceiver(&receiver);

    while(device->run()) {

        driver->beginScene();

        //Read in data from network
        //network.update(time, ownShipData, otherShipsData, buoysData);

        //Update the internal model, and call the gui
        controller.update(time, ownShipData, otherShipsData, buoysData);

        driver->endScene();
    }

    return(0);
}
