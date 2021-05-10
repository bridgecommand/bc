#include "irrlicht.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <string>
#include "PositionDataStruct.hpp"
#include "OwnShipDataStruct.hpp"
#include "OtherShipDataStruct.hpp"
#include "GeneralDataStruct.hpp"
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
#include <unistd.h>
#endif //__APPLE__

#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

//Includes for copying scenario files
#ifdef _WIN32
    #include <windows.h>
    #include <Shellapi.h>
#else // _WIN32
    #ifdef __APPLE__
        #include <copyfile.h>
        #include <sys/stat.h>
    #else
        #include <dirent.h>
        #include <sys/stat.h>
        #include <fstream>
    #endif
#endif // __APPLE__

#ifdef __linux__
    #include <unistd.h>
#endif

// Irrlicht Namespaces
//using namespace irr;

//Set up global for ini reader to have access to irrlicht logger if needed.
namespace IniFile {
    irr::ILogger* irrlichtLogger = 0;
}

//To do: Utility function to find scenario list
void getDirectoryList(irr::IrrlichtDevice* device, std::vector<std::string>&dirList, std::string path) {

	irr::io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        std::cout << "Failed to get access to file system" << std::endl;
        exit(EXIT_FAILURE);
    }
    //store current dir
    irr::io::path cwd = fileSystem->getWorkingDirectory();

    //change to dir
    if (!fileSystem->changeWorkingDirectoryTo(path.c_str())) {
        std::cout << "Failed to change to scenario directory" << std::endl;
        exit(EXIT_FAILURE); //Couldn't change to dir
    }

    irr::io::IFileList* fileList = fileSystem->createFileList();
    if (fileList==0) {
        std::cout << "Could not get scenario list" << std::endl;
        exit(EXIT_FAILURE); //Could not get file list for scenarios TODO: Message for user
    }

    //List here
    for (irr::u32 i=0;i<fileList->getFileCount();i++) {
        if (fileList->isDirectory(i)) {
            const irr::io::path& fileName = fileList->getFileName(i);
            if (fileName.findFirst('.')!=0) { //Check it doesn't start with '.' (., .., or hidden)
                //std::cout << fileName.c_str() << std::endl;
                dirList.push_back(fileName.c_str());
            }
        }
    }

    //change back
    if (!fileSystem->changeWorkingDirectoryTo(cwd)) {
        std::cout << "Could not return to normal working directory" << std::endl;
        exit(EXIT_FAILURE); //Couldn't change dir back
    }
    fileList->drop();
}

