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

#ifndef __WATER_HPP_INCLUDED__
#define __WATER_HPP_INCLUDED__

#include "irrlicht.h"
#include "MovingWater.hpp"

class Water
{
    public:
        Water();
        virtual ~Water();
        void load(irr::scene::ISceneManager* smgr, irr::f32 weather, irr::u32 disableShaders);
        void update(irr::f32 tideHeight, irr::core::vector3df viewPosition, irr::u32 lightLevel, irr::f32 weather);
        irr::f32 getWaveHeight(irr::f32 relPosX, irr::f32 relPosZ) const;
        irr::core::vector2df getLocalNormals(irr::f32 relPosX, irr::f32 relPosZ) const;
        irr::core::vector3df getPosition() const;
        void setVisible(bool visible);

    private:
        irr::f32 tileWidth;
        irr::scene::MovingWaterSceneNode* waterNode;
};

#endif

