#ifndef __TERRAIN_HPP_INCLUDED__
#define __TERRAIN_HPP_INCLUDED__

#include "irrlicht.h"

#include <string>

class Terrain
{
    public:
        Terrain();
        virtual ~Terrain();
        void load(const std::string& worldPath, irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver);
        irr::f32 longToX(irr::f32 longitude) const;
        irr::f32 latToZ(irr::f32 latitude) const;
        irr::f32 getHeight(irr::f32 x, irr::f32 z) const;

    private:
        irr::scene::ITerrainSceneNode* terrain;
        irr::f32 terrainLong;
        irr::f32 terrainXWidth;
        irr::f32 terrainLongExtent;
        irr::f32 terrainLat;
        irr::f32 terrainZWidth;
        irr::f32 terrainLatExtent;
};

#endif
