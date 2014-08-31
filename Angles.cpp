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
