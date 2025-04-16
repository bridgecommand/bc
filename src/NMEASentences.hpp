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

#ifndef __NMEASENTENCES_HPP_INCLUDED__
#define __NMEASENTENCES_HPP_INCLUDED__

#include "irrTypes.h"
#include <stdexcept>
#include <string>
#include <sys/types.h>

const irr::f32 INVALID_LAT = 9999.99;
const irr::f32 INVALID_LONG = 9999.99;


inline irr::f32 parseNmeaLat(std::string latitude, char direction)
{
    // hhmm.mm [N/S] to [+/-]x.xxxxx
    if (latitude.length() != 7 || (direction != 'N' && direction != 'S')) return INVALID_LAT;

    irr::s32 mod = 1;
    if (direction == 'S') mod = -1;

    try 
    {
        irr::f32 hours = std::stof(latitude.substr(0, 2));
        irr::f32 minutes = std::stof(latitude.substr(2, 5));
        return mod * (hours + (minutes / 60.0));
    } catch (const std::invalid_argument& e) 
    {
        return INVALID_LAT;
    }

}

inline irr::f32 parseNmeaLong(std::string longitude, char direction)
{
    // hhhmm.mm [N/S] to [+/-]x.xxxxx
    if (longitude.length() != 8 || (direction != 'W' && direction != 'E')) return INVALID_LONG;

    irr::s32 mod = 1;
    if (direction == 'W') mod = -1;

    try 
    {
        irr::f32 hours = std::stof(longitude.substr(0, 3));
        irr::f32 minutes = std::stof(longitude.substr(3, 5));
        return mod * (hours + (minutes / 60.0));
    } catch (const std::invalid_argument& e)
    {
        return INVALID_LAT;
    }
}

struct APB
{
    char status;
    char warning;
    irr::f32 cross_track_error; // how far off track are we (perpendicular distance to track)
    char direction; // direction to steer, OpenCPN sets this to direction to track, not to waypoint 
    char cross_track_units; // N for Nautical miles
    char arrival_circle_entered; // A -> true, V -> false
    char perpendicular_passed; // perpendicular passed at waypoint, A-> true, V -> false
    irr::f32 bearing_orig_to_dest;
    char bearing_orig_to_dest_type; // M -> magnetic, T -> true
    std::string dest_waypoint_id;
    irr::f32 bearing_to_dest; // bearing of current pos to dest
    char bearing_to_dest_type;
    irr::f32 heading_to_dest; // heading to steer to 
    char heading_to_dest_type;
};

struct RMB
{
    char status;
    irr::f32 cross_track_error;
    char direction;
    std::string dest_waypoint_id;
    std::string orig_waypoint_id;
    std::string dest_waypoint_latitude; // original hhmm.mm format
    char dest_waypoint_latitude_dir; // N or S
    std::string dest_waypoint_longitude; // original hhhmm.mm format
    char dest_waypoint_longitude_dir; // W or E
    irr::f32 range_to_dest; // in NM
    irr::f32 bearing_to_dest; // degrees true
    irr::f32 dest_closing_velocity; // knots
    char arrival_status; // A -> circle entered, V -> not entered / left
    char faa_mode; // A -> Autonomous, D -> Differential, E -> Estimated
                   // M -> Manual, S -> Simulator, N -> Not Valid
};

#endif
