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

#include "Angles.hpp"
#include <algorithm>
#include <iostream>

//From OpenCV via http://stackoverflow.com/a/20723890
int Angles::localisinf(double x)
{
    union { uint64_t u; double f; } ieee754;
    ieee754.f = x;
    return ( (unsigned)(ieee754.u >> 32) & 0x7fffffff ) == 0x7ff00000 &&
           ( (unsigned)ieee754.u == 0 );
}

int Angles::localisnan(double x)
{
    union { uint64_t u; double f; } ieee754;
    ieee754.f = x;
    return ( (unsigned)(ieee754.u >> 32) & 0x7fffffff ) +
           ( (unsigned)ieee754.u != 0 ) > 0x7ff00000;
}
//End From OpenCV via http://stackoverflow.com/a/20723890

bool Angles::isAngleBetween(irr::core::vector2df angle, irr::core::vector2df startAng, irr::core::vector2df endAng)
{
    //Aim is to be a faster way of calculating if an angle is between two others, not using atan2
    //Ref: http://stackoverflow.com/a/13641047, inverted for different hand of rotation, and follow up answers

    return ((startAng.Y*angle.X-startAng.X*angle.Y) * (startAng.Y*endAng.X-startAng.X*endAng.Y) >= 0) && //Is direction from start to checked, and start to end the same
           ((angle.Y * endAng.X-angle.X * endAng.Y) * (startAng.Y*endAng.X-startAng.X*endAng.Y) >= 0); //Is direction from checked to end and start to end the same?
}

bool Angles::isAngleBetween(irr::f32 angle, irr::f32 startAng, irr::f32 endAng) {

    //Return false if any is not a normal number (e.g. NaN)
    if (localisinf(angle) || localisnan(angle) || localisinf(startAng) || localisnan(startAng) || localisinf(endAng) || localisnan(endAng)) {
        return false;
    }

    //adjust angles to make start angle in range 0-360. Change both angles together, so their difference is maintained
    while (startAng < 0) {
        startAng+=360;
        endAng+=360;
    }
    while (startAng >= 360) {
        startAng-=360;
        endAng-=360;
    }

    //normalise angle
    angle = normaliseAngle(angle);

    if(endAng <= 360) { //Simple case
        return (angle >= startAng && angle <=endAng);
    } else { //End angle > 360
        return (angle >= startAng || angle <= normaliseAngle(endAng));
    }
}

irr::f32 Angles::normaliseAngle(irr::f32 angle) { //ensure angle is in range 0-360

    //Return unchanged if NaN etc.
    if (localisinf(angle) || localisnan(angle)) {
        return angle;
    }

    while (angle < 0) {
        angle+=360;
    }

    while (angle >= 360) {
        angle-=360;
    }

    return angle;
}

irr::core::vector3df Angles::irrAnglesFromYawPitchRoll(irr::f32 yaw, irr::f32 pitch, irr::f32 roll)
//Convert yaw,pitch,roll (in degrees) into irrlicht 'euler angles' in degrees, as used by setRotation,
//essentially changing the order the transformations are applied in.
{
    //Create rotation matrices (default to identity in construction)
    irr::core::matrix4 total;
    irr::core::matrix4 p;
    irr::core::matrix4 r;
    irr::core::matrix4 y;
    //Create the individual components
    r.setRotationDegrees(irr::core::vector3df(0,0,roll));
    p.setRotationDegrees(irr::core::vector3df(pitch,0,0));
    y.setRotationDegrees(irr::core::vector3df(0,yaw,0));
    //apply rotations, in order of yaw, pitch, roll
    total*=y;
    total*=p;
    total*=r;

    return total.getRotationDegrees();
}

int Angles::sign(float in)
{
    if( in > 0 ) {return 1;}
    if( in < 0 ) {return -1;}
    return 0;
}
