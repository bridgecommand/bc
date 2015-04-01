/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

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
        std::string getName() const;
        std::vector<Leg> getLegs() const;
        RadarData getRadarData(irr::core::vector3df scannerPosition) const;
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::core::vector3df viewPosition, irr::u32 lightLevel);

    protected:
    private:

        std::string name;
        std::vector<Leg> legs;
        std::vector<NavLight> navLights;
        irr::f32 length; //For radar calculation
        irr::f32 height; //For radar
        irr::f32 rcs;
};

#endif
