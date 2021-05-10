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

#ifndef __GENERALDATASTRUCT_HPP_INCLUDED__
#define __GENERALDATASTRUCT_HPP_INCLUDED__

#include <string>

#include "irrlicht.h"
#include "../Constants.hpp"

struct GeneralData //To hold information about a position, including defaults for new scenarios
{
    irr::f32 startTime, sunRiseTime, sunSetTime, weather, rain, visibility;
    irr::u32 startDay, startMonth, startYear;
    std::string scenarioName;
    std::string description;
    bool willOverwrite, multiplayerName;
    GeneralData():startTime(10*SECONDS_IN_HOUR), sunRiseTime(6), sunSetTime(18), weather(1.0), visibility(8.0), rain(0), startDay(1), startMonth(1), startYear(2015), scenarioName("New Scenario"), willOverwrite(false), multiplayerName(false), description("Scenario description") {}
};

#endif // __POSITIONDATASTRUCT_HPP_INCLUDED__

