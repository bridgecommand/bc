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

#ifndef __OTHERSHIPS_HPP_INCLUDED__
#define __OTHERSHIPS_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>

#include "Leg.hpp"

//Forward declarations
class SimulationModel;
class OtherShip;
class RadarData;

class OtherShips
{
    public:
        OtherShips();
        virtual ~OtherShips();
        void load(const std::string& scenarioName, irr::f32 scenarioStartTime, irr::scene::ISceneManager* smgr, SimulationModel* model);
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::core::vector3df viewPosition, irr::u32 lightLevel);
        RadarData getRadarData(irr::u32 number, irr::core::vector3df scannerPosition) const;
        irr::u32 getNumber() const;
        irr::core::vector3df getPosition(int number) const;
        irr::f32 getHeading(int number) const;
        std::vector<Leg> getLegs(int number) const;
        void changeLeg(int shipNumber, int legNumber, irr::f32 bearing, irr::f32 speed, irr::f32 distance, irr::f32 scenarioTime);
        std::string getName(int number) const;
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);

    private:
        std::vector<OtherShip> otherShips;
};

#endif
