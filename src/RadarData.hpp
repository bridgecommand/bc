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

#ifndef __RADARDATA_HPP_INCLUDED__
#define __RADARDATA_HPP_INCLUDED__

#include "irrlicht.h"

#include <string>

struct RadarData {

    irr::f32 height;
    irr::f32 angle;
    irr::f32 range;
    irr::f32 relX;
    irr::f32 relZ;
    irr::f32 heading;
    irr::f32 length;
    irr::f32 minRange;
    irr::f32 maxRange;
    irr::f32 minAngle;
    irr::f32 maxAngle;
    irr::f32 rcs;
    irr::f32 solidHeight;
    void* contact;

    bool hidden;
    std::string racon; //Racon code if set
    irr::f32 raconOffsetTime;
    bool SART; //SART enabled?
    //irr::f32 radarHorizon; //Only used for tracking contacts outside current radar visibility range

    //Default constructor - initialise to zero
    RadarData():
        height(0),
        angle(0),
        range(0),
        relX(0),
        relZ(0),
        heading(0),
        length(0),
        minRange(0),
        maxRange(0),
        minAngle(0),
        maxAngle(0),
        rcs(0),
        solidHeight(0),
        contact(0),
        hidden(false),
        racon(""),
        raconOffsetTime(0),
        SART(false)
        {}

};

#endif // __RADARDATA_HPP_INCLUDED__

