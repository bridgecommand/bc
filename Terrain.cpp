#include "irrlicht.h"

#include "Terrain.hpp"

using namespace irr;

Terrain::Terrain()
{

}

Terrain::~Terrain()
{
    //dtor
}

void Terrain::load(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver)
{
    terrain = smgr->addTerrainSceneNode(
                       "World/SimpleEstuary/heightRotated.bmp", //FIXME: Heightmap image manually rotated
                       0,					// parent node
                       -1,					// node id
		               core::vector3df(0.f, -44.07f, 0.f),		// position
		               core::vector3df(0.f, 0.0*180.f, 0.f),		// rotation (NOTE 180 deg rotation) (FIXME:Disabled for now)
		               core::vector3df(6.97705f, 0.56498f, 8.6871f),	// scale
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
    f32 terrainLong = -10.0; //FIXME: Hardcoding - these should all be member variables, set on terrain load
    f32 terrainXWidth = 3572.25;
    f32 terrainLongExtent = 0.05;
    return ((longitude - terrainLong ) * (terrainXWidth)) / terrainLongExtent;
}

irr::f32 Terrain::latToZ(irr::f32 latitude) const
{
    f32 terrainLat = 50.0; //FIXME: Hardcoding - these should all be member variables, set on terrain load
    f32 terrainZWidth = 4447.8;
    f32 terrainLatExtent = 0.04;
    return ((latitude - terrainLat ) * (terrainZWidth)) / terrainLatExtent;
}
