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
std::vector<uint32_t> AIS::lastUpdates;
std::vector<bool> AIS::classAReport(168, 0);
std::vector<bool> AIS::message5Report(424, 0);
std::vector<bool> AIS::message21Report(272, 0);
uint32_t AIS::lastMessage5Time = 0;
uint32_t AIS::lastMessage21Time = 0;
uint32_t AIS::message5ShipIndex = 0;
// arbitrary MMSIs from European countries to assign to otherShips
constexpr const int AIS::mmsis[] = {211032189, 226155323, 232984311, 224513921, 245193002, 247829914};

std::vector<uint32_t> AIS::getReadyShips(SimulationModel* model, uint32_t now) {
    if (!initialized) {
        for (uint32_t i=0; i < model->getNumberOfOtherShips(); i++) {
            lastUpdates.push_back(i * 600); // offset ship reports in 600 ms increments
        }
        initialized = true;
    }

    std::vector<uint32_t> readyShips;
    readyShips.clear();

    for (uint32_t ship=0; ship < lastUpdates.size(); ship++) {
        uint32_t elapsed_time;
        if (now > lastUpdates[ship]) {
            elapsed_time = now - lastUpdates[ship];
        } else {
            elapsed_time = 0;
        }
        uint32_t reportingInterval;
        float shipSpeed = model->getOtherShipSpeed(ship);

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

std::tuple<std::string, int> AIS::generateClassAReport(SimulationModel* model, uint32_t ship) {

    bool done = false;

    uint32_t heading = (uint32_t) model->getOtherShipHeading(ship);
    uint32_t mmsi = model->getOtherShipMMSI(ship);

    if (mmsi == 0) {
        // mmsi is not set, give the ship a vanity mmsi
        mmsi = mmsis[ship % (sizeof(mmsis) / sizeof(mmsis[0]))] + (ship % 10000);
        // keep incrementing mmsi until it reaches a value that has not been allocated
        // e.g. via scenario config
        bool collision = true;
        uint32_t ships = model->getNumberOfOtherShips();
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
    uint32_t speed = std::min<int>((int) 10.0f * MPS_TO_KTS * model->getOtherShipSpeed(ship), 1022);

    // BC internal coordinate system
    float shipLong = model->getOtherShipLong(ship);
    float shipLat  = model->getOtherShipLat(ship);

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
    uint32_t cog = 10 * heading;
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

void AIS::encodeUnsigned(std::vector<bool>& bits, int startBit, int numBits, uint32_t value) {
    for (int i = 0; i < numBits; i++) {
        bits[startBit + numBits - 1 - i] = value % 2;
        value >>= 1;
    }
}

void AIS::encodeSigned(std::vector<bool>& bits, int startBit, int numBits, int32_t value) {
    bool isNeg = value < 0;
    for (int i = 0; i < numBits; i++) {
        bits[startBit + numBits - 1 - i] = value % 2;
        value >>= 1;
    }
    if (isNeg) bits[startBit] = 1; // set sign bit
}

void AIS::encodeText(std::vector<bool>& bits, int startBit, int maxChars, const std::string& text) {
    for (int c = 0; c < maxChars; c++) {
        uint8_t sixBit = 0; // '@' (padding)
        if (c < (int)text.size()) {
            char ch = text[c];
            // Convert to uppercase
            if (ch >= 'a' && ch <= 'z') ch -= 32;
            // AIS 6-bit ASCII encoding
            if (ch >= 64 && ch <= 95) {
                sixBit = ch - 64; // @, A-Z, [, \, ], ^, _
            } else if (ch >= 32 && ch <= 63) {
                sixBit = ch; // space, digits, punctuation
            }
            // else: unsupported char → '@' (0)
        }
        encodeUnsigned(bits, startBit + c * 6, 6, sixBit);
    }
}

bool AIS::isMessage5Due(SimulationModel* model, uint32_t now) {
    if (model->getNumberOfOtherShips() == 0) return false;
    uint32_t elapsed = (now > lastMessage5Time) ? (now - lastMessage5Time) : 0;
    if (elapsed >= 360000) { // 6 minutes
        lastMessage5Time = now;
        return true;
    }
    return false;
}

bool AIS::isMessage21Due(uint32_t now) {
    uint32_t elapsed = (now > lastMessage21Time) ? (now - lastMessage21Time) : 0;
    if (elapsed >= 180000) { // 3 minutes
        lastMessage21Time = now;
        return true;
    }
    return false;
}

std::tuple<std::string, int> AIS::generateMessage5(SimulationModel* model, uint32_t ship) {
    // Reset the bit vector
    std::fill(message5Report.begin(), message5Report.end(), 0);

    uint32_t mmsi = model->getOtherShipMMSI(ship);
    // MMSI should already be assigned by generateClassAReport

    std::string shipName = model->getOtherShipName(ship);
    float length = model->getOtherShipLength(ship);
    float breadth = model->getOtherShipBreadth(ship);

    // 0-5: Message Type = 5
    encodeUnsigned(message5Report, 0, 6, 5);

    // 6-7: Repeat Indicator = 0 (default)
    encodeUnsigned(message5Report, 6, 2, 0);

    // 8-37: MMSI
    encodeUnsigned(message5Report, 8, 30, mmsi);

    // 38-39: AIS Version = 0
    encodeUnsigned(message5Report, 38, 2, 0);

    // 40-69: IMO Number = 0 (not available)
    encodeUnsigned(message5Report, 40, 30, 0);

    // 70-111: Call Sign (7 chars) - not available, fill with '@'
    encodeText(message5Report, 70, 7, "");

    // 112-231: Vessel Name (20 chars)
    encodeText(message5Report, 112, 20, shipName);

    // 232-239: Ship Type = 70 (cargo, all types, no additional info)
    encodeUnsigned(message5Report, 232, 8, 70);

    // 240-248: Dimension to Bow (9 bits) - assume AIS antenna at centre
    uint32_t dimBow = std::min<uint32_t>((uint32_t)(length / 2.0f), 511);
    encodeUnsigned(message5Report, 240, 9, dimBow);

    // 249-257: Dimension to Stern (9 bits)
    uint32_t dimStern = std::min<uint32_t>((uint32_t)(length - dimBow), 511);
    encodeUnsigned(message5Report, 249, 9, dimStern);

    // 258-263: Dimension to Port (6 bits)
    uint32_t dimPort = std::min<uint32_t>((uint32_t)(breadth / 2.0f), 63);
    encodeUnsigned(message5Report, 258, 6, dimPort);

    // 264-269: Dimension to Starboard (6 bits)
    uint32_t dimStarboard = std::min<uint32_t>((uint32_t)(breadth - dimPort), 63);
    encodeUnsigned(message5Report, 264, 6, dimStarboard);

    // 270-273: Type of EPFD = 1 (GPS)
    encodeUnsigned(message5Report, 270, 4, 1);

    // 274-277: ETA Month = 0 (N/A)
    encodeUnsigned(message5Report, 274, 4, 0);
    // 278-282: ETA Day = 0 (N/A)
    encodeUnsigned(message5Report, 278, 5, 0);
    // 283-287: ETA Hour = 24 (N/A)
    encodeUnsigned(message5Report, 283, 5, 24);
    // 288-293: ETA Minute = 60 (N/A)
    encodeUnsigned(message5Report, 288, 6, 60);

    // 294-301: Maximum Draught = 0 (not available), in 1/10 m
    encodeUnsigned(message5Report, 294, 8, 0);

    // 302-421: Destination (20 chars) - not available, fill with '@'
    encodeText(message5Report, 302, 20, "");

    // 422: DTE = 0 (data terminal ready)
    message5Report[422] = 0;

    // 423: Spare
    message5Report[423] = 0;

    // Convert to armored ASCII - 424 bits needs padding to multiple of 6
    // 424 / 6 = 70 remainder 4, so we need 2 fill bits
    // Pad to 426 bits (71 6-bit chars)
    std::vector<bool> padded(message5Report);
    padded.push_back(0);
    padded.push_back(0);

    std::string payload = bitsToArmoredASCII(padded);
    return std::make_tuple(payload, 2); // 2 fill bits
}

std::tuple<std::string, int> AIS::generateMessage21(SimulationModel* model, uint32_t buoyNumber) {
    // Reset the bit vector
    std::fill(message21Report.begin(), message21Report.end(), 0);

    float buoyLong = model->getBuoyLong(buoyNumber);
    float buoyLat = model->getBuoyLat(buoyNumber);

    // Generate AtoN MMSI: 99MIDXXXX format
    // Use 993 (generic MID) + buoy number
    uint32_t atonMMSI = 993000000 + (buoyNumber + 1);

    std::uint32_t timestamp = model->getTimestamp() % 60;

    // 0-5: Message Type = 21
    encodeUnsigned(message21Report, 0, 6, 21);

    // 6-7: Repeat Indicator = 0
    encodeUnsigned(message21Report, 6, 2, 0);

    // 8-37: MMSI
    encodeUnsigned(message21Report, 8, 30, atonMMSI);

    // 38-42: Type of AtoN = 1 (default/unspecified)
    encodeUnsigned(message21Report, 38, 5, 1);

    // 43-162: Name of AtoN (20 chars)
    // Use "BUOY" + number as name
    std::string buoyName = "BUOY " + std::to_string(buoyNumber + 1);
    encodeText(message21Report, 43, 20, buoyName);

    // 163: Position Accuracy = 1 (DGPS-quality)
    message21Report[163] = 1;

    // 164-191: Longitude (28-bit signed, 1/10000 minute)
    std::int32_t longitude = (int32_t)(600000.0f * buoyLong);
    encodeSigned(message21Report, 164, 28, longitude);

    // 192-218: Latitude (27-bit signed, 1/10000 minute)
    std::int32_t latitude = (int32_t)(600000.0f * buoyLat);
    encodeSigned(message21Report, 192, 27, latitude);

    // 219-248: Dimension/Reference for position (30 bits)
    // For a buoy, set small dimensions: bow=1, stern=1, port=1, starboard=1
    encodeUnsigned(message21Report, 219, 9, 1); // Dim to bow
    encodeUnsigned(message21Report, 228, 9, 1); // Dim to stern
    encodeUnsigned(message21Report, 237, 6, 1); // Dim to port
    encodeUnsigned(message21Report, 243, 6, 1); // Dim to starboard

    // 249-252: Type of EPFD = 1 (GPS)
    encodeUnsigned(message21Report, 249, 4, 1);

    // 253-258: UTC Second
    encodeUnsigned(message21Report, 253, 6, timestamp);

    // 259: Off-Position Indicator = 0 (on position)
    message21Report[259] = 0;

    // 260-267: AtoN Status = 0 (default)
    encodeUnsigned(message21Report, 260, 8, 0);

    // 268: RAIM = 0
    message21Report[268] = 0;

    // 269: Virtual AtoN = 0 (real)
    message21Report[269] = 0;

    // 270: Assigned Mode = 0 (autonomous)
    message21Report[270] = 0;

    // 271: Spare
    message21Report[271] = 0;

    // 272 bits / 6 = 45 remainder 2, so 4 fill bits needed
    std::vector<bool> padded(message21Report);
    padded.push_back(0);
    padded.push_back(0);
    padded.push_back(0);
    padded.push_back(0);

    std::string payload = bitsToArmoredASCII(padded);
    return std::make_tuple(payload, 4); // 4 fill bits
}
