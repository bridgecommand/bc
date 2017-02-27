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

using namespace irr;

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

    //Fixme: Note we're only loading the primary terrain here
    terrainLong = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLong",1));
    terrainLat = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLat",1));
    terrainLongExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLongExtent",1));
    terrainLatExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLatExtent",1));

    irr::f32 terrainMaxHeight=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainMaxHeight",1));
    irr::f32 seaMaxDepth=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("SeaMaxDepth",1));
    irr::f32 terrainHeightMapSize=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapSize",1));

    std::string heightMapName = IniFile::iniFileToString(worldTerrainFile, IniFile::enumerate1("HeightMap",1));
    std::string textureMapName = IniFile::iniFileToString(worldTerrainFile, IniFile::enumerate1("Texture",1));

    //Terrain dimensions in metres
    terrainXWidth = terrainLongExtent * 2.0 * PI * EARTH_RAD_M * cos( irr::core::degToRad(terrainLat + terrainLatExtent/2.0)) / 360.0;
    terrainZWidth = terrainLatExtent  * 2.0 * PI * EARTH_RAD_M / 360;

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

    /*terrain = smgr->addTerrainSceneNode(
                       heightMapPath.c_str(),//Height map file: Note this should normally be .png - avoid .bmp because of odd irrlicht handling.
                       0,					 // parent node
                       -1,					 // node id
		               core::vector3df(0.f, terrainY, 0.f),		// position
		               //core::vector3df(0.f, 180.f, 0.f),		// rotation (NOTE 180 deg rotation required for irrlicht terrain loading) - Also set in .moveNode method
                       core::vector3df(0.f, 0.f, 0.f),		// rotation (0 using modified irrlicht, which loads terrain 180deg rotated to standard irrlicht)
		               core::vector3df(scaleX,scaleY,scaleZ),	// scale
		               video::SColor ( 255, 255, 255, 255 ),	// vertexColor
		               5,					// maxLOD
		               scene::ETPS_17,		// patchSize
		               0					// smooth factor
                       );
    */
    //Add an empty terrain
    terrain = smgr->addTerrainSceneNode("",0,-1,core::vector3df(0.f, terrainY, 0.f),core::vector3df(0.f, 0.f, 0.f),core::vector3df(scaleX,scaleY,scaleZ),video::SColor(255,255,255,255),5,scene::ETPS_17,0,true);
    //Load the map
    io::IReadFile* heightMapFile = smgr->getFileSystem()->createAndOpenFile(heightMapPath.c_str());
    //Check the height map file has loaded and the terrain exists
    if (terrain==0 || heightMapFile == 0) {
        //Could not load terrain
        std::cout << "Could not load terrain." << std::endl;
        exit(EXIT_FAILURE);
    }

    //Load the terrain and check success
    bool loaded = false;
    //Check if extension is .f32 for binary floating point file
    std::string extension = "";
    if (heightMapName.length() > 3) {
        extension = heightMapName.substr(heightMapName.length() - 4,4);
        Utilities::to_lower(extension);
    }
    if (extension.compare(".f32") == 0 ) {
        //Binary file
        loaded = terrain->loadHeightMapRAW(heightMapFile,32,true,true);
        //Set scales etc to be 1.0, so heights are used directly
        terrain->setScale(core::vector3df(scaleX,1.0f,scaleZ));
        terrain->setPosition(core::vector3df(0.f, 0.f, 0.f));
    } else {
        loaded = terrain->loadHeightMap(heightMapFile);
    }

    if (!loaded) {
        //Could not load terrain
        //ToDo: Tell user that terrain couldn't be loaded
        std::cout << "Could not load terrain." << std::endl;
        exit(EXIT_FAILURE);
    }

    heightMapFile->drop();
    //TODO: Do we need to drop terrain?

    terrain->setMaterialFlag(video::EMF_FOG_ENABLE, true);
    terrain->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting
    //Todo: Anti-aliasing flag?
    terrain->setMaterialTexture(0, driver->getTexture(textureMapPath.c_str()));
}

irr::f32 Terrain::getHeight(irr::f32 x, irr::f32 z) const //Get height from global coordinates
{
    //irr::f32 localX = x-terrainXWidth; //Needed due to 180 degree rotation of the terrain
    //irr::f32 localZ = z-terrainZWidth; //Needed due to 180 degree rotation of the terrain
    //return terrain->getHeight(localX,localZ);
    return terrain->getHeight(x,z);
}

irr::f32 Terrain::longToX(irr::f32 longitude) const
{
    return ((longitude - terrainLong ) * (terrainXWidth)) / terrainLongExtent;
}

irr::f32 Terrain::latToZ(irr::f32 latitude) const
{
    return ((latitude - terrainLat ) * (terrainZWidth)) / terrainLatExtent;
}

irr::f32 Terrain::xToLong(irr::f32 x) const{
    return terrainLong + x*terrainLongExtent/terrainXWidth;
}

irr::f32 Terrain::zToLat(irr::f32 z) const{
    return terrainLat + z*terrainLatExtent/terrainZWidth;
}

void Terrain::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    core::vector3df currentPos = terrain->getPosition();
    irr::f32 newPosX = currentPos.X + deltaX;
    irr::f32 newPosY = currentPos.Y + deltaY;
    irr::f32 newPosZ = currentPos.Z + deltaZ;

    terrain->setPosition(core::vector3df(newPosX,newPosY,newPosZ));
}
