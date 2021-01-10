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

#ifndef __TIDE_HPP_INCLUDED__
#define __TIDE_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>
#include <stdint.h> //for uint64_t

class Tide {

struct tidalHarmonic {
    irr::f32 amplitude; //Metres
    irr::f32 offset; //Offset in degrees (Relative to peak at 0000 on 1 Jan 1970)
    irr::f32 speed; //Degrees per hour

    //Default constructor - initialise to zero
    tidalHarmonic():
        amplitude(0),offset(0),speed(0){}
};

struct tidalDiamond {
    irr::f32 longitude;
    irr::f32 latitude;
    irr::f32 speedXNeaps[13]; // m/s speed for each hour from 6 hours before to 6 hours after high tide, at springs
    irr::f32 speedZNeaps[13];
    irr::f32 speedXSprings[13];
    irr::f32 speedZSprings[13];

    //Default constructor - initialise to zero
    tidalDiamond():
        longitude(0),latitude(0){
            for (int i = 0; i<13; i++) {
                speedXNeaps[i] = 0;speedZNeaps[i] = 0;speedXSprings[i] = 0;speedZSprings[i] = 0;
            }
        }

};

public:
    Tide();
    virtual ~Tide();
    void load(const std::string& worldName);
    void update(uint64_t absoluteTime);
    irr::f32 getTideHeight() const; //To be called after update(time)
    irr::core::vector2df getTidalStream(irr::f32 longitude, irr::f32 latitude, uint64_t absoluteTime) const; //Does not need update() to be called before this

private:
    uint64_t highTideTime(uint64_t startSearchTime, int searchDirection=0) const; //Find previous or next high tide time. Search direction of 0 gives the nearest one (by gradient climb), positive gives next, and negative gives previous
    uint64_t lowTideTime(uint64_t startSearchTime, int searchDirection=0) const; //Find previous or next low tide time.  Search direction of 0 gives the nearest one (by gradient descent), positive gives next, and negative gives previous
    irr::f32 calcTideHeight(uint64_t absoluteTime) const;

    irr::f32 tideHeight;
    //irr::core::vector2df tidalStream; //Speed in m/s
    std::vector<tidalHarmonic> tidalHarmonics;
    std::vector<tidalDiamond> tidalDiamonds;
    irr::f32 meanRangeSprings; //For tidal stream
    irr::f32 meanRangeNeaps;  //For tidal stream
    irr::f32 getTideGradient(uint64_t absoluteTime) const; //return der(TideHeight) (in ?? units)


};

#endif // __TIDE_HPP_INCLUDED__

