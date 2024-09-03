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

#ifndef __SIMULATIONDATASTRUCTURE_HPP_INCLUDED__
#define __SIMULATIONDATASTRUCTURE_HPP_INCLUDED__

#include <string>
#include <vector>
#include "irrlicht.h"

//These classes are used as structures to hold a scenario definition, and therefore have all members as public.
//Methods for serialisation and deserialisation are included for utility

class OwnShipData {
    public:
    std::string ownShipName;    
    irr::f32 initialSpeed, initialLong, initialLat, initialBearing;
    irr::f32 initialX, initialZ; // initialX and initialZ only used for scenario editor

    OwnShipData():initialSpeed(0),initialLong(0),initialLat(0), initialBearing(0), initialX(0), initialZ(0){}
    std::string serialise(bool withSpaces);
    void deserialise(std::string data);
};

class LegData {
    public:
    irr::f32 bearing, speed, distance;
    irr::f32 startTime; //startTime only used in scenario editor

    LegData():bearing(0),speed(0),distance(0),startTime(0){}

    std::string serialise(bool withSpaces);
    void deserialise(std::string data);
};

class OtherShipData {
    public:
    std::string shipName;
    irr::u32 mmsi;
    irr::f32 initialLong, initialLat;
    irr::f32 initialX, initialZ; // initialX and initialZ only used for scenario editor
    std::vector<LegData> legs;

    OtherShipData():initialLong(0), initialLat(0), initialX(0), initialZ(0){}

    std::string serialise(bool withSpaces);
    void deserialise(std::string data);
};

class ScenarioData {
    public:
    std::string scenarioName, worldName;
    irr::f32 startTime, sunRise, sunSet, weather, rainIntensity, visibilityRange;
    irr::u32 startDay, startMonth, startYear;
    OwnShipData ownShipData;
    std::vector<OtherShipData> otherShipsData;
    std::string description; // description only used in scenario editor
    bool multiplayerName, willOverwrite; // multiplayerName only used in scenario editor
    bool dataPopulated; // dataPopulated only used in scenario editor

    ScenarioData():startTime(0),sunRise(0),sunSet(0),weather(0),rainIntensity(0),visibilityRange(0),startDay(0),startMonth(0),startYear(0),description(""),multiplayerName(false),willOverwrite(false),dataPopulated(false){}

    std::string serialise(bool withSpaces);
    void deserialise(std::string data);
};

#endif
