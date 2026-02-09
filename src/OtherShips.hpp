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

#include "graphics/Types.hpp"

#include <vector>
#include <string>

#include "Leg.hpp"
#include "OperatingModeEnum.hpp"

//Forward declarations
class SimulationModel;
class OtherShip;
struct RadarData;
class OtherShipData;
namespace irr {
    class IrrlichtDevice;
    namespace scene { class ISceneManager; class ISceneNode; }
}

class OtherShips
{
    public:
        OtherShips();
        ~OtherShips();
        void load(std::vector<OtherShipData> otherShipsData, float scenarioStartTime, OperatingMode::Mode mode, irr::scene::ISceneManager* smgr, SimulationModel* model, irr::IrrlichtDevice* dev);
        void update(float deltaTime, float scenarioTime, float tideHeight, uint32_t lightLevel, bc::graphics::Vec3 ownShipPosition, float ownShipLength);
        RadarData getRadarData(uint32_t number, bc::graphics::Vec3 scannerPosition) const;
        uint32_t getNumber() const;
        bc::graphics::Vec3 getPosition(int number) const;
        float getLength(int number) const;
        float getBreadth(int number) const;
        float getHeading(int number) const;
        float getSpeed(int number) const; //Speed in m/s
        uint32_t getMMSI(int number) const;
        float getEstimatedDisplacement(int number) const;
        void setSpeed(int number, float speed); //Speed in m/s
        void setMMSI(int number, uint32_t mmsi);
        void setPos(int number, float positionX, float positionZ);
        void setHeading(int number, float hdg);
        void setRateOfTurn(int number, float rateOfTurn);
        std::vector<Leg> getLegs(int number) const;
        void changeLeg(int shipNumber, int legNumber, float bearing, float speed, float distance, float scenarioTime);
        void addLeg(int shipNumber, int afterLegNumber, float bearing, float speed, float distance, float scenarioTime);
        void deleteLeg(int shipNumber, int legNumber, float scenarioTime);
        void resetLegs(int shipNumber, float course, float speedKts, float distanceNm, float scenarioTime);
        std::string getName(int number) const;
        void moveNode(float deltaX, float deltaY, float deltaZ);
        void enableAllTriangleSelectors();
        irr::scene::ISceneNode* getSceneNode(int number);

    private:
        std::vector<OtherShip*> otherShips;
        SimulationModel* model;
};

#endif
