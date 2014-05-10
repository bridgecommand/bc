#ifndef __RADARSCREEN_HPP_INCLUDED__
#define __RADARSCREEN_HPP_INCLUDED__

#include "irrlicht.h"

class RadarScreen
{
    public:
        RadarScreen();
        virtual ~RadarScreen();

        void load(irr::video::IVideoDriver* drv, irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* par, irr::core::vector3df off);
        void update(irr::video::IImage* radarImage);


    private:
        irr::video::IVideoDriver* driver;
        irr::scene::IBillboardSceneNode* radarScreen;
        irr::scene::ISceneNode* parent;
        irr::core::vector3df offset;
};

#endif
