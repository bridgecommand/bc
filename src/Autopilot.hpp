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

#ifndef __AUTOPILOT_HPP_INCLUDED__
#define __AUTOPILOT_HPP_INCLUDED__

#include "NMEASentences.hpp"
#include "SimulationModel.hpp"
#include "irrTypes.h"
#include <array>

class Autopilot
{
    public:
        Autopilot(SimulationModel*);
        ~Autopilot();
        bool receiveAPB(APB);
        bool receiveRMB(RMB);
    private:
        bool AUTOPILOT_ENABLED;
        SimulationModel* model;
        std::array<irr::f32, 2> currentWaypointPos;
        std::string currentWaypointId;
        irr::f32 crossTrackError;
        irr::f32 currentLegLen;
        char directionToSteer;
};

#endif 
