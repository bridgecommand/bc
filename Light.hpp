#ifndef __LIGHT_HPP_INCLUDED__
#define __LIGHT_HPP_INCLUDED__

#include "irrlicht.h"

class Light
{
    public:
        Light();
        virtual ~Light();
        void load(irr::scene::ISceneManager* smgr);
        void update(irr::f32 scenarioTime);
        irr::video::SColor getLightSColor();

    private:
        irr::video::SColor ambientColor;
        irr::scene::ISceneManager* smgr;
};

#endif // __LIGHT_HPP_INCLUDED__
