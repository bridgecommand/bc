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

//Constructor
ControllerModel::ControllerModel(irr::IrrlichtDevice* device, Lang* lang, GUIMain* gui, std::string worldName, OwnShipData* ownShipData, std::vector<OtherShipData>* otherShipsData, std::vector<PositionData>* buoysData, GeneralData* generalData)
{

    this->gui = gui;
    this->lang = lang;
    this->device = device;
    driver = device->getVideoDriver();

    this->ownShipData = ownShipData;
    this->otherShipsData = otherShipsData;
    this->buoysData = buoysData;
    this->generalData = generalData;
    this->worldName = worldName;

    checkOverwrite();//Check if the scenario name (preset in generalData) will cause an overwrite, and if so, set flag in generalData

    unscaledMap = 0;
    scaledMap = 0;

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

    //Fixme: Note we're only loading the primary terrain here
    terrainLong = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLong",1));
    terrainLat = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLat",1));
    terrainLongExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLongExtent",1));
    terrainLatExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLatExtent",1));

    std::string displayMapName = IniFile::iniFileToString(worldTerrainFile, "MapImage");
    if (displayMapName.empty())
        {displayMapName = IniFile::iniFileToString(worldTerrainFile, "RadarImage");} //Fall back to 'RadarImage' if 'MapImage' parameter isn't set

    //If still empty, we don't have an image to load, so fail here
    if (displayMapName.empty()) {
        std::cout << "Could not load name of map image from ini file: " <<  worldTerrainFile << " (please check this file exists and has not been corrupted)." << std::endl;
        exit(EXIT_FAILURE);
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

    //Calculate scaling needed
    irr::u32 widthFromHeight = loadedSize.Height * widthToHeight;
    irr::u32 heightFromWidth = loadedSize.Width / widthToHeight;
    //Always scale bigger if needed
    irr::u32 requiredWidth = std::max(widthFromHeight, loadedSize.Width);
    irr::u32 requiredHeight = std::max(heightFromWidth, loadedSize.Height);

    std::cout << "Required width px " << requiredWidth << " height px " << requiredHeight << std::endl;

    //Create scaled map with the same image format of the size required
    scaledMap = driver->createImage(unscaledMap->getColorFormat(),irr::core::dimension2d<irr::u32>(requiredWidth,requiredHeight));
    //Copy and scale image
    unscaledMap->copyToScaling(scaledMap);

    //Drop the unscaled map, as we don't need this again
    unscaledMap->drop();
    unscaledMap=0;

    //Save scale
    metresPerPx = terrainXWidth / requiredWidth;

    std::cout << "Actual width px " << scaledMap->getDimension().Width << " height px " << scaledMap->getDimension().Height << std::endl;

}

//Destructor
ControllerModel::~ControllerModel()
{
    scaledMap->drop();
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
    irr::video::IImage* tempImage = driver->createImage(scaledMap->getColorFormat(),screenSize); //Empty image

    //Copy in data
    irr::s32 topLeftX = -1*ownShipData->X/metresPerPx + driver->getScreenSize().Width/2 + mapOffsetX;
    irr::s32 topLeftZ = ownShipData->Z/metresPerPx    + driver->getScreenSize().Height/2 - scaledMap->getDimension().Height + mapOffsetZ;

    scaledMap->copyTo(tempImage,irr::core::position2d<irr::s32>(topLeftX,topLeftZ)); //Fixme: Check bounds are reasonable

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
    gui->updateGuiData(*generalData,mapOffsetX,mapOffsetZ,metresPerPx,*ownShipData,*buoysData,*otherShipsData,displayMapTexture,selectedShip,selectedLeg, terrainLong, terrainLongExtent, terrainXWidth, terrainLat, terrainLatExtent, terrainZWidth);
}

void ControllerModel::resetOffset()
{
    mapOffsetX = 0;
    mapOffsetZ = 0;
}

void ControllerModel::setShipPosition(irr::s32 ship, irr::core::vector2df position)
{
    if (ship==0) {
        //Own ship
        ownShipData->X = position.X;
        ownShipData->Z = position.Y;
    } else if (ship>0) {
        //Other ship
        irr::s32 otherShipNumber = ship-1; //Ship number is minimum of 1, so subtract 1 to start at 0
        if (otherShipNumber < otherShipsData->size()) {
            otherShipsData->at(otherShipNumber).X = position.X;
            otherShipsData->at(otherShipNumber).Z = position.Y;
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

void ControllerModel::setScenarioData(GeneralData newData)
{
    *generalData=newData;
    recalculateLegTimes(); //These need to be updated to match new startTime.
    checkOverwrite(); //Check if the scenario name chosen will cause overwrite, and if so, set flag.
}

void ControllerModel::checkOverwrite() //Check if the scenario name chosen will mean that an existing scenario gets overwritten, and update flag in GeneralData
{
    //Find path to scenario folder
    std::string userFolder = Utilities::getUserDir();
    std::string scenarioPath = "Scenarios/";
    if (Utilities::pathExists(userFolder + scenarioPath)) {
        scenarioPath = userFolder + scenarioPath;
    }

    //Check if path exists already
    std::string fullScenarioPath = scenarioPath.append(generalData->scenarioName);

    if (Utilities::pathExists(fullScenarioPath)) {
        generalData->willOverwrite=true;
    } else {
        generalData->willOverwrite=false;
    }
}

void ControllerModel::changeLeg(irr::s32 ship, irr::s32 index, irr::f32 legCourse, irr::f32 legSpeed, irr::f32 legDistance)  //Change othership (or ownship) course, speed etc.
{
    //If ownship:
    if (ship==0) {
        ownShipData->heading = legCourse;
        ownShipData->initialSpeed = legSpeed;
    }

    //If other ship:
    if (ship>0) {
        int otherShipIndex = ship-1;
        if (otherShipIndex < otherShipsData->size()) {
            if (index < otherShipsData->at(otherShipIndex).legs.size()) {
                otherShipsData->at(otherShipIndex).legs.at(index).bearing = legCourse;
                otherShipsData->at(otherShipIndex).legs.at(index).speed = legSpeed;
                otherShipsData->at(otherShipIndex).legs.at(index).distance = legDistance;
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
        if (otherShipIndex < otherShipsData->size()) {
            if (index < otherShipsData->at(otherShipIndex).legs.size()) {
                //Delete this leg
                otherShipsData->at(otherShipIndex).legs.erase(otherShipsData->at(otherShipIndex).legs.begin() + index);
                recalculateLegTimes(); //Subsequent leg start times may have changed, so recalculate these
            }
        }
    }
}

void ControllerModel::addLeg(irr::s32 ship, irr::s32 afterLegNumber, irr::f32 legCourse, irr::f32 legSpeed, irr::f32 legDistance)
{
    //If other ship:
    if (ship>0) {
        int otherShipIndex = ship-1;
        if (otherShipIndex < otherShipsData->size()) {
            std::vector<Leg>* legs = &otherShipsData->at(otherShipIndex).legs;
            //Check if leg is reasonable, and is before the 'stop leg'
            //A special case allows afterLegNumber to equal -1, for when only a single 'stop leg' exists
            if (afterLegNumber >= -1 && afterLegNumber < ((int)legs->size() - 1)) {

                //If the 'after' leg is the penultimate, add a leg before the stop one, starting now
                if (afterLegNumber == ((int)legs->size()-2))  { //This also catches the special case where there is only the 'stop' leg, so the 'afterLegNumber value is -1

                    Leg newLeg;
                    newLeg.bearing = legCourse;
                    newLeg.speed = legSpeed;
                    newLeg.distance = legDistance;
                    legs->insert(legs->end()-1, newLeg); //Insert before final leg

                } else {

                    Leg newLeg;
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
    newShip.X = position.X;
    newShip.Z = position.Y;
    newShip.name = name;
    //Add a 'stop' leg
    Leg stopLeg;
    stopLeg.bearing=0;
    stopLeg.speed=0;
    stopLeg.distance=0;
    newShip.legs.push_back(stopLeg);

    //Add to list
    otherShipsData->push_back(newShip);

    recalculateLegTimes(); //Subsequent leg start times may have changed, so recalculate these
}

void ControllerModel::recalculateLegTimes()
{
    //Run through all othership legs, recalculating leg stop times
    irr::f32 scenarioStartTime = generalData->startTime; //Legs start at the start of the scenario

    for (int thisShip = 0; thisShip < otherShipsData->size(); thisShip++) {

        irr::f32 legStartTime = scenarioStartTime; //Legs start at the start of the scenario
        for (int thisLeg = 0; thisLeg < otherShipsData->at(thisShip).legs.size(); thisLeg++) {
            otherShipsData->at(thisShip).legs.at(thisLeg).startTime = legStartTime;
            irr::f32 thisLegDistance = otherShipsData->at(thisShip).legs.at(thisLeg).distance;
            irr::f32 thisLegSpeed = otherShipsData->at(thisShip).legs.at(thisLeg).speed;
            //Update legStart time for start of next leg:
            legStartTime+= SECONDS_IN_HOUR*(thisLegDistance/fabs(thisLegSpeed)); // nm/kts -> hours, so convert to seconds
        }
    }

}

void ControllerModel::changeOwnShipName(std::string name)
{
    ownShipData->name = name;

}

void ControllerModel::changeOtherShipName(irr::s32 ship, std::string name)
{
    irr::s32 shipIndex = ship-1; //'ship' number starts at 1 for otherShips
    if (shipIndex < otherShipsData->size()) {
        otherShipsData->at(shipIndex).name = name;
    }
}

void ControllerModel::save()
{
    //Do save here
    std::cout << "About to save" << std::endl;

    //check there's a scenario name to save to
    if (generalData->scenarioName.empty()) {
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
    std::string fullScenarioPath = scenarioPath.append(generalData->scenarioName);

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
    envFile << "StartTime=" << generalData->startTime/SECONDS_IN_HOUR << std::endl;
    envFile << "StartDay=" << generalData->startDay << std::endl;
    envFile << "StartMonth=" << generalData->startMonth << std::endl;
    envFile << "StartYear=" << generalData->startYear << std::endl;
    envFile << "SunRise=" << generalData->sunRiseTime << std::endl;
    envFile << "SunSet=" << generalData->sunSetTime << std::endl;
    //envFile << "Variation=0" << std::endl;
    envFile << "VisibilityRange=" << generalData->visibility << std::endl;
    envFile << "Weather=" << generalData->weather << std::endl;
    //envFile << "WindDirection=90" << std::endl;
    envFile << "Rain=" << generalData->rain << std::endl;
    envFile.close();
    if (!envFile.good()) {successOfFar=false;}

    //othership.ini
    std::string otherPath = fullScenarioPath + "/othership.ini";
    std::ofstream otherFile;

    otherFile.open(otherPath.c_str());
    otherFile << "Number=" << otherShipsData->size() << std::endl;
    for (int i = 1; i<=otherShipsData->size(); i++) {
        otherFile << "Type(" << i << ")=\"" << otherShipsData->at(i-1).name << "\"" << std::endl;
        otherFile << "InitLong(" << i << ")=" << xToLong(otherShipsData->at(i-1).X) << std::endl;
        otherFile << "InitLat(" << i << ")=" << zToLat(otherShipsData->at(i-1).Z) << std::endl;
        //Don't save last leg, as this is an automatically added 'stop' leg.
        otherFile << "Legs(" << i << ")=" << otherShipsData->at(i-1).legs.size() - 1 << std::endl;

        for (int j = 1; j<=otherShipsData->at(i-1).legs.size() - 1; j++) {
            otherFile << "Bearing(" << i << "," << j << ")=" << otherShipsData->at(i-1).legs.at(j-1).bearing << std::endl;
            otherFile << "Speed(" << i << "," << j << ")=" << otherShipsData->at(i-1).legs.at(j-1).speed << std::endl;
            otherFile << "Distance(" << i << "," << j << ")=" << otherShipsData->at(i-1).legs.at(j-1).distance << std::endl;
        }
    }
    otherFile.close();
    if (!otherFile.good()) {successOfFar=false;}

    //ownship.ini
    std::string ownPath = fullScenarioPath + "/ownship.ini";
    std::ofstream ownFile;

    ownFile.open(ownPath.c_str());
    ownFile << "ShipName=\"" << ownShipData->name << "\"" << std::endl;
    ownFile << "InitialLong=" << xToLong(ownShipData->X) << std::endl;
    ownFile << "InitialLat=" << zToLat(ownShipData->Z) << std::endl;
    ownFile << "InitialBearing=" << ownShipData->heading << std::endl;
    ownFile << "InitialSpeed=" << ownShipData->initialSpeed << std::endl;
    ownFile.close();
    if (!ownFile.good()) {successOfFar=false;}

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