void findWhatToLoad(irr::IrrlichtDevice* device, std::string& worldName, std::string& scenarioName, bool& multiplayer, Lang* language, std::string userFolder)
//Will fill one of worldName of scenarioName, depending on user's selection.
{

	irr::video::IVideoDriver* driver = device->getVideoDriver();

    //Get screen width
    irr::u32 su = driver->getScreenSize().Width;
    irr::u32 sh = driver->getScreenSize().Height;

    //Find list of scenarios, and list of available world models
    std::string scenarioPath = "Scenarios/";
    if (Utilities::pathExists(userFolder + scenarioPath)) {
        scenarioPath = userFolder + scenarioPath;
    }
    std::vector<std::string> scenarioDirList;
    getDirectoryList(device,scenarioDirList,scenarioPath); //Populates scenarioDirList

    std::string worldPath = "World/";
    std::vector<std::string> worldDirList;
    getDirectoryList(device,worldDirList,worldPath); //Populates worldDirList
    if (Utilities::pathExists(userFolder + worldPath)) {
        worldPath = userFolder + worldPath;
        getDirectoryList(device,worldDirList,worldPath); //Append to worldDirList, for user specific world models
    }


    const irr::s32 SCENARIO_BOX_ID = 101;
    const irr::s32 WORLD_BOX_ID = 102;
    const irr::s32 OK_SCENARIO_BUTTON_ID = 103;
    const irr::s32 OK_WORLD_BUTTON_ID = 104;

    irr::gui::IGUIWindow* scnWorldChoiceWindow = device->getGUIEnvironment()->addWindow(irr::core::rect<irr::s32>(0.01*su, 0.01*sh, 0.99*su, 0.99*sh), false);
    irr::gui::IGUIListBox* scenarioListBox = device->getGUIEnvironment()->addListBox(irr::core::rect<irr::s32>(0.06*su,0.200*sh,0.435*su,0.80*sh),scnWorldChoiceWindow,SCENARIO_BOX_ID); //TODO: Set ID so we can use event receiver
    irr::gui::IGUIListBox* worldListBox =    device->getGUIEnvironment()->addListBox(irr::core::rect<irr::s32>(0.545*su,0.200*sh,0.920*su,0.60*sh),scnWorldChoiceWindow,WORLD_BOX_ID); //TODO: Set ID so we can use event receiver
    irr::gui::IGUICheckBox* multiplayerBox = device->getGUIEnvironment()->addCheckBox(false,irr::core::rect<irr::s32>(0.545*su,0.700*sh,0.920*su,0.730*sh),scnWorldChoiceWindow,-1,language->translate("multiplayer").c_str());
    irr::gui::IGUIStaticText* scenarioText = device->getGUIEnvironment()->addStaticText(language->translate("selectScenario").c_str(),irr::core::rect<irr::s32>(0.035*su,0.150*sh,0.485*su,0.190*sh),false,true,scnWorldChoiceWindow);
    irr::gui::IGUIStaticText* worldText = device->getGUIEnvironment()->addStaticText(language->translate("selectWorld").c_str(),irr::core::rect<irr::s32>(0.520*su,0.150*sh,0.970*su,0.190*sh),false,true,scnWorldChoiceWindow);
    irr::gui::IGUIButton* scenarioOK = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(0.01*su,0.85*sh,0.485*su,0.90*sh),scnWorldChoiceWindow,OK_SCENARIO_BUTTON_ID,language->translate("editScenario").c_str());
    irr::gui::IGUIButton* worldOK = device->getGUIEnvironment()->addButton(irr::core::rect<irr::s32>(0.495*su,0.85*sh,0.970*su,0.90*sh),scnWorldChoiceWindow,OK_WORLD_BUTTON_ID,language->translate("newScenario").c_str());
    scnWorldChoiceWindow->getCloseButton()->setVisible(false);

    //Add scenarios to list box
    for (std::vector<std::string>::iterator it = scenarioDirList.begin(); it != scenarioDirList.end(); ++it) {
        scenarioListBox->addItem(irr::core::stringw(it->c_str()).c_str()); //Fixme - odd conversion from char* to wchar*!
    }
    //select first one if possible
    if (scenarioListBox->getItemCount()>0) {
        scenarioListBox->setSelected(0);
    }
    //Add world to list box
    for (std::vector<std::string>::iterator it = worldDirList.begin(); it != worldDirList.end(); ++it) {
        worldListBox->addItem(irr::core::stringw(it->c_str()).c_str()); //Fixme - odd conversion from char* to wchar*!
    }
    //select first one if possible
    if (worldListBox->getItemCount()>0) {
        worldListBox->setSelected(0);
    }

    //set focus on first box
    //device->getGUIEnvironment()->setFocus(scenarioListBox);

    //Link to our event receiver
    StartupEventReceiver startupReceiver(scenarioListBox,worldListBox,SCENARIO_BOX_ID,WORLD_BOX_ID,OK_SCENARIO_BUTTON_ID,OK_WORLD_BUTTON_ID);
    device->setEventReceiver(&startupReceiver);

    while (device->run() && startupReceiver.getScenarioSelected() < 0 && startupReceiver.getWorldSelected() < 0 ) {
        driver->beginScene();
        device->getGUIEnvironment()->drawAll();
        driver->endScene();
    }

    irr::s32 selectedScenario = startupReceiver.getScenarioSelected();
    if (selectedScenario >= 0 && selectedScenario < scenarioDirList.size()) {
        scenarioName = scenarioDirList.at(selectedScenario); //Get scenario name

        //check if name ends in _mp, and if so, record that this is a multiplayer scenario
        multiplayer = false;
        if (scenarioName.length() >= 3) {
            std::string endChars = scenarioName.substr(scenarioName.length()-3,3);
            if (endChars == "_mp" || endChars == "_MP") {
                multiplayer = true;
            }
        }
    }

    irr::s32 selectedWorld = startupReceiver.getWorldSelected();
    if (selectedWorld >= 0 && selectedWorld < worldDirList.size()) {
        worldName = worldDirList.at(selectedWorld);
        multiplayer = multiplayerBox->isChecked();
    }
    //Store multiplayer status if new scenario

    //Clean up
    scenarioText->remove();
    worldText->remove();
    scenarioListBox->remove();
    worldListBox->remove();
    scenarioOK->remove();
    worldOK->remove();
    multiplayerBox->remove();
    device->setEventReceiver(0);

    //Show patience message
    irr::gui::IGUIStaticText* patienceText = device->getGUIEnvironment()->addStaticText(language->translate("loadingMap").c_str(),irr::core::rect<irr::s32>(0.01*su,0.04*sh,0.95*su,0.95*sh),false,true,scnWorldChoiceWindow);
    if (device->run()) {
        driver->beginScene();
        device->getGUIEnvironment()->drawAll();
        driver->endScene();
    }

    //Finish cleaning up
    patienceText->remove();
    scnWorldChoiceWindow->remove();

}

