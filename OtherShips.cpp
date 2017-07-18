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

#include "Constants.hpp"
#include "OtherShip.hpp"
#include "IniFile.hpp"
#include "RadarData.hpp"
#include "SimulationModel.hpp"
#include "ScenarioDataStructure.hpp"

#include <iostream> //debugging

using namespace irr;

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

void OtherShips::load(std::vector<OtherShipData> otherShipsData, irr::f32 scenarioStartTime, OperatingMode::Mode mode, irr::scene::ISceneManager* smgr, SimulationModel* model, irr::IrrlichtDevice* dev)
{

    //Store reference to model
    this->model = model;

    for(u32 i=0;i<otherShipsData.size();i++)
    {
        //Get ship type and construct filename
        std::string otherShipName = otherShipsData.at(i).shipName;
        //Get initial position
        f32 shipX = model->longToX(otherShipsData.at(i).initialLong);
        f32 shipZ = model->latToZ(otherShipsData.at(i).initialLat);

        //Load leg information
        std::vector<Leg> legs;
        irr::f32 legStartTime = scenarioStartTime;
        if (mode==OperatingMode::Normal) { //Only load leg information in normal mode
            for(irr::u32 j=0; j<otherShipsData.at(i).legs.size(); j++){
                //go through each leg (if any), and load
                Leg currentLeg;
                currentLeg.bearing = otherShipsData.at(i).legs.at(j).bearing;
                currentLeg.speed = otherShipsData.at(i).legs.at(j).speed;
                currentLeg.startTime = legStartTime;

                //Use distance to calculate startTime of next leg, and stored for later reference.
                irr::f32 distance = otherShipsData.at(i).legs.at(j).distance;
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
        otherShips.push_back(new OtherShip (otherShipName,core::vector3df(shipX,0.0f,shipZ),legs,smgr, dev));
    }

}

void OtherShips::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::u32 lightLevel)
{
    for(std::vector<OtherShip*>::iterator it = otherShips.begin(); it != otherShips.end(); ++it) {

        //Find local wave height
        irr::core::vector3df prevPosition = (*it)->getPosition();
        f32 waveHeightFiltered = prevPosition.Y - tideHeight - (*it)->getHeightCorrection(); //Calculate the previous wave height:

        //Apply up/down motion from waves, with some filtering
        f32 timeConstant = 0.5;//Time constant in s; TODO: Make dependent on vessel size
        f32 factor = deltaTime/(timeConstant+deltaTime);
        waveHeightFiltered = (1-factor) * waveHeightFiltered + factor*model->getWaveHeight(prevPosition.X,prevPosition.Z); //TODO: Check implementation of simple filter!

        (*it)->update(deltaTime, scenarioTime, tideHeight+waveHeightFiltered, lightLevel);
    }

}

RadarData OtherShips::getRadarData(irr::u32 number, irr::core::vector3df scannerPosition) const
//Get data for OtherShip (number) relative to scannerPosition
{
    RadarData radarData;

    if (number<=otherShips.size()) {
        radarData = otherShips[number-1]->getRadarData(scannerPosition);
    }
    return radarData;
}

irr::u32 OtherShips::getNumber() const
{
    return otherShips.size();
}

irr::core::vector3df OtherShips::getPosition(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getPosition();
    } else {
        return core::vector3df(0,0,0);
    }
}

irr::f32 OtherShips::getLength(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getLength();
    } else {
        return 0.0;
    }
}

irr::f32 OtherShips::getWidth(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getWidth();
    } else {
        return 0.0;
    }
}

irr::f32 OtherShips::getHeading(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getHeading();
    } else {
        return 0;
    }
}

irr::f32 OtherShips::getSpeed(int number) const
{
    if (number < (int)otherShips.size() && number >= 0) {
        return otherShips.at(number)->getSpeed();
    } else {
        return 0;
    }
}

void OtherShips::setSpeed(int number, irr::f32 speed)
{
    if (number < (int)otherShips.size() && number >= 0) {
        otherShips.at(number)->setSpeed(speed);
    }
}

void OtherShips::setPos(int number, irr::f32 positionX, irr::f32 positionZ)
{
    if (number < (int)otherShips.size() && number >= 0) {
        otherShips.at(number)->setPosition(positionX,positionZ);
    }
}

void OtherShips::setHeading(int number, irr::f32 hdg)
{
    if (number < (int)otherShips.size() && number >= 0) {
        otherShips.at(number)->setHeading(hdg);
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

void OtherShips::changeLeg(int shipNumber, int legNumber, irr::f32 bearing, irr::f32 speed, irr::f32 distance, irr::f32 scenarioTime)
{
    //Check if ship exists
    if (shipNumber < (int)otherShips.size() && shipNumber >= 0) {
        otherShips.at(shipNumber)->changeLeg(legNumber, bearing, speed, distance, scenarioTime);
    }
}

void OtherShips::addLeg(int shipNumber, int afterLegNumber, irr::f32 bearing, irr::f32 speed, irr::f32 distance, irr::f32 scenarioTime)
{
    //Check if ship exists
    if (shipNumber < (int)otherShips.size() && shipNumber >= 0) {
        otherShips.at(shipNumber)->addLeg(afterLegNumber, bearing, speed, distance, scenarioTime);
    }
}

void OtherShips::deleteLeg(int shipNumber, int legNumber, irr::f32 scenarioTime)
{
    //Check if ship exists
    if (shipNumber < (int)otherShips.size() && shipNumber >= 0) {
        otherShips.at(shipNumber)->deleteLeg(legNumber, scenarioTime);
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

void OtherShips::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    for(std::vector<OtherShip*>::iterator it = otherShips.begin(); it != otherShips.end(); ++it) {
        (*it)->moveNode(deltaX,deltaY,deltaZ);
    }
}
