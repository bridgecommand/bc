/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2015 James Packer

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

#include "ScenarioDataStructure.hpp"
#include "Utilities.hpp"

#include <iostream> //Debuggung

//Serialisers:
//Separators (largest first: # , | / ?

std::string LegData::serialise(bool withSpaces)
{
    std::string serialised;
    std::string separator = "?";
    if (withSpaces) {
        separator = "? ";
    }
    serialised.append(Utilities::lexical_cast<std::string>(bearing));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(speed));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(distance));
    return serialised;
}

void LegData::deserialise(std::string data)
{
    std::vector<std::string> splitData = Utilities::split(data,'?');
    if (splitData.size() == 3) {
        bearing = Utilities::lexical_cast<irr::f32>(splitData.at(0));
        speed = Utilities::lexical_cast<irr::f32>(splitData.at(1));
        distance = Utilities::lexical_cast<irr::f32>(splitData.at(2));
    }
}

std::string OtherShipData::serialise(bool withSpaces)
{
    std::string serialised;
    std::string separator = "|";
    if (withSpaces) {
        separator = "| ";
    }
    serialised.append(shipName);
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(mmsi));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(initialLong));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(initialLat));
    serialised.append(separator);
    for(unsigned int i=0;i<legs.size();i++) {
        serialised.append(legs.at(i).serialise(withSpaces));
        if (i+1<legs.size()) { //Add terminating mark if not the last
            if (withSpaces) {
                serialised.append("/ ");
            } else {
                serialised.append("/");
            }
        }
    }
    return serialised;
}

void OtherShipData::deserialise(std::string data)
{
    std::vector<std::string> splitData = Utilities::split(data,'|');
    if (splitData.size() == 5) {
        shipName = splitData.at(0);
        mmsi = Utilities::lexical_cast<irr::f32>(splitData.at(1));
        initialLong = Utilities::lexical_cast<irr::f32>(splitData.at(2));
        initialLat = Utilities::lexical_cast<irr::f32>(splitData.at(3));
        //clear any existing legs data
        legs.clear();
        std::vector<std::string> legsVector = Utilities::split(splitData.at(4),'/');
        for(unsigned int i=0; i<legsVector.size(); i++) {
            LegData tempLeg;
            tempLeg.deserialise(legsVector.at(i));
            legs.push_back(tempLeg);
        }
    }
}

std::string OwnShipData::serialise(bool withSpaces)
{
    std::string serialised;
    std::string separator = ",";
    if (withSpaces) {
        separator = ", ";
    }
    serialised.append(ownShipName);
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(initialSpeed));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(initialLong));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(initialLat));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(initialBearing));
    return serialised;
}

void OwnShipData::deserialise(std::string data)
{
    std::vector<std::string> splitData = Utilities::split(data,',');
    if (splitData.size() == 5) {
        ownShipName = splitData.at(0);
        initialSpeed = Utilities::lexical_cast<irr::f32>(splitData.at(1));
        initialLong = Utilities::lexical_cast<irr::f32>(splitData.at(2));
        initialLat = Utilities::lexical_cast<irr::f32>(splitData.at(3));
        initialBearing = Utilities::lexical_cast<irr::f32>(splitData.at(4));
    }
}

std::string ScenarioData::serialise(bool withSpaces)
{
    std::string separator = "#";
    if (withSpaces) {
        separator = "# ";
    }
    // SCN2 is the same as SCN1, but allows whitespace around the delimiters
    std::string serialised = "SCN2"; //Scenario data, serialised format 2
    serialised.append(separator);
    serialised.append(scenarioName);
    serialised.append(separator);
    serialised.append(worldName);
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(startTime));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(startDay));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(startMonth));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(startYear));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(sunRise));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(sunSet));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(weather));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(rainIntensity));
    serialised.append(separator);
    serialised.append(Utilities::lexical_cast<std::string>(visibilityRange));
    serialised.append(separator);
    serialised.append(ownShipData.serialise(withSpaces));
    serialised.append(separator);
    for(unsigned int i=0;i<otherShipsData.size();i++) {
        serialised.append(otherShipsData.at(i).serialise(withSpaces));
        if (i+1<otherShipsData.size()) { //Add terminating mark if not the last
            if (withSpaces) {
                serialised.append(", ");
            } else {
                serialised.append(",");
            }
        }
    }
    return serialised;
}

void ScenarioData::deserialise(std::string data)
{

    std::vector<std::string> splitData = Utilities::split(data,'#');
    if (splitData.size() == 14) {
        //note that splitData.at(0) is the version of the serialised data format
        // SCN2 is the same as SCN1, but allows whitespace around the delimiters
        if ((splitData.at(0) == "SCN1") || (splitData.at(0) == "SCN2")) {
            scenarioName = splitData.at(1);
            worldName = splitData.at(2);
            startTime = Utilities::lexical_cast<irr::f32>(splitData.at(3));
            startDay = Utilities::lexical_cast<irr::u32>(splitData.at(4));
            startMonth = Utilities::lexical_cast<irr::u32>(splitData.at(5));
            startYear = Utilities::lexical_cast<irr::u32>(splitData.at(6));
            sunRise = Utilities::lexical_cast<irr::f32>(splitData.at(7));
            sunSet = Utilities::lexical_cast<irr::f32>(splitData.at(8));
            weather = Utilities::lexical_cast<irr::f32>(splitData.at(9));
            rainIntensity = Utilities::lexical_cast<irr::f32>(splitData.at(10));
            visibilityRange = Utilities::lexical_cast<irr::f32>(splitData.at(11));
            ownShipData.deserialise(splitData.at(12));
            //clear any existing legs data
            otherShipsData.clear();
            std::vector<std::string> otherShipsVector = Utilities::split(splitData.at(13),',');
            for(unsigned int i=0; i<otherShipsVector.size(); i++) {
                OtherShipData tempOther;
                tempOther.deserialise(otherShipsVector.at(i));
                otherShipsData.push_back(tempOther);
            }
            dataPopulated = true; // Currently only used in scenario editor
        }
    }

}