int copyDir(std::string source, std::string dest)
{

    //Copy contents of source dir into dest dir

    #ifdef _WIN32
        //Windows version: Creates dest dir automatically
        source.append(1,'\0'); //Add an extra null to end of string
        dest.append(1,'\0');
        replace(dest.begin(),dest.end(),'/','\\'); //Replace / with \ in dest (think about network paths??)

        SHFILEOPSTRUCT fileOp;
        fileOp.wFunc = FO_COPY;
        fileOp.pFrom = source.c_str();
        fileOp.pTo = dest.c_str();
        fileOp.fFlags = /*FOF_SILENT | */FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;

        return SHFileOperation(&fileOp);
    #else
        #ifdef __APPLE__
            //Apple version: Requires that dest dir exists
            copyfile_state_t s;
            s=copyfile_state_alloc();
            //use copyfile here to do recursive copy
            int returnValue = copyfile(source.c_str(), dest.c_str(), s, COPYFILE_DATA | COPYFILE_RECURSIVE);
            copyfile_state_free(s);
            return returnValue;
        #else // __APPLE__
            //Other posix
            //Note: Not implemented yet for other posix: need to implement recursive directory copy.
            //Requires that dest dir exists
            //std::cout << "Copying from:" << source << " to:" << dest << std::endl;
            if (!Utilities::pathExists(dest)) {
                return -1;
            }

            //For each folder at root level, create new folder in dest, and call copyDir on this
            DIR *dir = opendir(source.c_str());
            if (!dir) {return -1;}
            struct dirent *entry = readdir(dir);
            while (entry != NULL) {
                if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
                    std::string newDir = dest;

                    newDir.append(source);
                    newDir.append("/");
                    newDir.append(entry->d_name);
                    //newDir.append("/");

                    //std::cout << "Dest: " << dest << std::endl;
                    //std::cout << "Trying to create '" << newDir << "'" << std::endl;
                    if (mkdir(newDir.c_str(),0755)==0) {
                        //Recursive here
                        std::string fromDir = source;
                        fromDir.append("/");
                        fromDir.append(entry->d_name);

                        std::string toDir = dest;

                        copyDir(fromDir, toDir);
                    } else {
                        return -1;
                    }
                } else if (entry->d_type == DT_REG) {
                    //Copy file
                    //entry->d_name;
                    std::string newFile = dest;
                    newFile.append(source);
                    newFile.append("/");
                    newFile.append(entry->d_name);

                    std::string fromFile = source;
                    fromFile.append("/");
                    fromFile.append(entry->d_name);

                    //std::cout << "About to try and create >>" << newFile << "<< from >>" << fromFile << "<<" << std::endl;

                    std::ifstream fromStream(fromFile.c_str(), std::ios::binary);
                    std::ofstream destStream(newFile.c_str(), std::ios::binary);
                    if (fromStream && destStream) {
                        destStream << fromStream.rdbuf();
                    }

                }

                entry = readdir(dir);
            }

            //For each file at root level, create the file and copy contents


        #endif // __APPLE__
    #endif // _WIN32

    return -1;
}

