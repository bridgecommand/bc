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

#ifndef __TERRAIN_HPP_INCLUDED__
#define __TERRAIN_HPP_INCLUDED__

#include "irrlicht.h"

#include <string>

class Terrain
{
    public:
        Terrain();
        virtual ~Terrain();
        void load(const std::string& worldPath, irr::scene::ISceneManager* smgr);
        irr::f32 longToX(irr::f32 longitude) const;
        irr::f32 latToZ(irr::f32 latitude) const;
        irr::f32 getHeight(irr::f32 x, irr::f32 z) const;
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);

    private:
        irr::scene::ITerrainSceneNode* terrain;
        irr::f32 terrainLong;
        irr::f32 terrainXWidth;
        irr::f32 terrainLongExtent;
        irr::f32 terrainLat;
        irr::f32 terrainZWidth;
        irr::f32 terrainLatExtent;
};

#endif
