#ifndef __OTHERSHIP_HPP_INCLUDED__
#define __OTHERSHIP_HPP_INCLUDED__

#include "irrlicht.h"

class OtherShip
{
    public:
        OtherShip(const irr::io::path& filename, const irr::core::vector3df& location, const irr::f32 scale, irr::scene::ISceneManager* smgr);
        virtual ~OtherShip();
    protected:
    private:
        irr::scene::IMeshSceneNode* otherShip; //The scene node for the other ship.
};

#endif
