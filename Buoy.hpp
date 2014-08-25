#ifndef __BUOY_HPP_INCLUDED__
#define __BUOY_HPP_INCLUDED__

#include "irrlicht.h"

#include <string>
#include <cmath>

class RadarData;

class Buoy
{
    public:
        Buoy(const std::string& name, const irr::core::vector3df& location, irr::f32 radarCrossSection, irr::scene::ISceneManager* smgr);
        virtual ~Buoy();
        irr::core::vector3df getPosition() const;
        void setPosition(irr::core::vector3df position);
        irr::f32 getLength() const;
        irr::f32 getHeight() const;
        irr::f32 getRCS() const;
        RadarData getRadarData(irr::core::vector3df scannerPosition) const;
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);
    protected:
    private:
        irr::scene::IMeshSceneNode* buoy; //The scene node for the buoy.
        irr::f32 length; //For radar calculation
        irr::f32 height; //For radar calculation
        irr::f32 rcs;
};

#endif // __BUOY_HPP_INCLUDED__
