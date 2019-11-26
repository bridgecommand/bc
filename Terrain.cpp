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

//NOTE: This uses a modified version of Irrlicht for terrain loading, which loads the terrain natively rotated
//180 degrees compared to standard Irrlicht 1.8.1.

#include "Terrain.hpp"

#include "IniFile.hpp"
#include "Constants.hpp"
#include "Utilities.hpp"

#include <iostream>

//using namespace irr;

Terrain::Terrain()
{

}

Terrain::~Terrain()
{
    //dtor
}

void Terrain::load(const std::string& worldPath, irr::scene::ISceneManager* smgr)
{

    irr::video::IVideoDriver* driver = smgr->getVideoDriver();

    //Get full path to the main Terrain.ini file
    std::string worldTerrainFile = worldPath;
    worldTerrainFile.append("/terrain.ini");

    irr::u32 numberOfTerrains = IniFile::iniFileTou32(worldTerrainFile, "Number");
    if (numberOfTerrains <= 0) {
        std::cerr << "Could not load terrain." << std::endl;
        exit(EXIT_FAILURE);
    }

    for (unsigned int i = 1; i<=numberOfTerrains; i++) {


        irr::f32 terrainLong = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLong",i));
        irr::f32 terrainLat = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLat",i));
        irr::f32 terrainLongExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLongExtent",i));
        irr::f32 terrainLatExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLatExtent",i));

        irr::f32 terrainMaxHeight=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainMaxHeight",i));
        irr::f32 seaMaxDepth=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("SeaMaxDepth",i));
        irr::f32 terrainHeightMapSize=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapSize",i));

        std::string heightMapName = IniFile::iniFileToString(worldTerrainFile, IniFile::enumerate1("HeightMap",i));
        std::string textureMapName = IniFile::iniFileToString(worldTerrainFile, IniFile::enumerate1("Texture",i));

        //Terrain dimensions in metres
        irr::f32 terrainXWidth = terrainLongExtent * 2.0 * PI * EARTH_RAD_M * cos( irr::core::degToRad(terrainLat + terrainLatExtent/2.0)) / 360.0;
        irr::f32 terrainZWidth = terrainLatExtent  * 2.0 * PI * EARTH_RAD_M / 360;

        //calculations just needed for terrain loading
        irr::f32 scaleX = terrainXWidth / (terrainHeightMapSize);
        irr::f32 scaleY = (terrainMaxHeight + seaMaxDepth)/ (255.0);
        irr::f32 scaleZ = terrainZWidth / (terrainHeightMapSize);
        irr::f32 terrainY = -1*seaMaxDepth;

        //Full paths
        std::string heightMapPath = worldPath;
        heightMapPath.append("/");
        heightMapPath.append(heightMapName);
        std::string textureMapPath = worldPath;
        textureMapPath.append("/");
        textureMapPath.append(textureMapName);

        //Fixme: Could also check that the terrain is now 2^n + 1 square (was 2^n in B3d version)
        //Add an empty terrain
        irr::scene::ITerrainSceneNode* terrain = smgr->addTerrainSceneNode("",0,-1,irr::core::vector3df(0.f, terrainY, 0.f),irr::core::vector3df(0.f, 0.f, 0.f),irr::core::vector3df(scaleX,scaleY,scaleZ),irr::video::SColor(255,255,255,255),5,irr::scene::ETPS_17,0,true);
        //Load the map
        irr::io::IReadFile* heightMapFile = smgr->getFileSystem()->createAndOpenFile(heightMapPath.c_str());
        //Check the height map file has loaded and the terrain exists
        if (terrain==0 || heightMapFile == 0) {
            //Could not load terrain
            std::cerr << "Could not load terrain." << std::endl;
            exit(EXIT_FAILURE);
        }

        //Load the terrain and check success
        bool loaded = false;
        //Check if extension is .irr::f32 for binary floating point file
        std::string extension = "";
        if (heightMapName.length() > 3) {
            extension = heightMapName.substr(heightMapName.length() - 4,4);
            Utilities::to_lower(extension);
        }
        if (extension.compare(".irr::f32") == 0 ) {
            //Binary file
            loaded = terrain->loadHeightMapRAW(heightMapFile,32,true,true);
            //Set scales etc to be 1.0, so heights are used directly
            terrain->setScale(irr::core::vector3df(scaleX,1.0f,scaleZ));
            terrain->setPosition(irr::core::vector3df(0.f, 0.f, 0.f));
        } else {
            loaded = terrain->loadHeightMap(heightMapFile);
        }

        if (!loaded) {
            //Could not load terrain
            std::cerr << "Could not load terrain." << std::endl;
            exit(EXIT_FAILURE);
        }

        heightMapFile->drop();
        //TODO: Do we need to drop terrain?

        terrain->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
        terrain->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting
        //Todo: Anti-aliasing flag?
        terrain->setMaterialTexture(0, driver->getTexture(textureMapPath.c_str()));

        if (i==1) {
            //Private member variables used in further calculations
            primeTerrainLong = terrainLong;
            primeTerrainXWidth = terrainXWidth;
            primeTerrainLongExtent = terrainLongExtent;
            primeTerrainLat = terrainLat;
            primeTerrainZWidth = terrainZWidth;
            primeTerrainLatExtent = terrainLatExtent;


        } else {
            //Non-primary terrains need to be moved to account for their position (primary terrain starts at 0,0)

            irr::core::vector3df currentPos = terrain->getPosition();
            irr::f32 deltaX = (terrainLong - primeTerrainLong) * primeTerrainXWidth / primeTerrainLongExtent;
            irr::f32 deltaZ = (terrainLat - primeTerrainLat) * primeTerrainZWidth / primeTerrainLatExtent;
            irr::f32 newPosX = currentPos.X + deltaX;
            irr::f32 newPosY = currentPos.Y;
            irr::f32 newPosZ = currentPos.Z + deltaZ;
            terrain->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
        }

        terrains.push_back(terrain);

    }


}

irr::f32 Terrain::getHeight(irr::f32 x, irr::f32 z) const //Get height from global coordinates
{
    //Check down list, find highest number that does not return -FLT_MAX (or return -FLT_MAX if none)
    for (int i=(int)terrains.size()-1; i>=0; i--) {
        irr::f32 thisHeight = terrains.at(i)->getHeight(x,z);
        if (thisHeight > -FLT_MAX) {
            return thisHeight;
        }
    }

    //Fallback
    return -FLT_MAX;
}

irr::f32 Terrain::longToX(irr::f32 longitude) const
{
    return ((longitude - primeTerrainLong ) * (primeTerrainXWidth)) / primeTerrainLongExtent;
}

irr::f32 Terrain::latToZ(irr::f32 latitude) const
{
    return ((latitude - primeTerrainLat ) * (primeTerrainZWidth)) / primeTerrainLatExtent;
}

irr::f32 Terrain::xToLong(irr::f32 x) const{
    return primeTerrainLong + x*primeTerrainLongExtent/primeTerrainXWidth;
}

irr::f32 Terrain::zToLat(irr::f32 z) const{
    return primeTerrainLat + z*primeTerrainLatExtent/primeTerrainZWidth;
}

void Terrain::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    for (unsigned int i=0; i<terrains.size(); i++) {
        irr::core::vector3df currentPos = terrains.at(i)->getPosition();
        irr::f32 newPosX = currentPos.X + deltaX;
        irr::f32 newPosY = currentPos.Y + deltaY;
        irr::f32 newPosZ = currentPos.Z + deltaZ;
        terrains.at(i)->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
    }
}
