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

#include "OtherShips.hpp"

#include "irrlicht.h"
#include "Constants.hpp"
#include "OtherShip.hpp"
#include "IniFile.hpp"
#include "RadarData.hpp"
#include "SimulationModel.hpp"
#include "ScenarioDataStructure.hpp"

#include <iostream> //debugging

//using namespace irr;

namespace {
    inline irr::core::vector3df toIrrVec(const bc::graphics::Vec3& v) { return {v.x, v.y, v.z}; }
    inline bc::graphics::Vec3 fromIrrVec(const irr::core::vector3df& v) { return {v.X, v.Y, v.Z}; }
}

OtherShips::OtherShips()
{

}

OtherShips::~OtherShips()
{
    for(std::vector<OtherShip*>::iterator it = otherShips.begin(); it != otherShips.end(); ++it) {
        delete (*it);
    }
    otherShips.clear();
}

void OtherShips::load(std::vector<OtherShipData> otherShipsData, float scenarioStartTime, OperatingMode::Mode mode, irr::scene::ISceneManager* smgr, SimulationModel* model, irr::IrrlichtDevice* dev)
{

    //Store reference to model
    this->model = model;

    for(uint32_t i=0;i<otherShipsData.size();i++)
    {
        //Get ship type and construct filename
        std::string otherShipName = otherShipsData.at(i).shipName;
        //Get initial position
        float shipX = model->longToX(otherShipsData.at(i).initialLong);
        float shipZ = model->latToZ(otherShipsData.at(i).initialLat);

        //Set MMSI
        uint32_t mmsi = otherShipsData.at(i).mmsi;

        //Set if it's drifting with wind/stream
        bool drifting = otherShipsData.at(i).drifting;

        //Load leg information
        std::vector<Leg> legs;
        float legStartTime = scenarioStartTime;
        if (mode==OperatingMode::Normal) { //Only load leg information in normal mode
            for(uint32_t j=0; j<otherShipsData.at(i).legs.size(); j++){
                //go through each leg (if any), and load
                Leg currentLeg;
                currentLeg.bearing = otherShipsData.at(i).legs.at(j).bearing;
                currentLeg.speed = otherShipsData.at(i).legs.at(j).speed;
                currentLeg.startTime = legStartTime;

                //Use distance to calculate startTime of next leg, and stored for later reference.
                float distance = otherShipsData.at(i).legs.at(j).distance;
                currentLeg.distance = distance;

                legs.push_back(currentLeg);

                //find the start time for the next leg
                legStartTime = legStartTime + SECONDS_IN_HOUR*(distance/fabs(currentLeg.speed)); // nm/kts -> hours, so convert to seconds
            }
            //add a final 'stop' leg, which the ship will remain on after it has passed the other legs.
            Leg stopLeg;
            stopLeg.bearing=0;
            stopLeg.speed=0;
            stopLeg.distance=0;
            stopLeg.startTime = legStartTime;
            legs.push_back(stopLeg);
        }

        //Create otherShip and load into vector
        std::string internalName = "OtherShip_";
        internalName.append(std::to_string(i));
        otherShips.push_back(new OtherShip (otherShipName,internalName,mmsi,bc::graphics::Vec3(shipX,0.0f,shipZ),legs,drifting,model,smgr, dev));
    }

}

void OtherShips::update(float deltaTime, float scenarioTime, float tideHeight, uint32_t lightLevel, bc::graphics::Vec3 ownShipPosition, float ownShipLength)
{
    irr::core::vector3df irrOwnShipPos = toIrrVec(ownShipPosition);

    for(std::vector<OtherShip*>::iterator it = otherShips.begin(); it != otherShips.end(); ++it) {

        //Find local wave height
        bc::graphics::Vec3 prevPosition = (*it)->getPosition();
        float waveHeightFiltered = prevPosition.y - tideHeight - (*it)->getHeightCorrection(); //Calculate the previous wave height:

        //Apply up/down motion from waves, with some filtering
        float timeConstant = 0.5;//Time constant in s; TODO: Make dependent on vessel size
        float factor = deltaTime/(timeConstant+deltaTime);
        waveHeightFiltered = (1-factor) * waveHeightFiltered + factor*model->getWaveHeight(prevPosition.x,prevPosition.z); //TODO: Check implementation of simple filter!

        //Special case, if paused, just use the actual wave height. A bit of a bodge, but avoids having to store the previous filter value
        if (deltaTime == 0) {
            waveHeightFiltered = model->getWaveHeight(prevPosition.x,prevPosition.z);
        }

        (*it)->update(deltaTime, scenarioTime, tideHeight+waveHeightFiltered, lightLevel);

        //Set or clear triangle selector depending on distance from own ship
        if ((*it)->getSceneNode()->getAbsolutePosition().getDistanceFrom(irrOwnShipPos) < (ownShipLength + (*it)->getLength())) {
            (*it)->enableTriangleSelector(true);
        } else {
            (*it)->enableTriangleSelector(false);
        }
    }

}

