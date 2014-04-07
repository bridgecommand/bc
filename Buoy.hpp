#ifndef __BUOY_HPP_INCLUDED__
#define __BUOY_HPP_INCLUDED__

#include "irrlicht.h"

class Buoy
{
    public:
        Buoy(const irr::io::path&, const irr::core::vector3df&, irr::scene::ISceneManager*);
        virtual ~Buoy();
    protected:
    private:
        irr::scene::IMeshSceneNode* buoy; //The scene node for the buoy.
};

#endif // __BUOY_HPP_INCLUDED__
