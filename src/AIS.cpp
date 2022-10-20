/*   BridgeCommand 5.7 Copyright (C) James Packer
     This file is Copyright (C) 2022 Fraunhofer FKIE

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

#include "AIS.hpp"
#include "Constants.hpp"
#include "SimulationModel.hpp"
#include "libs/Irrlicht/irrlicht-svn/include/irrTypes.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <tuple>
#include <vector>
#include <cstdlib>

int AIS::currentShip = 0;
bool AIS::initialized = false;
std::vector<irr::u32> AIS::lastUpdates;
std::vector<bool> AIS::classAReport(168, 0);
// arbitrary MMSIs from European countries to assign to otherShips
constexpr const int AIS::mmsis[] = {211032189, 226155323, 232984311, 224513921, 245193002, 247829914};

std::vector<irr::u32> AIS::getReadyShips(SimulationModel* model, irr::u32 now) {
    if (!initialized) {
        for (irr::u32 i=0; i < model->getNumberOfOtherShips(); i++) {
            lastUpdates.push_back(i * 600); // offset ship reports in 600 ms increments
        }
        initialized = true;
    }

    std::vector<irr::u32> readyShips;
    readyShips.clear();

    for (irr::u32 ship=0; ship < lastUpdates.size(); ship++) {
        irr::u32 elapsed_time;
        if (now > lastUpdates[ship]) {
            elapsed_time = now - lastUpdates[ship];
        } else {
            elapsed_time = 0;
        }
        irr::u32 reportingInterval;
        irr::f32 shipSpeed = model->getOtherShipSpeed(ship);

        // TODO: take into account course changes
        // TODO: take into account transmission range in the case of huge maps
        if (shipSpeed <= 0) {
            reportingInterval = 180000; // 3 mins when moored
        } else if (shipSpeed <= 14 * KTS_TO_MPS) {
            reportingInterval = 10000; // 10 seconds under 14 knots
        } else if (shipSpeed <= 23 * KTS_TO_MPS) {
            reportingInterval = 6000; // 6 seconds under 23 knots
        } else {
            reportingInterval = 2000; // 2 seconds over 23 knots
        }

        // random delay to reporting to avoid coalescence of reports after a while
        if (elapsed_time >= reportingInterval + (rand() % 500)) {
            lastUpdates[ship] = now;
            readyShips.push_back(ship);
        }
    }
    return readyShips;
}

std::tuple<std::string, int> AIS::generateClassAReport(SimulationModel* model, irr::u32 ship) {

    bool done = false;

    irr::u32 heading = (irr::u32) model->getOtherShipHeading(ship);
    irr::u32 mmsi = model->getOtherShipMMSI(ship);

    if (mmsi == 0) {
        // mmsi is not set, give the ship a vanity mmsi
        mmsi = mmsis[ship % (sizeof(mmsis) / sizeof(mmsis[0]))] + (ship % 10000);
        // keep incrementing mmsi until it reaches a value that has not been allocated
        // e.g. via scenario config
        bool collision = true;
        irr::u32 ships = model->getNumberOfOtherShips();
        while (collision) {
            collision = false;
            for (int i=0; i < ships; i++) {
                if (model->getOtherShipMMSI(i) == mmsi) {
                    collision = true;
                    break;
                }
            }
            if (collision) mmsi++;
        }
        model->setOtherShipMMSI(ship, mmsi);
    }

    // AIS speed over ground is in 0.1-knot increments, capped to 102.2 knots
    // getOtherShipSpeed returns speed in m/s, multiply by 1.9438445 to get knots
    irr::u32 speed = std::min<int>((int) 10.0f * MPS_TO_KTS * model->getOtherShipSpeed(ship), 1022);

    // BC internal coordinate system
    irr::f32 shipLong = model->getOtherShipLong(ship);
    irr::f32 shipLat  = model->getOtherShipLat(ship);

    std::uint32_t timestamp = model->getTimestamp() % 60;


    // fill class A report fields
    
    // 0-5: message type, set to 0b000001 for normal class A position report
    classAReport[5] = 1;
    
    // 6-7 repeat indicator, set to 0b11 to signify do not repeat
    classAReport[6] = 1;
    classAReport[7] = 1;

    // 8-37 MMSI, 9-decimal digit in 30 bit field
    for (int i=0; i < 30; i++) {
        classAReport[8 + 29 - i] = mmsi % 2;
        mmsi >>= 1;
    }

    // 38-41 navigation status
    // set to 0b0000 for underway using engine
    classAReport[38] = 0;
    classAReport[39] = 0;
    classAReport[40] = 0;
    classAReport[41] = 0;
    if (speed == 0) {
        // if not moving, set to 0b0001 for anchored
        classAReport[41] = 1;
    }

    // 42-49 rate of turn, set to 0x80 for no turn information available
    // TODO: add rate of turn of other ships 
    classAReport[42] = 1;
    for (int i=43; i <= 49; i++) {
        classAReport[i] = 0;
    }

    // 50-59 speed over ground, 10 bit field
    for (int i=0; i < 10; i++) {
        classAReport[50 + 9 - i] = speed % 2;
        speed >>= 1;
    }

    // 60 position accuracy, set to 0b1 to indicate DGPS-quality fix, since
    // shipLong and shipLat have 5 decimals giving a 1m resolution.
    classAReport[60] = 1;

    // 61-88 longitude in a 28-bit field encoding a signed integer representing a float with a
    // resolution of 0.0001 corresponding to the longitude in minutes
    std::int32_t longitude = (int) 600000.0f * shipLong;
    bool longIsNeg = longitude < 0;
    for (int i=0; i < 28; i++) {
        classAReport[61 + 27 - i] = longitude % 2;
        longitude >>= 1;
    }
    if (longIsNeg) classAReport[61] = 1; // set the sign bit
    
    // 89-115 latitude in a 27-bit field encoding a signed integer representing a float with a
    // resolution of 0.0001 corresponding to the latitude in minutes
    std::int32_t latitude = (int) 600000.0f * shipLat;
    bool latIsNeg = latitude < 0;
    for (int i=0; i < 27; i++) {
        classAReport[89 + 26 - i] = latitude % 2;
        latitude >>= 1;
    }
    if (latIsNeg) classAReport[89] = 1; // set the sign bit

    // 116-127 course over ground, 12 bit field, unsigned int representing a float with
    // a resolution of 0.1 corresponding to the course over ground in degrees relative to true north
    irr::u32 cog = 10 * heading;
    for (int i=0; i < 12; i++) {
        classAReport[116 + 11 - i] = cog % 2;
        cog >>= 1;
    }

    // 128-136 true heading, 9 bit field, unsigned int
    for (int i=0; i < 9; i++) {
        classAReport[128 + 8 - i] = heading % 2;
        heading >>= 1;
    }

    // 137-142 timestamp, 6 bit field, unsigned int corresponding to the seconds of current UTC time
    for (int i=0; i < 6; i++) {
        classAReport[137 + 5 - i] = timestamp % 2;
        timestamp >>= 1;
    }

    // 143-144 maneuver indicator, set to 0b00 for no special maneuver
    classAReport[143] = 0;
    classAReport[144] = 1;

    // 145-147 not used
    
    // 148 RAIM flag, set to 0b0 for unset
    classAReport[148] = 0;

    // 149-167 radio status, 19 bit field, unsigned integer for radio diagnostic, leave as 0 for now
    
    // convert bit sequence to armored ASCII
    std::string payload = bitsToArmoredASCII(classAReport);

    // number of bits we need to append to get the payload length to a multiple of 6
    // always 0 since we always generate a class A Report of length 168
    // int fillBits = (6 - (168 % 6)) % 6;

    return std::make_tuple(payload, 0);
}

std::string AIS::bitsToArmoredASCII(std::vector<bool> bits) {
    // must be called with padded payload!
    assert(bits.size() % 6 == 0);

    int counter = 0;

    std::string payload(bits.size() / 6, 0);
    int index = 0;

    for (int i=0; i < bits.size(); i++) {
        payload[index] <<= 1;
        payload[index] |= bits[i];
        counter += 1;
        
        if (counter % 6 == 0) {
            counter = 0;
            payload[index] += 48;

            if (payload[index] >= 88) {
                payload[index] += 8;
            }
            index += 1;
        }
    }
    return payload;
}
