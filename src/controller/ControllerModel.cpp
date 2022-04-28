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

#define MAX_PX_IN_MAP 160000000

//Constructor
ControllerModel::ControllerModel(irr::IrrlichtDevice* device, GUIMain* gui, Network* network, std::string worldName, irr::u32 _zoomLevels)
{

    this->gui = gui;
    this->network = network;
    this->device = device;
    driver = device->getVideoDriver();
	this->zoomLevels = _zoomLevels;

    unscaledMap = 0;
    for (unsigned int i = 0; i<zoomLevels; i++) {
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

    //Calculate scaling needed
    //irr::u32 widthFromHeight = loadedSize.Height * widthToHeight;
    //irr::u32 heightFromWidth = loadedSize.Width / widthToHeight;
    //Always scale bigger if needed
    //std::cout << terrainXWidth <<  " " << terrainZWidth << std::endl;
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
			std::cout << "About to create empty scaled map " << i << std::endl;
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

//    std::cout << "Actual width px " << scaledMap->getDimension().Width << " height px " << scaledMap->getDimension().Height << std::endl;

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

void ControllerModel::update(const irr::f32& time, const ShipData& ownShipData, const std::vector<OtherShipDisplayData>& otherShipsData, const std::vector<PositionData>& buoysData, const irr::f32& weather, const irr::f32& visibility, const irr::f32& rain, bool& mobVisible, PositionData& mobData, const std::vector<AISData>& aisData)
{
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
    irr::s32 topLeftX = -1*ownShipData.X/metresPerPx.at(currentZoom) + driver->getScreenSize().Width/2 + mapOffsetX;
    irr::s32 topLeftZ = ownShipData.Z/metresPerPx.at(currentZoom)    + driver->getScreenSize().Height/2 - scaledMap.at(currentZoom)->getDimension().Height + mapOffsetZ;

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

    //Process the AIS data here:
    //Check for new AIS data
    for (int i=0;i<aisData.size();i++) {
        
        //If message type is 1,2 or 3, calculate X and Z position
        

        //Check if a record exists for this MMSI. If not, create a new record.
        int aisShipsId = -1;
        for (int j=0;j<aisShips.size();j++) {
            if (aisShips.at(j).mmsi == aisData.at(i).mmsi) {
                aisShipsId = j;
            }
        }
        
        if (aisShipsId == -1) {
            //Not found, add a new record
            aisShips.push_back(aisData.at(i));
            aisShipsId = aisShips.size()-1;
            aisShips.at(aisShipsId).X = longToX(aisData.at(i).longitude);
            aisShips.at(aisShipsId).Z = latToZ(aisData.at(i).latitude);
        } else {
            //Found, update record
            if (aisData.at(i).messageID == 1 || aisData.at(i).messageID == 2 || aisData.at(i).messageID == 3) {
                aisShips.at(aisShipsId).cog = aisData.at(i).cog;
                aisShips.at(aisShipsId).latitude = aisData.at(i).latitude;
                aisShips.at(aisShipsId).longitude = aisData.at(i).longitude;
                aisShips.at(aisShipsId).sog = aisData.at(i).sog;
                aisShips.at(aisShipsId).X = longToX(aisData.at(i).longitude);
                aisShips.at(aisShipsId).Z = latToZ(aisData.at(i).latitude);
            } else if (aisData.at(i).messageID == 5) {
                aisShips.at(aisShipsId).name = aisData.at(i).name;
            }
        }

        if (aisData.at(i).messageID == 1 || aisData.at(i).messageID == 2 || aisData.at(i).messageID == 3) {
            //Send a message to update the othership leg position, if the MMSI matches an own ship

            bool sendNetworkUpdate = false;
            int otherShipNumber = 0;
            for(int j = 0; j < otherShipsData.size(); j++) {
                if (otherShipsData.at(j).mmsi == aisShips.at(aisShipsId).mmsi) {
                    sendNetworkUpdate = true;
                    otherShipNumber = j+1; //For the network message the first other ship is 1.
                }
            }

            if (sendNetworkUpdate) {
                
                //Get COG in degrees and SOG in Nm, from AIS coding
                irr::f32 cogFromAIS = 0;
                if (aisShips.at(aisShipsId).cog != 3600) {
                    cogFromAIS = (irr::f32)aisShips.at(aisShipsId).cog/10.0;
                }
                irr::f32 sogFromAIS = 0;
                if (aisShips.at(aisShipsId).sog != 1023) {
                    sogFromAIS = (irr::f32)aisShips.at(aisShipsId).sog/10.0;
                }
                
                std::string messageToSend = "MCRL,"; //Reset legs, to give a single leg, and set position
                messageToSend.append(Utilities::lexical_cast<std::string>(otherShipNumber));
                messageToSend.append(",");
                messageToSend.append(Utilities::lexical_cast<std::string>(aisShips.at(aisShipsId).X));
                messageToSend.append(",");
                messageToSend.append(Utilities::lexical_cast<std::string>(aisShips.at(aisShipsId).Z));
                messageToSend.append(",");
                messageToSend.append(Utilities::lexical_cast<std::string>(cogFromAIS));
                messageToSend.append(",");
                messageToSend.append(Utilities::lexical_cast<std::string>(sogFromAIS));
                messageToSend.append("#");
                network->setStringToSend(messageToSend);
            }

        }
        
    
    }

    //Send the current data to the gui, and update it
    gui->updateGuiData(time,mapOffsetX,mapOffsetZ,metresPerPx.at(currentZoom),ownShipData.X,ownShipData.Z,ownShipData.heading, buoysData,otherShipsData,aisShips,mobVisible, mobData.X, mobData.Z, displayMapTexture,selectedShip,selectedLeg, terrainLong, terrainLongExtent, terrainXWidth, terrainLat, terrainLatExtent, terrainZWidth, weather, visibility, rain);
}

void ControllerModel::resetOffset()
{
    mapOffsetX = 0;
    mapOffsetZ = 0;
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

void ControllerModel::setMouseDown(bool isMouseDown)
{
    mouseDown = isMouseDown;
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
