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

#ifndef __LEG_HPP_INCLUDED__
#define __LEG_HPP_INCLUDED__

//Note that legs are changed by the controller, and there is no guarantee that legs prior to the current leg will be consistent with the route from the start position to the current position
//Legs should only be used to calculate current and future movement.

struct Leg //To hold information about each leg of an othership programmed route. Distance is implicit, set by startTime of next leg, but is recorded so we can recreate time information when we change legs
{
    irr::f32 bearing, speed, startTime, distance;
    Leg():bearing(0),speed(0),startTime(0), distance(0){}
};

#endif
