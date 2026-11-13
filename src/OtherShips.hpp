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
        void load(std::vector<OtherShipData> otherShipsData, float scenarioStartTime, OperatingMode::Mode mode, irr::scene::ISceneManager* smgr, SimulationModel* model, irr::IrrlichtDevice* dev);
        void update(float deltaTime, float scenarioTime, float tideHeight, unsigned int lightLevel, irr::core::vector3df ownShipPosition, float ownShipLength);
        RadarData getRadarData(unsigned int number, irr::core::vector3df scannerPosition) const;
        unsigned int getNumber() const;
        irr::core::vector3df getPosition(int number) const;
        float getLength(int number) const;
        float getBreadth(int number) const;
        float getHeading(int number) const;
        float getSpeed(int number) const; //Speed in m/s
        unsigned int getMMSI(int number) const;
        float getEstimatedDisplacement(int number) const;
        bool getSARTOn(int number) const;
        void setSARTOn(int number, bool sartState);
        void setSpeed(int number, float speed); //Speed in m/s
        void setMMSI(int number, unsigned int mmsi);
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
        void offsetLegTimes(float deltaTime);
        void enableAllTriangleSelectors();
        irr::scene::ISceneNode* getSceneNode(int number);

    private:
        std::vector<OtherShip*> otherShips;
        SimulationModel* model;
};

#endif
