#ifndef __WATER_HPP_INCLUDED__
#define __WATER_HPP_INCLUDED__

#include "irrlicht.h"

class Water
{
    public:
        Water();
        virtual ~Water();
        void load(irr::scene::ISceneManager* smgr);
        void update(irr::core::vector3df viewPosition);

    private:
        irr::scene::ISceneNode* waterNode;
        irr::f32 tileWidth;
        irr::s32 round(irr::f32 numberIn);
};

#endif

