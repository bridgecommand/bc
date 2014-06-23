#ifndef __OTHERSHIP_HPP_INCLUDED__
#define __OTHERSHIP_HPP_INCLUDED__

#include "irrlicht.h"

#include "Ship.hpp"

#include "NavLight.hpp"
#include "Leg.hpp"

#include <cmath>
#include <vector>

class RadarData;

class OtherShip : public Ship
{
    public:
        OtherShip (const std::string& name,const irr::core::vector3df& location, std::vector<Leg> legsLoaded, irr::scene::ISceneManager* smgr);
        //virtual ~OtherShip();

        irr::f32 getLength() const;
        irr::f32 getHeight() const;
        irr::f32 getRCS() const;
        RadarData getRadarData(irr::core::vector3df scannerPosition) const;
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::core::vector3df viewPosition);

    protected:
    private:

        std::vector<Leg> legs;
        std::vector<NavLight> navLights;
        irr::f32 length; //For radar calculation
        irr::f32 height; //For radar
        irr::f32 rcs;
};

#endif
