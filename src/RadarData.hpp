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

#include <string>

struct RadarData {

    float height;
    float angle;
    float range;
    float relX;
    float relZ;
    float heading;
    float length;
    float width;
    float minRange;
    float maxRange;
    float minAngle;
    float maxAngle;
    float rcs;
    float solidHeight;
    void* contact;

    bool hidden;
    std::string racon; //Racon code if set
    float raconOffsetTime;
    bool SART; //SART enabled?
    //float radarHorizon; //Only used for tracking contacts outside current radar visibility range

    //Default constructor - initialise to zero
    RadarData():
        height(0),
        angle(0),
        range(0),
        relX(0),
        relZ(0),
        heading(0),
        length(0),
        width(0),
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

