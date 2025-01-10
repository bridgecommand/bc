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

#ifndef __RAIN_HPP_INCLUDED__
#define __RAIN_HPP_INCLUDED__

#include "irrlicht.h"
#include <vector>

class Rain
{

public:

    Rain();
    ~Rain();
    void load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, irr::IrrlichtDevice* dev, irr::f32 ShipPosX, irr::f32 ShipPosY, irr::f32 ShipPosZ, irr::f32 ShipLength, irr::f32 ShipBreadth);
    void update(irr::f32 ShipPosX, irr::f32 ShipPosY, irr::f32 ShipPosZ, irr::f32 RainLevel);

    
private:

    irr::scene::ISceneNode* parent;
    irr::scene::IParticleSystemSceneNode* ps[4];
 
};

#endif
