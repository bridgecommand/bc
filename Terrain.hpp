#ifndef __TERRAIN_HPP_INCLUDED__
#define __TERRAIN_HPP_INCLUDED__

#include "irrlicht.h"

class Terrain
{
    public:
        Terrain();
        virtual ~Terrain();
        void loadTerrain(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver);

    private:
        irr::scene::ITerrainSceneNode* terrain;
};

#endif
