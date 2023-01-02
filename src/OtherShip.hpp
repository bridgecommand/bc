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

struct RadarData;

class OtherShip : public Ship
{
    public:
        OtherShip (const std::string& name, const irr::u32& mmsi, const irr::core::vector3df& location, std::vector<Leg> legsLoaded, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev);
        ~OtherShip();

        irr::f32 getHeight() const;
        irr::f32 getRCS() const;
        std::string getName() const;
        std::vector<Leg> getLegs() const;
        void changeLeg(int legNumber, irr::f32 bearing, irr::f32 speed, irr::f32 distance, irr::f32 scenarioTime);
        void addLeg(int afterLegNumber, irr::f32 bearing, irr::f32 speed, irr::f32 distance, irr::f32 scenarioTime);
        void deleteLeg(int legNumber, irr::f32 scenarioTime);
        void resetLegs(irr::f32 course, irr::f32 speedKts, irr::f32 distanceNm, irr::f32 scenarioTime);
        RadarData getRadarData(irr::core::vector3df scannerPosition) const;
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::u32 lightLevel);
        void enableTriangleSelector(bool selectorEnabled);
        void setRateOfTurn(irr::f32 rateOfTurn); // This could be moved to Ship.hpp

    protected:
    private:

        std::string name;
        std::vector<Leg> legs;
        std::vector<NavLight*> navLights;
        irr::f32 height; //For radar
        irr::f32 solidHeight; //For radar
        irr::f32 rcs;
        irr::f32 rateOfTurn;
        std::vector<Leg>::size_type findCurrentLeg(irr::f32 scenarioTime);
        irr::scene::ITriangleSelector* selector;
        bool triangleSelectorEnabled;
};

#endif
