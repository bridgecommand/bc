#ifndef __OTHERSHIP_HPP_INCLUDED__
#define __OTHERSHIP_HPP_INCLUDED__

#include "irrlicht.h"

#include "Leg.hpp"

#include <vector>

class OtherShip
{
    public:
        OtherShip(const irr::io::path& filename, const irr::core::vector3df& location, const irr::f32 scaleFactor, const irr::f32 yCorrection, std::vector<Leg> legsLoaded, irr::scene::ISceneManager* smgr);
        virtual ~OtherShip();
        void update();
    protected:
    private:
        irr::scene::IMeshSceneNode* otherShip; //The scene node for the other ship.
        std::vector<Leg> legs;
};

#endif
