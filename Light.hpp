#ifndef __LIGHT_HPP_INCLUDED__
#define __LIGHT_HPP_INCLUDED__

#include "irrlicht.h"

class Light
{
    public:
        Light();
        virtual ~Light();
        void load(irr::scene::ISceneManager* smgr);
        void update();
        irr::video::SColor getLightSColor();

    private:
        irr::video::SColor ambientColor;
        irr::scene::ISceneManager* scene;
};

#endif // __LIGHT_HPP_INCLUDED__
