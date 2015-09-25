#include "irrlicht.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include "PositionDataStruct.hpp"
#include "ShipDataStruct.hpp"
#include "OtherShipDataStruct.hpp"
#include "StartupEventReceiver.hpp"
//#include "Network.hpp"
#include "ControllerModel.hpp"
#include "GUI.hpp"
#include "EventReceiver.hpp"

#include "../Constants.hpp"
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

void findWhatToLoad(IrrlichtDevice* device, std::string& worldName, std::string& scenarioName, Lang* language, std::string userFolder)
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

    const irr::s32 SCENARIO_BOX_ID = 101;
    const irr::s32 WORLD_BOX_ID = 102;
    const irr::s32 OK_SCENARIO_BUTTON_ID = 103;
    const irr::s32 OK_WORLD_BUTTON_ID = 104;

    irr::gui::IGUIWindow* scnWorldChoiceWindow = device->getGUIEnvironment()->addWindow(core::rect<s32>(0.01*su, 0.01*sh, 0.99*su, 0.99*sh), false);
    irr::gui::IGUIListBox* scenarioListBox = device->getGUIEnvironment()->addListBox(core::rect<s32>(0.01*su,0.100*sh,0.485*su,0.80*sh),scnWorldChoiceWindow,SCENARIO_BOX_ID); //TODO: Set ID so we can use event receiver
    irr::gui::IGUIListBox* worldListBox =    device->getGUIEnvironment()->addListBox(core::rect<s32>(0.495*su,0.100*sh,0.970*su,0.80*sh),scnWorldChoiceWindow,WORLD_BOX_ID); //TODO: Set ID so we can use event receiver
    irr::gui::IGUIStaticText* scenarioText = device->getGUIEnvironment()->addStaticText(language->translate("selectScenario").c_str(),core::rect<s32>(0.01*su,0.050*sh,0.485*su,0.090*sh),false,true,scnWorldChoiceWindow);
    irr::gui::IGUIStaticText* worldText = device->getGUIEnvironment()->addStaticText(language->translate("selectWorld").c_str(),core::rect<s32>(0.495*su,0.050*sh,0.970*su,0.090*sh),false,true,scnWorldChoiceWindow);
    irr::gui::IGUIButton* scenarioOK = device->getGUIEnvironment()->addButton(core::rect<s32>(0.01*su,0.85*sh,0.485*su,0.90*sh),scnWorldChoiceWindow,OK_SCENARIO_BUTTON_ID,language->translate("editScenario").c_str());
    irr::gui::IGUIButton* worldOK = device->getGUIEnvironment()->addButton(core::rect<s32>(0.495*su,0.85*sh,0.970*su,0.90*sh),scnWorldChoiceWindow,OK_WORLD_BUTTON_ID,language->translate("newScenario").c_str());
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

    //Link to our event receiver
    StartupEventReceiver startupReceiver(scenarioListBox,worldListBox,SCENARIO_BOX_ID,WORLD_BOX_ID,OK_SCENARIO_BUTTON_ID,OK_WORLD_BUTTON_ID);
    device->setEventReceiver(&startupReceiver);

    while (device->run() && startupReceiver.getScenarioSelected() < 0 && startupReceiver.getWorldSelected() < 0 ) {
        driver->beginScene();
        device->getGUIEnvironment()->drawAll();
        driver->endScene();
    }

    //Clean up
    scenarioText->remove();
    worldText->remove();
    scenarioListBox->remove();
    worldListBox->remove();
    scenarioOK->remove();
    worldOK->remove();
    scnWorldChoiceWindow->remove();

    irr::s32 selectedScenario = startupReceiver.getScenarioSelected();
    if (selectedScenario >= 0 && selectedScenario < scenarioDirList.size()) {
        scenarioName = scenarioDirList.at(selectedScenario);
    }

    irr::s32 selectedWorld = startupReceiver.getWorldSelected();
    if (selectedWorld >= 0 && selectedWorld < worldDirList.size()) {
        worldName = worldDirList.at(selectedWorld);
    }
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

    //Query which scenario or world to start with
    std::string worldName;
    std::string scenarioName;
    findWhatToLoad(device, worldName, scenarioName, &language, userFolder); //worldName or scenarioName updated by reference
    //check that one of worldName and scenarioName have been set
    if (worldName.length() == 0 && scenarioName.length() == 0) {
        std::cout << "Failed to select a scenario or world model to use" << std::endl;
        exit(EXIT_FAILURE);
    }

    //if worldName isn't set, we need to find it from the scenario.
    if (worldName.length() == 0) {

        //Find scenario path
        std::string scenarioPath = "Scenarios/";
        if (Utilities::pathExists(userFolder + scenarioPath)) {
            scenarioPath = userFolder + scenarioPath;
        }
        scenarioPath.append(scenarioName);

        std::string environmentIniFilename = scenarioPath;
        environmentIniFilename.append("/environment.ini");
        worldName = IniFile::iniFileToString(environmentIniFilename,"Setting");

        if (worldName.length() == 0) {
            std::cout << "Could not find world model name from scenario file: " << environmentIniFilename << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    //GUI class
    GUIMain guiMain(device, &language);

    //Classes:  Data structures created in main, and shared with controller by pointer. Controller then pushes data to the GUI

    //Create data structures to hold own ship, other ship and buoy data
    irr::f32 time = 0; //Time since start of day 1 of the scenario
    ShipData ownShipData;
    std::vector<PositionData> buoysData;
    std::vector<OtherShipData> otherShipsData;

    //Main model
    ControllerModel controller(device, &guiMain, worldName, &ownShipData, &otherShipsData, &buoysData, &time);

    //If an existing scenario, load data into these structures
    if(scenarioName.length() != 0) {
        //Find scenario path
        std::string scenarioPath = "Scenarios/";
        if (Utilities::pathExists(userFolder + scenarioPath)) {
            scenarioPath = userFolder + scenarioPath;
        }
        scenarioPath.append(scenarioName);

        //Need to read in ownship.ini, othership.ini, environment.ini
        std::string environmentIniFilename = scenarioPath;
        environmentIniFilename.append("/environment.ini");

        std::string ownShipIniFilename = scenarioPath;
        ownShipIniFilename.append("/ownship.ini");

        std::string otherShipIniFilename = scenarioPath;
        otherShipIniFilename.append("/othership.ini");

        //Load general information
        time = SECONDS_IN_HOUR * IniFile::iniFileTof32(environmentIniFilename,"StartTime"); //Time since start of day

        //Load own ship information
        ownShipData.X = controller.longToX(IniFile::iniFileTof32(ownShipIniFilename,"InitialLong"));
        ownShipData.Z = controller.latToZ(IniFile::iniFileTof32(ownShipIniFilename,"InitialLat"));
        ownShipData.heading = IniFile::iniFileTof32(ownShipIniFilename,"InitialBearing");

        //Load other ship information

        int numberOfOtherShips = IniFile::iniFileTou32(otherShipIniFilename,"Number");
        for(u32 i=1;i<=numberOfOtherShips;i++) {

            //Temporary structure to load data
            OtherShipData thisShip;

            //Get initial position
            thisShip.X = controller.longToX(IniFile::iniFileTof32(otherShipIniFilename,IniFile::enumerate1("InitLong",i)));
            thisShip.Z = controller.latToZ(IniFile::iniFileTof32(otherShipIniFilename,IniFile::enumerate1("InitLat",i)));
            thisShip.name = IniFile::iniFileToString(otherShipIniFilename,IniFile::enumerate1("Type",i));
            int numberOfLegs = IniFile::iniFileTof32(otherShipIniFilename,IniFile::enumerate1("Legs",i));

            irr::f32 legStartTime = time; //Legs start at the start of the scenario
            for(irr::u32 currentLegNo=1; currentLegNo<=numberOfLegs; currentLegNo++){
                //go through each leg (if any), and load
                Leg currentLeg;
                currentLeg.bearing = IniFile::iniFileTof32(otherShipIniFilename,IniFile::enumerate2("Bearing",i,currentLegNo));
                currentLeg.speed = IniFile::iniFileTof32(otherShipIniFilename,IniFile::enumerate2("Speed",i,currentLegNo));
                currentLeg.startTime = legStartTime;

                //Use distance to calculate startTime of next leg, and stored for later reference.
                irr::f32 distance = IniFile::iniFileTof32(otherShipIniFilename,IniFile::enumerate2("Distance",i,currentLegNo));
                currentLeg.distance = distance;

                //Add the leg to the array
                thisShip.legs.push_back(currentLeg);

                //find the start time for the next leg
                legStartTime = legStartTime + SECONDS_IN_HOUR*(distance/fabs(currentLeg.speed)); // nm/kts -> hours, so convert to seconds
            }
            //add a final 'stop' leg, which the ship will remain on after it has passed the other legs.
            Leg stopLeg;
            stopLeg.bearing=0;
            stopLeg.speed=0;
            stopLeg.distance=0;
            stopLeg.startTime = legStartTime;
            thisShip.legs.push_back(stopLeg);

            //Add to array.
            otherShipsData.push_back(thisShip);
        }

    }

    //Load buoy data
    //construct path to world model
    std::string worldPath = "World/";
    worldPath.append(worldName);
    //Check if this world model exists in the user dir.
    if (Utilities::pathExists(userFolder + worldPath)) {
        worldPath = userFolder + worldPath;
    }
    std::string scenarioBuoyFilename = worldPath;
    scenarioBuoyFilename.append("/buoy.ini");
    //Find number of buoys
    u32 numberOfBuoys;
    numberOfBuoys = IniFile::iniFileTou32(scenarioBuoyFilename,"Number");
    for(u32 currentBuoy=1;currentBuoy<=numberOfBuoys;currentBuoy++) {

        PositionData thisBuoy;
        //Get buoy position
        thisBuoy.X = controller.longToX(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Long",currentBuoy)));
        thisBuoy.Z = controller.latToZ(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Lat",currentBuoy)));
        buoysData.push_back(thisBuoy);
    }

    //create event receiver, linked to model
    EventReceiver receiver(device, &controller, &guiMain/*, &network*/);
    device->setEventReceiver(&receiver);

    while(device->run()) {

        driver->beginScene();

        //Read in data from network
        //network.update(time, ownShipData, otherShipsData, buoysData);

        //Update the internal model, and call the gui
        controller.update();

        driver->endScene();
    }

    return(0);
}
