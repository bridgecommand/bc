#ifndef __BUOY_HPP_INCLUDED__
#define __BUOY_HPP_INCLUDED__

#include "irrlicht.h"

#include <string>
#include <cmath>

class Buoy
{
    public:
        Buoy(const std::string& name, const irr::core::vector3df& location, irr::f32 radarCrossSection, irr::scene::ISceneManager* smgr);
        virtual ~Buoy();
        irr::core::vector3df getPosition() const;
        irr::f32 getLength() const;
        irr::f32 getHeight() const;
        irr::f32 getRCS() const;
    protected:
    private:
        irr::scene::IMeshSceneNode* buoy; //The scene node for the buoy.
        irr::f32 length; //For radar calculation
        irr::f32 height; //For radar calculation
        irr::f32 rcs;
};

#endif // __BUOY_HPP_INCLUDED__
