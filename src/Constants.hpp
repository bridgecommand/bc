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

#ifndef __CONSTANTS_HPP_INCLUDED__
#define __CONSTANTS_HPP_INCLUDED__

#include "irrlicht.h"
#include <string>

//global constants
const int IDFlag_IsPickable=1;
const int IDFlag_IsPickable_2d=2;
const irr::f32 RHO_SW = 1024; // density of seawater kg / m^3
const irr::f32 RHO_FW = 1000; // density of freshwater kg / m^3
const irr::f32 RHO_AIR = 1.225; // density of air at sea level kg / m^3 approximately
				// treated as a non compressible fluid, which it isnt but
				// good enough to use for simulation purposes
const irr::f32 VIEW_PROPORTION_3D = 0.6;

//units conversions
const irr::f32 SECONDS_IN_HOUR = 3600.0;
const irr::f32 SECONDS_IN_DAY = SECONDS_IN_HOUR * 24;
const irr::f32 M_IN_NM = 1852.0;
const irr::f32 KTS_TO_MPS = M_IN_NM/SECONDS_IN_HOUR;
const irr::f32 MPS_TO_KTS = SECONDS_IN_HOUR/M_IN_NM;
const irr::f32 EARTH_RAD_M = 6.371e6;
const irr::f32 EARTH_RAD_CORRECTION = 1.333; //Effective earth's radius for radar calculations
const irr::f32 PI = 3.1415926535897932384626433832795;
const irr::f32 RAD_IN_DEG = PI/180.0;
const irr::f32 DEG_IN_RAD = 180.0 / PI;
const irr::f32 RAD_PER_S_IN_DEG_PER_MINUTE = 180.0/PI * 60 ;

//general definitions
const std::string LONGNAME = "Bridge Command 5.10.3-alpha.3";
const std::string VERSION = "5.10";
const std::string LONGVERSION = "5.10.3-alpha.3";
#endif