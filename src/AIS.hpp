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
        static std::tuple<std::string, int> generateClassAReport(SimulationModel*, irr::u32);
        static std::vector<irr::u32> getReadyShips(SimulationModel*, irr::u32);

    private:
        static const int mmsis[];
        static std::vector<irr::u32> lastUpdates; 
        static int currentShip;
        static bool initialized;
        static std::vector<bool> classAReport;
        static std::string bitsToArmoredASCII(std::vector<bool>);
};

#endif

