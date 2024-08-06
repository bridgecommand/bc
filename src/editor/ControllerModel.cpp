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

#include "ControllerModel.hpp"
#include "../IniFile.hpp"
#include "../Constants.hpp"
#include "../Utilities.hpp"
#include <iostream>
#include <fstream>
#ifdef _WIN32
#include <direct.h> //for windows _mkdir
#else
#include <sys/stat.h>
#endif // _WIN32

#define MAX_PX_IN_MAP 160000000

//Constructor
ControllerModel::ControllerModel(irr::IrrlichtDevice* device, Lang* lang, GUIMain* gui, std::string worldName, ScenarioData* scenarioData, std::vector<PositionData>* buoysData, irr::u32 _zoomLevels)
{

    this->gui = gui;
    this->lang = lang;
    this->device = device;
    driver = device->getVideoDriver();

    this->buoysData = buoysData;
    this->scenarioData = scenarioData;
    this->worldName = worldName;
	this->zoomLevels = _zoomLevels;

    checkName();//Check if the scenario name (preset in generalData) will cause an overwrite, and if so, set flag in generalData

    unscaledMap = 0;
    for (unsigned int i = 0; i<zoomLevels; i++) {
		std::cout << "Adding null scaled map levels" << std::endl;
		scaledMap.push_back(0);
		metresPerPx.push_back(0);
    }
	currentZoom = 3;


    mapOffsetX = 0;
    mapOffsetZ = 0;

    mouseDown = false;
    mouseClickedLastUpdate = false;

    selectedShip = -1; //Used to signify own ship selected
    selectedLeg = -1; //Used to signify no leg selected

    //construct path to world model
    std::string worldPath = "World/";
    worldPath.append(worldName);

    //Check if this world model exists in the user dir.
    std::string userFolder = Utilities::getUserDir();
    if (Utilities::pathExists(userFolder + worldPath)) {
        worldPath = userFolder + worldPath;
    }

    std::string worldTerrainFile = worldPath;
    worldTerrainFile.append("/terrain.ini");

    //If terrain.ini doesn't exist, look for *.bin and *.hdr
    bool usingHdrFileOnly = false;
    if (!Utilities::pathExists(worldTerrainFile)) {
        irr::io::IFileSystem* fileSystem = device->getFileSystem();
        
        //store current dir
        irr::io::path cwd = fileSystem->getWorkingDirectory();

        //change to scenario dir
        if (fileSystem->changeWorkingDirectoryTo(worldPath.c_str())) {
            irr::io::IFileList* fileList = fileSystem->createFileList();
            if (fileList!=0) {
                //List here
                for (irr::u32 i=0;i<fileList->getFileCount();i++) {
                    if (fileList->isDirectory(i)==false) {
                        const irr::io::path& fileName = fileList->getFileName(i);
                        if (irr::core::hasFileExtension(fileName,"hdr")) {
                            //Found a .hdr file for us to use
                            
                            worldTerrainFile = worldPath;
                            worldTerrainFile.append("/");
                            worldTerrainFile.append(fileName.c_str());
                            usingHdrFileOnly = true;
                        }
                    }
                }
            }
        }

        //Change dir back
        fileSystem->changeWorkingDirectoryTo(cwd);
    }

    std::string displayMapName;
    if (usingHdrFileOnly) {
        //load from 3dem header format
        terrainLong = IniFile::iniFileTof32(worldTerrainFile, "left_map_x");
        terrainLat = IniFile::iniFileTof32(worldTerrainFile, "lower_map_y");
        terrainLongExtent = IniFile::iniFileTof32(worldTerrainFile, "right_map_x")-terrainLong;
        terrainLatExtent = IniFile::iniFileTof32(worldTerrainFile, "upper_map_y")-terrainLat;

        //assume map is the same path, with .hdr replaced with .bmp
        displayMapName = std::string(device->getFileSystem()->getFileBasename(worldTerrainFile.c_str(),false).append(".bmp").c_str());


    } else {
        //Normal terrain.ini loading
        //Fixme: Note we're only loading the primary terrain here
        terrainLong = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLong",1));
        terrainLat = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLat",1));
        terrainLongExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLongExtent",1));
        terrainLatExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLatExtent",1));

        displayMapName = IniFile::iniFileToString(worldTerrainFile, "MapImage");
        if (displayMapName.empty())
            {displayMapName = IniFile::iniFileToString(worldTerrainFile, "RadarImage");} //Fall back to 'RadarImage' if 'MapImage' parameter isn't set

        //If still empty, we don't have an image to load, so fail here
        if (displayMapName.empty()) {
            std::cout << "Could not load name of map image from ini file: " <<  worldTerrainFile << " (please check this file exists and has not been corrupted)." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    //If the first height map has a .hdr extension, use this to get terrainLong etc
    std::string heightMapFile = IniFile::iniFileToString(worldTerrainFile, "HeightMap(1)");
    std::string extension = "";
    if (heightMapFile.length() > 3) {
        extension = heightMapFile.substr(heightMapFile.length() - 4,4);
        Utilities::to_lower(extension);
    }
    if (extension.compare(".hdr") == 0 ) {
        //Find full path of .hdr file
        std::string hdrPath = worldPath;
        hdrPath.append("/");
        hdrPath.append(heightMapFile);

        terrainLong = IniFile::iniFileTof32(hdrPath, "left_map_x");
        terrainLat = IniFile::iniFileTof32(hdrPath, "lower_map_y");
        terrainLongExtent = IniFile::iniFileTof32(hdrPath, "right_map_x")-terrainLong;
        terrainLatExtent = IniFile::iniFileTof32(hdrPath, "upper_map_y")-terrainLat;
    }

    //Load map image if possible (if not, end with error)
    std::string displayMapPath = worldPath;
    displayMapPath.append("/");
    displayMapPath.append(displayMapName);
    unscaledMap = driver->createImageFromFile(displayMapPath.c_str());
    if (unscaledMap==0) {
        std::cout << "Could not load map image for " << worldPath << std::endl;
        exit(EXIT_FAILURE);
    }

    //Scale map to the appropriate size
    //Terrain dimensions in metres
    terrainXWidth = terrainLongExtent * 2.0 * PI * EARTH_RAD_M * cos( irr::core::degToRad(terrainLat + terrainLatExtent/2.0)) / 360.0;
    terrainZWidth = terrainLatExtent  * 2.0 * PI * EARTH_RAD_M / 360;

    std::cout << "Width m " << terrainXWidth << " Height m " << terrainZWidth << std::endl;

    //Calculate ratio required
    irr::f32 widthToHeight;
    if (terrainXWidth>0 &&  terrainZWidth>0) {
        widthToHeight = terrainXWidth/terrainZWidth;
    } else {
        std::cout << "Zero map width or height. Please check world model." << std::endl;
        exit(EXIT_FAILURE);
    }
    irr::core::dimension2d<irr::u32> loadedSize = unscaledMap->getDimension();

    std::cout << "Loaded map image (" << loadedSize.Width << "x" << loadedSize.Height << ")" << std::endl;

    //Calculate scaling needed
    for (unsigned int i = 0; i<zoomLevels; i++) {
         irr::f32 scaling = 0.00625 * pow(2, i);

         irr::u32 requiredWidth = terrainXWidth*scaling;//std::max(widthFromHeight, loadedSize.Width);
         irr::u32 requiredHeight = terrainZWidth*scaling;//std::max(heightFromWidth, loadedSize.Height);

        //Avoid zero sized map
        if (requiredWidth < 1) {
            requiredWidth = 1;
        }
        if (requiredHeight < 1) {
            requiredHeight = 1;
        }

		 if (requiredHeight * requiredWidth < MAX_PX_IN_MAP) {

			 //Create scaled map with the same image format of the size required
			 std::cout << "About to create empty scaled map " << i << " (" << requiredWidth << "x" << requiredHeight << ")" << std::endl;
			 scaledMap.at(i) = driver->createImage(unscaledMap->getColorFormat(), irr::core::dimension2d<irr::u32>(requiredWidth, requiredHeight));
			 //Copy and scale image
			 std::cout << "About to copy in for " << i << std::endl;
			 //TODO: Check if empty scaled map has been created
			 unscaledMap->copyToScaling(scaledMap.at(i));

			 //Save scale
			 metresPerPx.at(i) = terrainXWidth / requiredWidth;
		 }
		 else {
			 //Don't try to create image
			 zoomLevels = i;
		 }
		 if (currentZoom >= zoomLevels) {
			 currentZoom = zoomLevels - 1;
		 }
		 if (currentZoom < 0) {
			 currentZoom = 0;
		 }
    }

    //Drop the unscaled map, as we don't need this again
    unscaledMap->drop();
    unscaledMap=0;

}

//Destructor
ControllerModel::~ControllerModel()
{
    for (unsigned int i = 0; i<zoomLevels; i++) {
        scaledMap.at(i)->drop();
    }
}

irr::f32 ControllerModel::longToX(irr::f32 longitude) const
{
    return ((longitude - terrainLong ) * (terrainXWidth)) / terrainLongExtent;
}

irr::f32 ControllerModel::latToZ(irr::f32 latitude) const
{
    return ((latitude - terrainLat ) * (terrainZWidth)) / terrainLatExtent;
}

irr::f32 ControllerModel::xToLong(irr::f32 x) const
{
    return terrainLong + x*terrainLongExtent/terrainXWidth;
}

irr::f32 ControllerModel::zToLat(irr::f32 z) const
{
    return terrainLat + z*terrainLatExtent/terrainZWidth;
}

void ControllerModel::update()
{
    //std::cout << mouseDown << std::endl;

    //Check if current zoom is valid, if not return.
    if(!(currentZoom<zoomLevels)) {
        return;
    }

    //Find mouse position change if clicked, and clicked last time
    if (mouseClickedLastUpdate && mouseDown) {
        irr::core::position2d<irr::s32> mouseNow = device->getCursorControl()->getPosition();
        irr::s32 mouseDeltaX = mouseNow.X - mouseLastPosition.X;
        irr::s32 mouseDeltaY = mouseNow.Y - mouseLastPosition.Y;

        //Change offset
        mapOffsetX += mouseDeltaX;
        mapOffsetZ += mouseDeltaY;
    }
    //Update mouse position and clicked state
    mouseLastPosition = device->getCursorControl()->getPosition();
    mouseClickedLastUpdate = mouseDown;


    //TODO: Work out the required area of the map image, and create this as a texture to go to the gui
    irr::core::dimension2d<irr::u32> screenSize = device->getVideoDriver()->getScreenSize();
    //grab an area this size from the scaled map
    irr::video::IImage* tempImage = driver->createImage(scaledMap.at(currentZoom)->getColorFormat(),screenSize); //Empty image
    tempImage->fill(irr::video::SColor(255,0,0,32)); //Initialise background

    //Copy in data
    irr::s32 topLeftX = -1*scenarioData->ownShipData.initialX/metresPerPx.at(currentZoom) + driver->getScreenSize().Width/2 + mapOffsetX;
    irr::s32 topLeftZ = scenarioData->ownShipData.initialZ/metresPerPx.at(currentZoom)    + driver->getScreenSize().Height/2 - scaledMap.at(currentZoom)->getDimension().Height + mapOffsetZ;

    scaledMap.at(currentZoom)->copyTo(tempImage,irr::core::position2d<irr::s32>(topLeftX,topLeftZ)); //Fixme: Check bounds are reasonable

    //Drop any previous textures
    for(irr::u32 i = 0; i < driver->getTextureCount(); i++) {
        if (driver->getTextureByIndex(i)->getName().getPath()=="DisplayTexture") {
            driver->removeTexture(driver->getTextureByIndex(i));
        }
    }

    //Make a texture - The name is required to remove the texture from memory.
    irr::video::ITexture* displayMapTexture = driver->addTexture("DisplayTexture", tempImage);

    tempImage->drop();

    //Send the current data to the gui, and update it
    gui->updateGuiData(*scenarioData,mapOffsetX,mapOffsetZ,metresPerPx.at(currentZoom),*buoysData,displayMapTexture,selectedShip,selectedLeg, terrainLong, terrainLongExtent, terrainXWidth, terrainLat, terrainLatExtent, terrainZWidth);
}

void ControllerModel::resetOffset()
{
    mapOffsetX = 0;
    mapOffsetZ = 0;
}

void ControllerModel::increaseZoom()
{
    if(currentZoom+1<zoomLevels) {
        currentZoom++;

        irr::f32 scaleChange = (irr::f32)scaledMap.at(currentZoom)->getDimension().Width/(irr::f32)scaledMap.at(currentZoom-1)->getDimension().Width;
        mapOffsetX*=scaleChange;
        mapOffsetZ*=scaleChange;
    }
}

void ControllerModel::decreaseZoom()
{
    if(currentZoom>0) {
        currentZoom--;

        irr::f32 scaleChange = (irr::f32)scaledMap.at(currentZoom)->getDimension().Width/(irr::f32)scaledMap.at(currentZoom+1)->getDimension().Width;
        mapOffsetX*=scaleChange;
        mapOffsetZ*=scaleChange;
    }
}

void ControllerModel::setShipPosition(irr::s32 ship, irr::core::vector2df position)
{
    if (ship==0) {
        //Own ship
        scenarioData->ownShipData.initialX = position.X;
        scenarioData->ownShipData.initialZ = position.Y;
    } else if (ship>0) {
        //Other ship
        irr::s32 otherShipNumber = ship-1; //Ship number is minimum of 1, so subtract 1 to start at 0
        if (otherShipNumber < scenarioData->otherShipsData.size()) {
            scenarioData->otherShipsData.at(otherShipNumber).initialX = position.X;
            scenarioData->otherShipsData.at(otherShipNumber).initialZ = position.Y;
        }
    }
}

void ControllerModel::updateSelectedShip(irr::s32 index) //To be called from eventReceiver, where index is from the combo box
{
    if(index < 1) { //If 0 or -1
        selectedShip = -1; //Own ship
    } else {
        selectedShip = index-1; //Other ship number
    }

    //No guarantee from this that the selected ship is valid
}

void ControllerModel::updateSelectedLeg(irr::s32 index) //To be called from eventReceiver, where index is from the combo box. -1 if nothing selected, 0 upwards for leg
{
    selectedLeg = index;
    //No guarantee from this that the selected leg is valid
}

void ControllerModel::setGeneralScenarioData(ScenarioData newData)
{
    scenarioData->startTime = newData.startTime;
    scenarioData->startDay = newData.startDay;
    scenarioData->startMonth = newData.startMonth;
    scenarioData->startYear = newData.startYear;
    scenarioData->sunRise = newData.sunRise;
    scenarioData->sunSet = newData.sunSet;
    scenarioData->weather = newData.weather;
    scenarioData->rainIntensity = newData.rainIntensity;
    scenarioData->visibilityRange = newData.visibilityRange;
    scenarioData->scenarioName = newData.scenarioName;
    scenarioData->description = newData.description;
    
    recalculateLegTimes(); //These need to be updated to match new startTime.
    checkName(); //Check if the scenario name chosen will cause overwrite, and if so, set flag.
}

void ControllerModel::checkName() //Check if the scenario name chosen will mean that an existing scenario gets overwritten, and update flag in GeneralData
{
    //Find path to scenario folder
    std::string userFolder = Utilities::getUserDir();
    std::string scenarioPath = "Scenarios/";
    if (Utilities::pathExists(userFolder + scenarioPath)) {
        scenarioPath = userFolder + scenarioPath;
    }

    //Check if path exists already
    std::string fullScenarioPath = scenarioPath.append(scenarioData->scenarioName);

    if (Utilities::pathExists(fullScenarioPath)) {
        scenarioData->willOverwrite=true;
    } else {
        scenarioData->willOverwrite=false;
    }

    //Check if a valid multiplayer name
    scenarioData->multiplayerName=false;
    if (scenarioData->scenarioName.length() >= 3) {
        std::string endChars = scenarioData->scenarioName.substr(scenarioData->scenarioName.length()-3,3);
        if (endChars == "_mp" || endChars == "_MP") {
            scenarioData->multiplayerName = true;
        }
    }

}

void ControllerModel::changeLeg(irr::s32 ship, irr::s32 index, irr::f32 legCourse, irr::f32 legSpeed, irr::f32 legDistance)  //Change othership (or ownship) course, speed etc.
{
    //If ownship:
    if (ship==0) {
        scenarioData->ownShipData.initialBearing = legCourse;
        scenarioData->ownShipData.initialSpeed = legSpeed;
    }

    //If other ship:
    if (ship>0) {
        int otherShipIndex = ship-1;
        if (otherShipIndex < scenarioData->otherShipsData.size()) {
            if (index < scenarioData->otherShipsData.at(otherShipIndex).legs.size()) {
                scenarioData->otherShipsData.at(otherShipIndex).legs.at(index).bearing = legCourse;
                scenarioData->otherShipsData.at(otherShipIndex).legs.at(index).speed = legSpeed;
                scenarioData->otherShipsData.at(otherShipIndex).legs.at(index).distance = legDistance;
            }
        }
    }
    recalculateLegTimes(); //Subsequent leg start times may have changed, so recalculate these
}

void ControllerModel::deleteLeg(irr::s32 ship, irr::s32 index)
{
    //If other ship:
    if (ship>0) {
        int otherShipIndex = ship-1;
        if (otherShipIndex < scenarioData->otherShipsData.size()) {
            if (index < scenarioData->otherShipsData.at(otherShipIndex).legs.size()) {
                //Delete this leg
                scenarioData->otherShipsData.at(otherShipIndex).legs.erase(scenarioData->otherShipsData.at(otherShipIndex).legs.begin() + index);
                recalculateLegTimes(); //Subsequent leg start times may have changed, so recalculate these
            }
        }
    }
}

void ControllerModel::setMMSI(irr::s32 ship, int mmsi)
{
    //If other ship:
    if (ship>0) {
        int otherShipIndex = ship-1;
        if (otherShipIndex < scenarioData->otherShipsData.size()) {
            scenarioData->otherShipsData.at(otherShipIndex).mmsi = mmsi;
        }
    }   
}

void ControllerModel::addLeg(irr::s32 ship, irr::s32 afterLegNumber, irr::f32 legCourse, irr::f32 legSpeed, irr::f32 legDistance)
{
    //If other ship:
    if (ship>0) {
        int otherShipIndex = ship-1;
        if (otherShipIndex < scenarioData->otherShipsData.size()) {
            std::vector<LegData>* legs = &scenarioData->otherShipsData.at(otherShipIndex).legs;
            //Check if leg is reasonable, and is before the 'stop leg'
            //A special case allows afterLegNumber to equal -1, for when only a single 'stop leg' exists
            if (afterLegNumber >= -1 && afterLegNumber < ((int)legs->size() - 1)) {

                //If the 'after' leg is the penultimate, add a leg before the stop one, starting now
                if (afterLegNumber == ((int)legs->size()-2))  { //This also catches the special case where there is only the 'stop' leg, so the 'afterLegNumber value is -1

                    LegData newLeg;
                    newLeg.bearing = legCourse;
                    newLeg.speed = legSpeed;
                    newLeg.distance = legDistance;
                    legs->insert(legs->end()-1, newLeg); //Insert before final leg

                } else {

                    LegData newLeg;
                    newLeg.bearing = legCourse;
                    newLeg.speed = legSpeed;
                    newLeg.distance = legDistance;

                    //std::cout << "B" << std::endl;

                    legs->insert(legs->begin()+afterLegNumber+1, newLeg); //Insert leg
                }
            }
        }
    }
    recalculateLegTimes(); //Subsequent leg start times may have changed, so recalculate these
}

void ControllerModel::addShip(std::string name, irr::core::vector2df position)
{
    OtherShipData newShip;
    newShip.initialX = position.X;
    newShip.initialZ = position.Y;
    newShip.shipName = name;
    newShip.mmsi = 0;
    //Add a 'stop' leg
    LegData stopLeg;
    stopLeg.bearing=0;
    stopLeg.speed=0;
    stopLeg.distance=0;
    newShip.legs.push_back(stopLeg);

    //Add to list
    scenarioData->otherShipsData.push_back(newShip);

    recalculateLegTimes(); //Subsequent leg start times may have changed, so recalculate these
}

void ControllerModel::deleteShip(irr::s32 ship) 
{
	//If other ship:
	if (ship > 0) {
		int otherShipIndex = ship - 1;
		if (otherShipIndex < scenarioData->otherShipsData.size()) {
			scenarioData->otherShipsData.erase(scenarioData->otherShipsData.begin()+otherShipIndex);
		}
	}
}

void ControllerModel::recalculateLegTimes()
{
    //Run through all othership legs, recalculating leg stop times
    irr::f32 scenarioStartTime = scenarioData->startTime; //Legs start at the start of the scenario

    for (int thisShip = 0; thisShip < scenarioData->otherShipsData.size(); thisShip++) {

        irr::f32 legStartTime = scenarioStartTime; //Legs start at the start of the scenario
        for (int thisLeg = 0; thisLeg < scenarioData->otherShipsData.at(thisShip).legs.size(); thisLeg++) {
            scenarioData->otherShipsData.at(thisShip).legs.at(thisLeg).startTime = legStartTime;
            irr::f32 thisLegDistance = scenarioData->otherShipsData.at(thisShip).legs.at(thisLeg).distance;
            irr::f32 thisLegSpeed = scenarioData->otherShipsData.at(thisShip).legs.at(thisLeg).speed;
            //Update legStart time for start of next leg:
            legStartTime+= SECONDS_IN_HOUR*(thisLegDistance/fabs(thisLegSpeed)); // nm/kts -> hours, so convert to seconds
        }
    }

}

void ControllerModel::changeOwnShipName(std::string name)
{
    scenarioData->ownShipData.ownShipName = name;

}

void ControllerModel::changeOtherShipName(irr::s32 ship, std::string name)
{
    irr::s32 shipIndex = ship-1; //'ship' number starts at 1 for otherShips
    if (shipIndex < scenarioData->otherShipsData.size()) {
        scenarioData->otherShipsData.at(shipIndex).shipName = name;
    }
}

void ControllerModel::save()
{
    //Do save here
    std::cout << "About to save" << std::endl;

    //check there's a scenario name to save to
    if (scenarioData->scenarioName.empty()) {
        device->getGUIEnvironment()->addMessageBox(lang->translate("failed").c_str(), lang->translate("failedScenarioSave").c_str());
        return;
    }

    //Find path to scenario folder
    std::string userFolder = Utilities::getUserDir();
    std::string scenarioPath = "Scenarios/";
    if (Utilities::pathExists(userFolder + scenarioPath)) {
        scenarioPath = userFolder + scenarioPath;
    }

    //Check if path exists already
    std::string fullScenarioPath = scenarioPath.append(scenarioData->scenarioName);

    if (!Utilities::pathExists(fullScenarioPath)) {
        //Path does not exist, try and create it
        #ifdef _WIN32
        _mkdir(fullScenarioPath.c_str());
        #else
        mkdir(fullScenarioPath.c_str(),0755);
        #endif // _WIN32

    } else {
        std::cout << "Overwriting scenario at " << fullScenarioPath << std::endl;
    }

    //Try and create files
    bool successOfFar = true;

    //environment.ini
    std::string envPath = fullScenarioPath + "/environment.ini";
    std::ofstream envFile;

    envFile.open(envPath.c_str());
    envFile << "Setting=\"" << worldName << "\"" << std::endl;
    envFile << "StartTime=" << scenarioData->startTime/SECONDS_IN_HOUR << std::endl;
    envFile << "StartDay=" << scenarioData->startDay << std::endl;
    envFile << "StartMonth=" << scenarioData->startMonth << std::endl;
    envFile << "StartYear=" << scenarioData->startYear << std::endl;
    envFile << "SunRise=" << scenarioData->sunRise << std::endl;
    envFile << "SunSet=" << scenarioData->sunSet << std::endl;
    //envFile << "Variation=0" << std::endl;
    envFile << "VisibilityRange=" << scenarioData->visibilityRange << std::endl;
    envFile << "Weather=" << scenarioData->weather << std::endl;
    //envFile << "WindDirection=90" << std::endl;
    envFile << "Rain=" << scenarioData->rainIntensity << std::endl;
    envFile.close();
    if (!envFile.good()) {successOfFar=false;}

    //othership.ini
    std::string otherPath = fullScenarioPath + "/othership.ini";
    std::ofstream otherFile;

    otherFile.open(otherPath.c_str());
    otherFile << "Number=" << scenarioData->otherShipsData.size() << std::endl;
    for (int i = 1; i<=scenarioData->otherShipsData.size(); i++) {
        otherFile << "Type(" << i << ")=\"" << scenarioData->otherShipsData.at(i-1).shipName << "\"" << std::endl;
        otherFile << "InitLong(" << i << ")=" << xToLong(scenarioData->otherShipsData.at(i-1).initialX) << std::endl;
        otherFile << "InitLat(" << i << ")=" << zToLat(scenarioData->otherShipsData.at(i-1).initialZ) << std::endl;
        otherFile << "mmsi(" << i << ")=" << scenarioData->otherShipsData.at(i-1).mmsi << std::endl;
        //Don't save last leg, as this is an automatically added 'stop' leg.
        otherFile << "Legs(" << i << ")=" << scenarioData->otherShipsData.at(i-1).legs.size() - 1 << std::endl;

        for (int j = 1; j<=scenarioData->otherShipsData.at(i-1).legs.size() - 1; j++) {
            otherFile << "Bearing(" << i << "," << j << ")=" << scenarioData->otherShipsData.at(i-1).legs.at(j-1).bearing << std::endl;
            otherFile << "Speed(" << i << "," << j << ")=" << scenarioData->otherShipsData.at(i-1).legs.at(j-1).speed << std::endl;
            otherFile << "Distance(" << i << "," << j << ")=" << scenarioData->otherShipsData.at(i-1).legs.at(j-1).distance << std::endl;
        }
    }
    otherFile.close();
    if (!otherFile.good()) {successOfFar=false;}

    //ownship.ini
    std::string ownPath = fullScenarioPath + "/ownship.ini";
    std::ofstream ownFile;

    ownFile.open(ownPath.c_str());
    ownFile << "ShipName=\"" << scenarioData->ownShipData.ownShipName << "\"" << std::endl;
    ownFile << "InitialLong=" << xToLong(scenarioData->ownShipData.initialX) << std::endl;
    ownFile << "InitialLat=" << zToLat(scenarioData->ownShipData.initialZ) << std::endl;
    ownFile << "InitialBearing=" << scenarioData->ownShipData.initialBearing << std::endl;
    ownFile << "InitialSpeed=" << scenarioData->ownShipData.initialSpeed << std::endl;
    ownFile.close();
    if (!ownFile.good()) {successOfFar=false;}

    //description.ini
    std::string descriptionPath = fullScenarioPath + "/description.ini";
    std::ofstream descriptionFile;

    descriptionFile.open(descriptionPath.c_str());
    descriptionFile << scenarioData->description;
    descriptionFile.close();
    if (!descriptionFile.good()) {successOfFar=false;}

    if (successOfFar) {
        device->getGUIEnvironment()->addMessageBox(lang->translate("saved").c_str(), lang->translate("scenarioSaved").c_str());
    } else {
        device->getGUIEnvironment()->addMessageBox(lang->translate("failed").c_str(), lang->translate("failedScenarioSave").c_str());
    }


}

void ControllerModel::setMouseDown(bool isMouseDown)
{
    mouseDown = isMouseDown;
}
