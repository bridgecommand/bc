#ifndef __TERRAIN_HPP_INCLUDED__
#define __TERRAIN_HPP_INCLUDED__

#include "irrlicht.h"

class Terrain
{
    public:
        Terrain();
        virtual ~Terrain();
        void load(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver);
        irr::f32 longToX(irr::f32 longitude) const;
        irr::f32 latToZ(irr::f32 latitude) const;
        irr::f32 getHeight(irr::f32 x, irr::f32 z) const;

    private:
        irr::scene::ITerrainSceneNode* terrain;
};

#endif
