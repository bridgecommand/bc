#ifndef __TERRAIN_HPP_INCLUDED__
#define __TERRAIN_HPP_INCLUDED__

#include "irrlicht.h"

class Terrain
{
    public:
        Terrain(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver);
        virtual ~Terrain();

    private:
        irr::scene::ITerrainSceneNode* terrain;
};

#endif
