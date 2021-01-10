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
#include <vector>

class Terrain
{
    public:
        Terrain();
        virtual ~Terrain();
        void load(const std::string& worldPath, irr::scene::ISceneManager* smgr);
        irr::f32 longToX(irr::f32 longitude) const;
        irr::f32 latToZ(irr::f32 latitude) const;
        irr::f32 xToLong(irr::f32 x) const;
        irr::f32 zToLat(irr::f32 z) const;
        irr::f32 getHeight(irr::f32 x, irr::f32 z) const;
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);

    private:
        std::vector<irr::scene::ITerrainSceneNode*> terrains;
        irr::f32 primeTerrainLong;
        irr::f32 primeTerrainXWidth;
        irr::f32 primeTerrainLongExtent;
        irr::f32 primeTerrainLat;
        irr::f32 primeTerrainZWidth;
        irr::f32 primeTerrainLatExtent;
};

#endif
