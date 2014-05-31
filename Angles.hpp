#ifndef __ANGLES_HPP_INCLUDED__
#define __ANGLES_HPP_INCLUDED__

//Utility functions for angles

#include "irrlicht.h"

namespace Angles
{
    bool isAngleBetween(irr::f32 angle, irr::f32 startAng, irr::f32 endAng);
    irr::f32 normaliseAngle(irr::f32 angle);
}

#endif // __ANGLES_HPP_INCLUDED__