void OtherShips::enableAllTriangleSelectors()
{
    for(std::vector<OtherShip*>::iterator it = otherShips.begin(); it != otherShips.end(); ++it) {
        // This will return to normal the next time OtherShips::update is called.
        (*it)->enableTriangleSelector(true);
    }
}

irr::scene::ISceneNode* OtherShips::getSceneNode(int number)
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getSceneNode();
    } else {
        return 0;
    }
}

RadarData OtherShips::getRadarData(uint32_t number, bc::graphics::Vec3 scannerPosition) const
//Get data for OtherShip (number) relative to scannerPosition
{
    RadarData radarData;

    if (number<=otherShips.size()) {
        radarData = otherShips[number-1]->getRadarData(scannerPosition);
    }
    return radarData;
}

uint32_t OtherShips::getNumber() const
{
    return otherShips.size();
}

bc::graphics::Vec3 OtherShips::getPosition(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getPosition();
    } else {
        return bc::graphics::Vec3(0,0,0);
    }
}

float OtherShips::getLength(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getLength();
    } else {
        return 0.0;
    }
}

float OtherShips::getBreadth(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getBreadth();
    } else {
        return 0.0;
    }
}

float OtherShips::getHeading(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getHeading();
    } else {
        return 0;
    }
}

float OtherShips::getSpeed(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getSpeed();
    } else {
        return 0;
    }
}

float OtherShips::getEstimatedDisplacement(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getEstimatedDisplacement();
    } else {
        return 0;
    }
}

void OtherShips::setSpeed(int number, float speed)
{
    if (number < (int)otherShips.size() && number >= 0) {
        otherShips.at(number)->setSpeed(speed);
    }
}

uint32_t OtherShips::getMMSI(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getMMSI();
    } else {
        return 0;
    }
}

void OtherShips::setMMSI(int number, uint32_t mmsi)
{
    if (number < (int)otherShips.size() && number >= 0) {
        otherShips.at(number)->setMMSI(mmsi);
    }
}

void OtherShips::setPos(int number, float positionX, float positionZ)
{
    if (number < (int)otherShips.size() && number >= 0) {
        otherShips.at(number)->setPosition(positionX,positionZ);
    }
}

void OtherShips::setHeading(int number, float hdg)
{
    if (number < (int)otherShips.size() && number >= 0) {
        otherShips.at(number)->setHeading(hdg);
    }
}

void OtherShips::setRateOfTurn(int number, float rateOfTurn)
{
    if (number < (int)otherShips.size() && number >= 0) {
        otherShips.at(number)->setRateOfTurn(rateOfTurn);
    }
}

std::vector<Leg> OtherShips::getLegs(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getLegs();
    } else {
        //Return an empty vector
        std::vector<Leg> legs;
        return legs;
    }
}

void OtherShips::changeLeg(int shipNumber, int legNumber, float bearing, float speed, float distance, float scenarioTime)
{
    //Check if ship exists
    if (shipNumber < (int)otherShips.size() && shipNumber >= 0) {
        otherShips.at(shipNumber)->changeLeg(legNumber, bearing, speed, distance, scenarioTime);
    }
}

void OtherShips::addLeg(int shipNumber, int afterLegNumber, float bearing, float speed, float distance, float scenarioTime)
{
    //Check if ship exists
    if (shipNumber < (int)otherShips.size() && shipNumber >= 0) {
        otherShips.at(shipNumber)->addLeg(afterLegNumber, bearing, speed, distance, scenarioTime);
    }
}

void OtherShips::deleteLeg(int shipNumber, int legNumber, float scenarioTime)
{
    //Check if ship exists
    if (shipNumber < (int)otherShips.size() && shipNumber >= 0) {
        otherShips.at(shipNumber)->deleteLeg(legNumber, scenarioTime);
    }
}

void OtherShips::resetLegs(int shipNumber, float course, float speedKts, float distanceNm, float scenarioTime)
{
    //Check if ship exists
    if (shipNumber < (int)otherShips.size() && shipNumber >= 0) {
        otherShips.at(shipNumber)->resetLegs(course, speedKts, distanceNm, scenarioTime);
    }
}

std::string OtherShips::getName(int number) const
{
    if(number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getName();
    } else {
        return "";
    }
}

void OtherShips::moveNode(float deltaX, float deltaY, float deltaZ)
{
    for(std::vector<OtherShip*>::iterator it = otherShips.begin(); it != otherShips.end(); ++it) {
        (*it)->moveNode(deltaX,deltaY,deltaZ);
    }
}
