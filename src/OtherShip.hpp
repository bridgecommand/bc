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
#include "DayShape.hpp"
#include "Leg.hpp"

#include <cmath>
#include <vector>
#include <cstdint> // for uint64_t

// Forward declarations
class SimulationModel;
struct RadarData;

class OtherShip : public Ship
{
    public:
        OtherShip (const std::string& name, const std::string& internalName, const irr::u32& mmsi, const irr::core::vector3df& location, std::vector<Leg> legsLoaded, bool drifting, bool SART, SimulationModel* model, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev);
        ~OtherShip();

        float getHeight() const;
        float getRCS() const;
        std::string getName() const;
        std::vector<Leg> getLegs() const;
        void changeLeg(int legNumber, float bearing, float speed, float distance, float scenarioTime);
        void addLeg(int afterLegNumber, float bearing, float speed, float distance, float scenarioTime);
        void deleteLeg(int legNumber, float scenarioTime);
        void resetLegs(float course, float speedKts, float distanceNm, float scenarioTime);
        void offsetLegTimes(float deltaTime);
        RadarData getRadarData(irr::core::vector3df scannerPosition) const;
        void update(float deltaTime, float scenarioTime, float tideHeight, irr::u32 lightLevel);
        void enableTriangleSelector(bool selectorEnabled);
        void setRateOfTurn(float rateOfTurn); // This could be moved to Ship.hpp
        void setSARTtimeStamp(uint64_t timeStamp);
        bool getSARTOn() const;
        void setSARTOn(bool sartState);

    protected:
    private:

        SimulationModel* model;
        std::string name;
        std::vector<Leg> legs;
        std::vector<NavLight*> navLights;
        std::vector<DayShape*> dayShapes;
        float height; //For radar
        float solidHeight; //For radar
        float rcs;
        float rateOfTurn;
        std::vector<Leg>::size_type findCurrentLeg(float scenarioTime);
        irr::scene::ITriangleSelector* selector;
        bool triangleSelectorEnabled;
        bool drifting;
        bool SART; // If SART is enabled
        uint64_t SARTtimeStamp;
};

#endif
