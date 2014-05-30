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

    irr::f32 radarHorizon; //Only used for tracking contacts outside current radar visibility range

    bool hidden;
    std::string racon; //Racon code if set
    irr::f32 raconOffsetTime;
    bool SART; //SART enabled?

};

#endif // __RADARDATA_HPP_INCLUDED__

