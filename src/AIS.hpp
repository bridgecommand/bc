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

#ifndef __AIS_HPP_INCLUDED__
#define __AIS_HPP_INCLUDED__

#include "SimulationModel.hpp"
#include <tuple>
#include <string>
#include <vector>

class AIS {
    public:
        static std::tuple<std::string, int> generateClassAReport(SimulationModel*, uint32_t);
        static std::tuple<std::string, int> generateMessage5(SimulationModel*, uint32_t);
        static std::tuple<std::string, int> generateMessage21(SimulationModel*, uint32_t buoyNumber);
        static std::vector<uint32_t> getReadyShips(SimulationModel*, uint32_t);
        static bool isMessage5Due(SimulationModel*, uint32_t now);
        static bool isMessage21Due(uint32_t now);

    private:
        static const int mmsis[];
        static std::vector<uint32_t> lastUpdates;
        static int currentShip;
        static bool initialized;
        static std::vector<bool> classAReport;
        static std::vector<bool> message5Report;
        static std::vector<bool> message21Report;
        static uint32_t lastMessage5Time;
        static uint32_t lastMessage21Time;
        static uint32_t message5ShipIndex;
        static std::string bitsToArmoredASCII(std::vector<bool>);
        static void encodeUnsigned(std::vector<bool>& bits, int startBit, int numBits, uint32_t value);
        static void encodeSigned(std::vector<bool>& bits, int startBit, int numBits, int32_t value);
        static void encodeText(std::vector<bool>& bits, int startBit, int maxChars, const std::string& text);
};

#endif