void checkUserScenarioDir(void)
{
    //Check if scenarios are in the user dir, and if not, try to copy in
    std::string userFolder = Utilities::getUserDir();

    std::string scenarioPath = "Scenarios";

    if (!Utilities::pathExists(userFolder + scenarioPath)) {

        #ifdef _WIN32
        std::cout << "Copying scenario files into " << userFolder + scenarioPath << std::endl;
        copyDir("Scenarios", userFolder + scenarioPath);
        #else
        //Make sure destination folder for scenarios exists. Not needed on windows as the copy method creates the output folder and directories above it.
        if (!Utilities::pathExists(Utilities::getUserDirBase())) {
            std::string pathToMake = Utilities::getUserDirBase();
            if (pathToMake.size() > 1) {pathToMake.erase(pathToMake.size()-1);} //Remove trailing slash
            mkdir(pathToMake.c_str(),0755);
        }
        if (!Utilities::pathExists(Utilities::getUserDir())) {
            std::string pathToMake = Utilities::getUserDir();
            if (pathToMake.size() > 1) {pathToMake.erase(pathToMake.size()-1);} //Remove trailing slash
            mkdir(pathToMake.c_str(),0755);
        }
        if (!Utilities::pathExists(Utilities::getUserDir() + "Scenarios/")) {
            std::string pathToMake = Utilities::getUserDir() + "Scenarios";
            mkdir(pathToMake.c_str(),0755);
        }
        std::cout << "Copying scenario files into " << userFolder << std::endl;
        copyDir("Scenarios", userFolder);
        #endif // __APPLE__


    }
}

