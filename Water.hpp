#ifndef __WATER_HPP_INCLUDED__
#define __WATER_HPP_INCLUDED__

#include "irrlicht.h"

class Water
{
    public:
        Water(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver);
        virtual ~Water();

    private:
        irr::scene::ISceneNode* waterNode;
};

#endif
