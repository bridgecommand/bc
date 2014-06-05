#ifndef __CONSTANTS_HPP_INCLUDED__
#define __CONSTANTS_HPP_INCLUDED__

#include "irrlicht.h"

//global constants
const irr::f32 SECONDS_IN_HOUR = 3600.0;
const irr::f32 M_IN_NM = 1852.0;
const irr::f32 KTS_TO_MPS = M_IN_NM/SECONDS_IN_HOUR;
const irr::f32 MPS_TO_KTS = SECONDS_IN_HOUR/M_IN_NM;
const irr::f32 EARTH_RAD_M = 6.371e6;
const irr::f32 EARTH_RAD_CORRECTION = 1.333; //Effective earth's radius for radar calculations
const irr::f32 PI = 3.1415926535897932384626433832795;


#endif
