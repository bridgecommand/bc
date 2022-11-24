/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2016 James Packer

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

#include "ShipPositions.hpp"
#include "../Constants.hpp"

ShipPositions::ShipPositions(unsigned int numberOfShips)
{
    ShipPosition emptyShipDataEntry;
    for (unsigned int i = 0; i<numberOfShips; i++) {
        shipData.push_back(emptyShipDataEntry);
    }
}

void ShipPositions::setShipPosition(unsigned int shipNumber, irr::f32 scenarioTime, irr::f32 positionX, irr::f32 positionZ, irr::f32 speed, irr::f32 bearing, irr::f32 rateOfTurn)
{
    if (shipNumber < shipData.size()) {
        shipData.at(shipNumber).speed = speed;
        shipData.at(shipNumber).positionX = positionX;
        shipData.at(shipNumber).positionZ = positionZ;
        shipData.at(shipNumber).bearing = bearing;
        shipData.at(shipNumber).rateOfTurn = rateOfTurn;
        shipData.at(shipNumber).timeStored = scenarioTime;
    }
}

void ShipPositions::getShipPosition(const unsigned int& shipNumber, const irr::f32& scenarioTime, irr::f32& positionX, irr::f32& positionZ, irr::f32& speed, irr::f32& bearing, irr::f32& rateOfTurn)
{
    if (shipNumber < shipData.size()) {
        //Extrapolate from last recorded point
        speed = shipData.at(shipNumber).speed; //In m/s

        irr::f32 deltaTime = scenarioTime - shipData.at(shipNumber).timeStored;
        irr::f32 deltaAngle = deltaTime*shipData.at(shipNumber).rateOfTurn;

        rateOfTurn = shipData.at(shipNumber).rateOfTurn;

        bearing = shipData.at(shipNumber).bearing + deltaAngle;
        
        irr::f32 deltaX = deltaTime*speed*sin(RAD_IN_DEG*bearing);
        irr::f32 deltaZ = deltaTime*speed*cos(RAD_IN_DEG*bearing);
        
        positionX = shipData.at(shipNumber).positionX + deltaX;
        positionZ = shipData.at(shipNumber).positionZ + deltaZ;
        
        

        //speed*=MPS_TO_KTS; //Convert for knots

    } else {
        speed = 0;
        positionX = 0;
        positionZ = 0;
        bearing = 0;
    }
}
