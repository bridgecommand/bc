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

bool Angles::isAngleBetween(irr::f32 angle, irr::f32 startAng, irr::f32 endAng) {

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

    //std::cout << angle << " " << startAng << " " << endAng << std::endl;

    if(endAng <= 360) { //Simple case
        return (angle >= startAng && angle <=endAng);
    } else { //End angle > 360
        return (angle >= startAng || angle <= normaliseAngle(endAng));
    }
}

irr::f32 Angles::normaliseAngle(irr::f32 angle) { //ensure angle is in range 0-360
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
