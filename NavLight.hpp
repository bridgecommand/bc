#ifndef __NAVLIGHT_HPP_INCLUDED__
#define __NAVLIGHT_HPP_INCLUDED__

#include "irrlicht.h"

class NavLight {

    public:
        NavLight(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, irr::core::dimension2d<irr::f32> lightSize, irr::core::vector3df position, irr::video::SColor colour, irr::f32 lightStartAngle, irr::f32 lightEndAngle, irr::f32 lightRange);
        ~NavLight();
        void update(irr::f32 scenarioTime, irr::core::vector3df viewPosition);

    private:
        irr::scene::IBillboardSceneNode* lightNode;
        irr::f32 startAngle;
        irr::f32 endAngle;
        irr::f32 range;
        bool isAngleBetween(irr::f32 angle, irr::f32 startAng, irr::f32 endAng);
        irr::f32 normaliseAngle(irr::f32 angle);
};

#endif
