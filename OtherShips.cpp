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

#include <iostream> //debugging

using namespace irr;

OtherShips::OtherShips()
{

}

OtherShips::~OtherShips()
{
    //dtor
}

void OtherShips::load(const std::string& scenarioName, irr::f32 scenarioStartTime, bool secondary, irr::scene::ISceneManager* smgr, SimulationModel* model)
{

    //construct path
    std::string scenarioOtherShipsFilename = scenarioName;
    scenarioOtherShipsFilename.append("/othership.ini");

    //Find number of other ships
    u32 numberOfOtherShips;
    numberOfOtherShips = IniFile::iniFileTou32(scenarioOtherShipsFilename,"Number");
    if (numberOfOtherShips > 0)
    {
        for(u32 i=1;i<=numberOfOtherShips;i++)
        {
            //Get ship type and construct filename
            std::string otherShipName = IniFile::iniFileToString(scenarioOtherShipsFilename,IniFile::enumerate1("Type",i));
            //Get initial position
            f32 shipX = model->longToX(IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate1("InitLong",i)));
            f32 shipZ = model->latToZ(IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate1("InitLat",i)));

            //Load leg information
            std::vector<Leg> legs;
            irr::u32 numberOfLegs = IniFile::iniFileTou32(scenarioOtherShipsFilename,IniFile::enumerate1("Legs",i));
            irr::f32 legStartTime = scenarioStartTime;
            if (!secondary) { //Don't load leg information in secondary mode
                for(irr::u32 currentLegNo=1; currentLegNo<=numberOfLegs; currentLegNo++){
                    //go through each leg (if any), and load
                    Leg currentLeg;
                    currentLeg.bearing = IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate2("Bearing",i,currentLegNo));
                    currentLeg.speed = IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate2("Speed",i,currentLegNo));
                    currentLeg.startTime = legStartTime;

                    //Use distance to calculate startTime of next leg, and stored for later reference.
                    irr::f32 distance = IniFile::iniFileTof32(scenarioOtherShipsFilename,IniFile::enumerate2("Distance",i,currentLegNo));
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
            otherShips.push_back(OtherShip (otherShipName,core::vector3df(shipX,0.0f,shipZ),legs,smgr));
        }
    }
}

void OtherShips::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::f32 tideHeight, irr::u32 lightLevel)
{
    for(std::vector<OtherShip>::iterator it = otherShips.begin(); it != otherShips.end(); ++it) {
        it->update(deltaTime, scenarioTime, tideHeight, lightLevel);
    }
}

RadarData OtherShips::getRadarData(irr::u32 number, irr::core::vector3df scannerPosition) const
//Get data for OtherShip (number) relative to scannerPosition
{
    RadarData radarData;

    if (number<=otherShips.size()) {
        radarData = otherShips[number-1].getRadarData(scannerPosition);
    }
    return radarData;
}

irr::u32 OtherShips::getNumber() const
{
    return otherShips.size();
}

irr::core::vector3df OtherShips::getPosition(int number) const
{
    if (number < otherShips.size()) {
        return otherShips.at(number).getPosition();
    } else {
        return core::vector3df(0,0,0);
    }
}

irr::f32 OtherShips::getLength(int number) const
{
    if (number < otherShips.size()) {
        return otherShips.at(number).getLength();
    } else {
        return 0.0;
    }
}

irr::f32 OtherShips::getWidth(int number) const
{
    if (number < otherShips.size()) {
        return otherShips.at(number).getWidth();
    } else {
        return 0.0;
    }
}

irr::f32 OtherShips::getHeading(int number) const
{
    if (number < otherShips.size()) {
        return otherShips.at(number).getHeading();
    } else {
        return 0;
    }
}

irr::f32 OtherShips::getSpeed(int number) const
{
    if (number < otherShips.size()) {
        return otherShips.at(number).getSpeed();
    } else {
        return 0;
    }
}

void OtherShips::setSpeed(int number, irr::f32 speed)
{
    if (number < otherShips.size()) {
        otherShips.at(number).setSpeed(speed);
    }
}

void OtherShips::setPos(int number, irr::f32 positionX, irr::f32 positionZ)
{
    if (number < otherShips.size()) {
        otherShips.at(number).setPosition(positionX,positionZ);
    }
}

void OtherShips::setHeading(int number, irr::f32 hdg)
{
    if (number < otherShips.size()) {
        otherShips.at(number).setHeading(hdg);
    }
}

std::vector<Leg> OtherShips::getLegs(int number) const
{
    if (number < otherShips.size()) {
        return otherShips.at(number).getLegs();
    } else {
        //Return an empty vector
        std::vector<Leg> legs;
        return legs;
    }
}

void OtherShips::changeLeg(int shipNumber, int legNumber, irr::f32 bearing, irr::f32 speed, irr::f32 distance, irr::f32 scenarioTime)
{
    //Check if ship exists
    if (shipNumber < otherShips.size()) {
        otherShips.at(shipNumber).changeLeg(legNumber, bearing, speed, distance, scenarioTime);
    }
}

void OtherShips::addLeg(int shipNumber, int afterLegNumber, irr::f32 bearing, irr::f32 speed, irr::f32 distance, irr::f32 scenarioTime)
{
    //Check if ship exists
    if (shipNumber < otherShips.size()) {
        otherShips.at(shipNumber).addLeg(afterLegNumber, bearing, speed, distance, scenarioTime);
    }
}

void OtherShips::deleteLeg(int shipNumber, int legNumber, irr::f32 scenarioTime)
{
    //Check if ship exists
    if (shipNumber < otherShips.size()) {
        otherShips.at(shipNumber).deleteLeg(legNumber, scenarioTime);
    }
}

std::string OtherShips::getName(int number) const
{
    if(number < otherShips.size()) {
        return otherShips.at(number).getName();
    } else {
        return "";
    }
}

void OtherShips::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    for(std::vector<OtherShip>::iterator it = otherShips.begin(); it != otherShips.end(); ++it) {
        it->moveNode(deltaX,deltaY,deltaZ);
    }
}
