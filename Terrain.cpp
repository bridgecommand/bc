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

    std::string heightMapName = IniFile::iniFileToString(worldTerrainFile, IniFile::enumerate1("HeightMap",1));
    std::string textureMapName = IniFile::iniFileToString(worldTerrainFile, IniFile::enumerate1("Texture",1));

    //Terrain dimensions in metres
    terrainXWidth = terrainLongExtent * 2.0 * PI * EARTH_RAD_M * cos( irr::core::degToRad(terrainLat + terrainLatExtent/2.0)) / 360.0;
    terrainZWidth = terrainLatExtent  * 2.0 * PI * EARTH_RAD_M / 360;

    //calculations just needed for terrain loading
    irr::f32 scaleX = terrainXWidth / (terrainHeightMapSize-1);//Fixme: Check this for new 2^n+1 terrains
    irr::f32 scaleY = (terrainMaxHeight + seaMaxDepth)/255.0;
    irr::f32 scaleZ = terrainZWidth / (terrainHeightMapSize-1);//Fixme: Check this for new 2^n+1 terrains
    irr::f32 terrainY = -1*seaMaxDepth;

    //Full paths
    std::string heightMapPath = worldPath;
    heightMapPath.append("/");
    heightMapPath.append(heightMapName);
    std::string textureMapPath = worldPath;
    textureMapPath.append("/");
    textureMapPath.append(textureMapName);

    //Fixme: Could also check that the terrain is now 2^n + 1 square (was 2^n in B3d version)

    terrain = smgr->addTerrainSceneNode(
                       heightMapPath.c_str(), //FIXME: Load from specified file
                       0,					// parent node
                       -1,					// node id
		               core::vector3df(0.f, terrainY, 0.f),		// position
		               core::vector3df(0.f, 180.f, 0.f),		// rotation (NOTE 180 deg rotation required for irrlicht terrain loading)
		               core::vector3df(scaleX,scaleY,scaleZ),	// scale
		               video::SColor ( 255, 255, 255, 255 ),	// vertexColor
		               5,					// maxLOD
		               scene::ETPS_17,		// patchSize
		               4					// smoothFactoespr
                       );

    terrain->setMaterialFlag(video::EMF_FOG_ENABLE, true);
    terrain->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting
    terrain->setMaterialTexture(0, driver->getTexture(textureMapPath.c_str())); //FIXME: Load from specified file

}

irr::f32 Terrain::getHeight(irr::f32 x, irr::f32 z) const //Get height from global coordinates
{
    irr::f32 localX = x-terrainXWidth; //Needed due to 180 degree rotation of the terrain
    irr::f32 localZ = z-terrainZWidth; //Needed due to 180 degree rotation of the terrain
    return terrain->getHeight(localX,localZ);
}

irr::f32 Terrain::longToX(irr::f32 longitude) const
{
    return ((longitude - terrainLong ) * (terrainXWidth)) / terrainLongExtent;
}

irr::f32 Terrain::latToZ(irr::f32 latitude) const
{
    return ((latitude - terrainLat ) * (terrainZWidth)) / terrainLatExtent;
}
