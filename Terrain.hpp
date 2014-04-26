#ifndef __TERRAIN_HPP_INCLUDED__
#define __TERRAIN_HPP_INCLUDED__

#include "irrlicht.h"

class Terrain
{
    public:
        Terrain();
        virtual ~Terrain();
        void loadTerrain(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver);
        irr::f32 getHeight(irr::f32 x, irr::f32 z);

    private:
        irr::scene::ITerrainSceneNode* terrain;
};

#endif