int main (int argc, char ** argv)
{

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
    //change up from BridgeCommand.app/Contents/MacOS/ed.app/Contents/MacOS to BridgeCommand.app/Contents/Resources
    exeFolderPath.append("/../../../../Resources");
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

    int fontSize = 12;
    float fontScale = IniFile::iniFileTof32(iniFilename, "font_scale");
    if (fontScale > 1) {
        fontSize = (int)(fontSize * fontScale + 0.5);
    } else {
	    fontScale = 1.0;
    }

    irr::u32 graphicsWidth = IniFile::iniFileTou32(iniFilename, "graphics_width");
    irr::u32 graphicsHeight = IniFile::iniFileTou32(iniFilename, "graphics_height");
    irr::u32 graphicsDepth = IniFile::iniFileTou32(iniFilename, "graphics_depth");
    bool fullScreen = (IniFile::iniFileTou32(iniFilename, "graphics_mode")==1); //1 for full screen

    irr::IrrlichtDevice *nulldevice = irr::createDevice(irr::video::EDT_NULL);
	irr::core::dimension2d<irr::u32> deskres = nulldevice->getVideoModeList()->getDesktopResolution();
	nulldevice->drop();
    if (graphicsWidth==0) {
        graphicsWidth = 1200 * fontScale;
        if (graphicsWidth > deskres.Width*0.9) {
            graphicsWidth = deskres.Width*0.9;
        }
    }
    if (graphicsHeight==0) {
        graphicsHeight = 900 * fontScale;
        if (graphicsHeight > deskres.Height*0.9) {
            graphicsHeight = deskres.Height*0.9;
        }
    }

	irr::u32 zoomLevels = IniFile::iniFileTou32(iniFilename, "zoom_levels");
	if (zoomLevels == 0) {
		zoomLevels = 10;
	}

    irr::IrrlichtDevice* device = irr::createDevice(irr::video::EDT_OPENGL, irr::core::dimension2d<irr::u32>(graphicsWidth,graphicsHeight),graphicsDepth,fullScreen,false,false,0);
    irr::video::IVideoDriver* driver = device->getVideoDriver();
    //scene::ISceneManager* smgr = device->getSceneManager();

    std::string fontName = IniFile::iniFileToString(iniFilename, "font");
    std::string fontPath = "media/fonts/" + fontName + "/" + fontName + "-" + std::to_string(fontSize) + ".xml";
    irr::gui::IGUIFont *font = device->getGUIEnvironment()->getFont(fontPath.c_str());
    if (font == NULL) {
        std::cout << "Could not load font, using fallback" << std::endl;
    } else {
        //set skin default font
        device->getGUIEnvironment()->getSkin()->setFont(font);
    }

    #ifdef __APPLE__
    //Mac OS - cd back to original dir - seems to be changed during createDevice
    irr::io::IFileSystem* fileSystem = device->getFileSystem();
    if (fileSystem==0) {
        std::cout << "Could not get filesystem" << std::endl;
        exit(EXIT_FAILURE); //Could not get file system TODO: Message for user
    }
    fileSystem->changeWorkingDirectoryTo(exeFolderPath.c_str());
    #endif

    //load language
    std::string modifier = IniFile::iniFileToString(iniFilename, "lang");
    if (modifier.length()==0) {
        modifier = "en"; //Default
    }
    std::string languageFile = "languageController-";
    languageFile.append(modifier);
    languageFile.append(".txt");
    if (Utilities::pathExists(userFolder + languageFile)) {
        languageFile = userFolder + languageFile;
    }
    Lang language(languageFile);

	//Check if user scenario dir exists. If not, try to copy scenarios into the user dir.
    checkUserScenarioDir();

    //Flush old key/clicks etc, with a 0.2s pause
    device->sleep(200);
    device->clearSystemMessages();

    //Query which scenario or world to start with
    std::string worldName;
    std::string scenarioName;
    bool multiplayer;
    findWhatToLoad(device, worldName, scenarioName, multiplayer, &language, userFolder); //worldName or scenarioName updated by reference
    //check that one of worldName and scenarioName have been set
    if (worldName.length() == 0 && scenarioName.length() == 0) {
        std::cout << "Failed to select a scenario or world model to use" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (multiplayer) {
        std::cout << "Multiplayer mode" << std::endl;
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

    //Get a list of available boat models (own and other ships)
    std::vector<std::string> ownShipTypes;
    std::vector<std::string> otherShipTypes;

    std::string otherShipModelPath;
    std::string ownShipModelPath = "Models/Ownship/";
    if (multiplayer) {
        otherShipModelPath = "Models/Ownship/"; //If in multiplayer mode, use own ship list for both own and others
    } else {
        otherShipModelPath = "Models/Othership/";
    }

    getDirectoryList(device,ownShipTypes,ownShipModelPath);
    if (Utilities::pathExists(userFolder + ownShipModelPath)) {
    	ownShipModelPath = userFolder + ownShipModelPath;
    	getDirectoryList(device,ownShipTypes,ownShipModelPath); //Append models from userFolder
    }

    getDirectoryList(device,otherShipTypes,otherShipModelPath);
    if (Utilities::pathExists(userFolder + otherShipModelPath)) {
    	otherShipModelPath = userFolder + otherShipModelPath;
    	getDirectoryList(device,otherShipTypes,otherShipModelPath); //Append models from userFolder
    }

    //GUI class
    GUIMain guiMain(device, &language, ownShipTypes, otherShipTypes, multiplayer);

    //Classes:  Data structures created in main, and shared with controller by pointer. Controller then pushes data to the GUI

    //Create data structures to hold own ship, other ship and buoy data
    GeneralData generalData;
    OwnShipEditorData ownShipData;
    std::vector<PositionData> buoysData;
    std::vector<OtherShipEditorData> otherShipsData;

    //Change default scenario name if in multiplayer mode
    if (multiplayer) {
        generalData.scenarioName.append("_mp");
    }

    //Make default name the first in the list, in case it isn't set by an update later
    if (ownShipTypes.size() > 0) {
        ownShipData.name = ownShipTypes.at(0);
    }
    if (otherShipTypes.size() > 0) {
        for (int i=0; i<otherShipsData.size(); i++)
        otherShipsData.at(i).name = otherShipTypes.at(0);
    }

    //Main model
    ControllerModel controller(device, &language, &guiMain, worldName, &ownShipData, &otherShipsData, &buoysData, &generalData, zoomLevels);

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

        std::string descriptionFilename = scenarioPath;
        descriptionFilename.append("/description.ini");

        //Load general information
        generalData.startTime = SECONDS_IN_HOUR * IniFile::iniFileTof32(environmentIniFilename,"StartTime"); //Time since start of day
        generalData.startDay = IniFile::iniFileTou32(environmentIniFilename,"StartDay");
        generalData.startMonth = IniFile::iniFileTou32(environmentIniFilename,"StartMonth");
        generalData.startYear = IniFile::iniFileTou32(environmentIniFilename,"StartYear");
        generalData.sunRiseTime = IniFile::iniFileTof32(environmentIniFilename,"SunRise");
        generalData.sunSetTime = IniFile::iniFileTof32(environmentIniFilename,"SunSet");
        generalData.weather = IniFile::iniFileTof32(environmentIniFilename,"Weather");
        generalData.visibility = IniFile::iniFileTof32(environmentIniFilename,"VisibilityRange");
        generalData.rain = IniFile::iniFileTof32(environmentIniFilename,"Rain");
        generalData.scenarioName = scenarioName;
        //defaults
        if(generalData.sunRiseTime==0.0) {generalData.sunRiseTime=6;}
        if(generalData.sunSetTime==0.0) {generalData.sunSetTime=18;}

        //Load own ship information
        ownShipData.X = controller.longToX(IniFile::iniFileTof32(ownShipIniFilename,"InitialLong"));
        ownShipData.Z = controller.latToZ(IniFile::iniFileTof32(ownShipIniFilename,"InitialLat"));
        ownShipData.heading = IniFile::iniFileTof32(ownShipIniFilename,"InitialBearing");
        ownShipData.name = IniFile::iniFileToString(ownShipIniFilename,"ShipName");
        ownShipData.initialSpeed = IniFile::iniFileTof32(ownShipIniFilename,"InitialSpeed");

        //Load other ship information
        int numberOfOtherShips = IniFile::iniFileTou32(otherShipIniFilename,"Number");
        for(irr::u32 i=1;i<=numberOfOtherShips;i++) {

            //Temporary structure to load data
            OtherShipEditorData thisShip;

            //Get initial position
            thisShip.X = controller.longToX(IniFile::iniFileTof32(otherShipIniFilename,IniFile::enumerate1("InitLong",i)));
            thisShip.Z = controller.latToZ(IniFile::iniFileTof32(otherShipIniFilename,IniFile::enumerate1("InitLat",i)));
            thisShip.name = IniFile::iniFileToString(otherShipIniFilename,IniFile::enumerate1("Type",i));
            thisShip.mmsi = IniFile::iniFileTou32(otherShipIniFilename,IniFile::enumerate1("mmsi",i));
            int numberOfLegs = IniFile::iniFileTof32(otherShipIniFilename,IniFile::enumerate1("Legs",i));

            irr::f32 legStartTime = generalData.startTime; //Legs start at the start of the scenario
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

        //Load description information
        std::ifstream descriptionStream (descriptionFilename.c_str());
        //Set UTF-8 on Linux/OSX etc
        #ifndef _WIN32
            try {
        #  ifdef __APPLE__
                char* thisLocale = setlocale(LC_ALL, "");
                if (thisLocale) {
                    descriptionStream.imbue(std::locale(thisLocale));
                }
        #  else
                descriptionStream.imbue(std::locale("en_US.UTF8"));
        #  endif
            } catch (const std::runtime_error& runtimeError) {
                descriptionStream.imbue(std::locale(""));
            }
        #endif

        std::string descriptionLines = "";
        if (descriptionStream.is_open()) {
            std::string descriptionLine;
            while ( std::getline (descriptionStream,descriptionLine) )
            {
                descriptionLines.append(descriptionLine);
                descriptionLines.append("\n");
            }
            descriptionStream.close();
        }
        generalData.description = descriptionLines;

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
    irr::u32 numberOfBuoys;
    numberOfBuoys = IniFile::iniFileTou32(scenarioBuoyFilename,"Number");
    for(irr::u32 currentBuoy=1;currentBuoy<=numberOfBuoys;currentBuoy++) {

        PositionData thisBuoy;
        //Get buoy position
        thisBuoy.X = controller.longToX(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Long",currentBuoy)));
        thisBuoy.Z = controller.latToZ(IniFile::iniFileTof32(scenarioBuoyFilename,IniFile::enumerate1("Lat",currentBuoy)));
        buoysData.push_back(thisBuoy);
    }

    //Check if pre-set scenario name will cause an overwrite when saved
    controller.checkName();

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
