#ifndef __NAVLIGHT_HPP_INCLUDED__
#define __NAVLIGHT_HPP_INCLUDED__

#include "irrlicht.h"

class NavLight {

    public:
        NavLight(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, irr::core::dimension2d<irr::f32> lightSize, irr::core::vector3df position, irr::video::SColor colour);
        ~NavLight();
        void Update();

    private:
        irr::scene::IBillboardSceneNode* lightNode;
        irr::f32 startAngle;
        irr::f32 endAngle;
        irr::f32 range;
};

#endif
