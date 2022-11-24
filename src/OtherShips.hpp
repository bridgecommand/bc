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
#include "OperatingModeEnum.hpp"

//Forward declarations
class SimulationModel;
class OtherShip;
struct RadarData;
class OtherShipData;

class OtherShips
{
    public:
        OtherShips();
        ~OtherShips();
        void load(std::vector<OtherShipData> otherShipsData, irr::f32 scenarioStartTime, OperatingMode::Mode mode, irr::scene::ISceneManager* smgr, SimulationModel* model, irr::IrrlichtDevice* dev);
        void update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::u32 lightLevel, irr::core::vector3df ownShipPosition, irr::f32 ownShipLength);
        RadarData getRadarData(irr::u32 number, irr::core::vector3df scannerPosition) const;
        irr::u32 getNumber() const;
        irr::core::vector3df getPosition(int number) const;
        irr::f32 getLength(int number) const;
        irr::f32 getWidth(int number) const;
        irr::f32 getHeading(int number) const;
        irr::f32 getSpeed(int number) const; //Speed in m/s
        irr::u32 getMMSI(int number) const;
        void setSpeed(int number, irr::f32 speed); //Speed in m/s
        void setMMSI(int number, irr::u32 mmsi);
        void setPos(int number, irr::f32 positionX, irr::f32 positionZ);
        void setHeading(int number, irr::f32 hdg);
        void setRateOfTurn(int number, irr::f32 rateOfTurn);
        std::vector<Leg> getLegs(int number) const;
        void changeLeg(int shipNumber, int legNumber, irr::f32 bearing, irr::f32 speed, irr::f32 distance, irr::f32 scenarioTime);
        void addLeg(int shipNumber, int afterLegNumber, irr::f32 bearing, irr::f32 speed, irr::f32 distance, irr::f32 scenarioTime);
        void deleteLeg(int shipNumber, int legNumber, irr::f32 scenarioTime);
        void resetLegs(int shipNumber, irr::f32 course, irr::f32 speedKts, irr::f32 distanceNm, irr::f32 scenarioTime);
        std::string getName(int number) const;
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);

    private:
        std::vector<OtherShip*> otherShips;
        SimulationModel* model;
};

#endif
