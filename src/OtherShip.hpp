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

#include "Ship.hpp"

#include "NavLight.hpp"
#include "Leg.hpp"

#include <cmath>
#include <vector>

// Forward declarations
class SimulationModel;
struct RadarData;
namespace irr { class IrrlichtDevice; namespace scene { class ISceneManager; class ITriangleSelector; } }

class OtherShip : public Ship
{
    public:
        OtherShip (const std::string& name, const std::string& internalName, const uint32_t& mmsi, const bc::graphics::Vec3& location, std::vector<Leg> legsLoaded, bool drifting, SimulationModel* model, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev);
        ~OtherShip();

        float getHeight() const;
        float getRCS() const;
        std::string getName() const;
        std::vector<Leg> getLegs() const;
        void changeLeg(int legNumber, float bearing, float speed, float distance, float scenarioTime);
        void addLeg(int afterLegNumber, float bearing, float speed, float distance, float scenarioTime);
        void deleteLeg(int legNumber, float scenarioTime);
        void resetLegs(float course, float speedKts, float distanceNm, float scenarioTime);
        RadarData getRadarData(bc::graphics::Vec3 scannerPosition) const;
        void update(float deltaTime, float scenarioTime, float tideHeight, uint32_t lightLevel);
        void enableTriangleSelector(bool selectorEnabled);
        void setRateOfTurn(float rateOfTurn); // This could be moved to Ship.hpp

    protected:
    private:

        SimulationModel* model;
        std::string name;
        std::vector<Leg> legs;
        std::vector<NavLight*> navLights;
        float height; //For radar
        float solidHeight; //For radar
        float rcs;
        float rateOfTurn;
        std::vector<Leg>::size_type findCurrentLeg(float scenarioTime);
        irr::scene::ITriangleSelector* selector;
        bool triangleSelectorEnabled;
        bool drifting;
};

#endif
