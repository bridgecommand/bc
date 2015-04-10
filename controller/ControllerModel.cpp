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
#include <iostream>

//Constructor
ControllerModel::ControllerModel(irr::IrrlichtDevice* device, GUIMain* gui)
{

    this->gui = gui;
    this->device = device;
    driver = device->getVideoDriver();

    unscaledMap = 0; //FIXME: check - do pointers get automatically initialised as 0?
    scaledMap = 0;

    //construct path to world model
    std::string worldPath = "../World/";
    worldPath.append("SimpleEstuary"); //Fixme: Hardcoded

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

    //Load map image if possible (if not, end with error)
    std::string displayMapPath = worldPath;
    displayMapPath.append("/");
    displayMapPath.append(displayMapName);
    unscaledMap = driver->createImageFromFile(displayMapPath.c_str()); //Fixme: need to get an irr::video::IImageLoader here.
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

    std::cout << "Width px " << requiredWidth << " Height px " << requiredHeight << std::endl;

    //Create scaled map with the same image format of the size required
    scaledMap = driver->createImage(unscaledMap->getColorFormat(),irr::core::dimension2d<irr::u32>(requiredWidth,requiredHeight));
    //Copy and scale image
    unscaledMap->copyToScaling(scaledMap);

    //Drop the unscaled map, as we don't need this again
    unscaledMap->drop();
    //Fixme - should unscaled map be set to zero now?

    //Save scale
    metresPerPx = terrainXWidth / requiredWidth;

    std::cout << "Actual width px " << scaledMap->getDimension().Width << " Height px " << scaledMap->getDimension().Height << std::endl;

}

//Destructor
ControllerModel::~ControllerModel()
{
    scaledMap->drop();
}

void ControllerModel::update(const irr::u64& timestamp, const ShipData& ownShipData, const std::vector<OtherShipData>& otherShipsData, const std::vector<PositionData>& buoysData)
{
    //TODO: Work out the required area of the map image, and create this as a texture to go to the gui
    irr::core::dimension2d<irr::u32> screenSize = device->getVideoDriver()->getScreenSize();
    //grab an area this size from the scaled map
    irr::video::IImage* tempImage = driver->createImage(scaledMap->getColorFormat(),screenSize); //Empty image

    //Copy in data
    irr::s32 topLeftX = -1*ownShipData.X/metresPerPx + driver->getScreenSize().Width/2;
    irr::s32 topLeftZ = ownShipData.Z/metresPerPx    + driver->getScreenSize().Height/2 - scaledMap->getDimension().Height;

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
    gui->updateGuiData(timestamp,metresPerPx,ownShipData.X,ownShipData.Z,buoysData,otherShipsData,displayMapTexture);
}
