#include "irrlicht.h"

#include "Terrain.hpp"

#include "IniFile.hpp"
#include "Constants.hpp"

#include <iostream>

using namespace irr;

Terrain::Terrain()
{

}

Terrain::~Terrain()
{
    //dtor
}

void Terrain::load(const std::string& worldPath, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver)
{

    //Get full path to the main Terrain.ini file
    std::string worldTerrainFile = worldPath;
    worldTerrainFile.append("/Terrain.ini");

    //Fixme: Note we're only loading the primary terrain here
    terrainLong = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLong",1));
    terrainLat = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLat",1));
    terrainLongExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLongExtent",1));
    terrainLatExtent = IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainLatExtent",1));

    irr::f32 terrainMaxHeight=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainMaxHeight",1));
    irr::f32 seaMaxDepth=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("SeaMaxDepth",1));
    irr::f32 terrainHeightMapSize=IniFile::iniFileTof32(worldTerrainFile, IniFile::enumerate1("TerrainHeightMapSize",1));

    //Terrain dimensions in metres
    terrainXWidth = terrainLongExtent * 2.0 * PI * EARTH_RAD_M * cos( irr::core::degToRad(terrainLat + terrainLatExtent/2.0)) / 360.0;
    terrainZWidth = terrainLatExtent  * 2.0 * PI * EARTH_RAD_M / 360;

    //calculations just needed for terrain loading
    irr::f32 scaleX = terrainXWidth / (terrainHeightMapSize-1);//Fixme: Check this for new 2^n+1 terrains
    irr::f32 scaleY = (terrainMaxHeight + seaMaxDepth)/255.0;
    irr::f32 scaleZ = terrainZWidth / (terrainHeightMapSize-1);//Fixme: Check this for new 2^n+1 terrains
    irr::f32 terrainY = -1*seaMaxDepth;

    //Fixme: Need to use path names for terrain height and texture maps, and
    //work out how to fix their rotation: Do we manually rotate files each time,
    //and save in temp location. Could also use this to ensure that file is 2^n + 1 square.

    terrain = smgr->addTerrainSceneNode(
                       "World/SimpleEstuary/heightRotated.bmp", //FIXME: Heightmap image manually rotated
                       0,					// parent node
                       -1,					// node id
		               core::vector3df(0.f, terrainY, 0.f),		// position
		               core::vector3df(0.f, 0*180.f, 0.f),		// rotation (NOTE 180 deg rotation) (FIXME:Disabled for now)
		               core::vector3df(scaleX,scaleY,scaleZ),	// scale
		               video::SColor ( 255, 255, 255, 255 ),	// vertexColor
		               5,					// maxLOD
		               scene::ETPS_17,		// patchSize
		               4					// smoothFactoespr
                       );

    terrain->setMaterialFlag(video::EMF_FOG_ENABLE, true);
    terrain->setMaterialTexture(0, driver->getTexture("World/SimpleEstuary/textureRotated.bmp")); //FIXME: Texture image manually rotated

}

irr::f32 Terrain::getHeight(irr::f32 x, irr::f32 z) const //Get height from global coordinates
{
    return terrain->getHeight(x,z); //To do: Account for 180 degree rotation required in terrain
}

irr::f32 Terrain::longToX(irr::f32 longitude) const
{
    return ((longitude - terrainLong ) * (terrainXWidth)) / terrainLongExtent;
}

irr::f32 Terrain::latToZ(irr::f32 latitude) const
{
    return ((latitude - terrainLat ) * (terrainZWidth)) / terrainLatExtent;
}
