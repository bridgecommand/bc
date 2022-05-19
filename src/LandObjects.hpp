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

#ifndef __LANDOBJECTS_HPP_INCLUDED__
#define __LANDOBJECTS_HPP_INCLUDED__

#include "irrlicht.h"

#include <vector>
#include <string>

//Forward declarations
class SimulationModel;
class Terrain;
class LandObject;

class LandObjects
{
    public:
        LandObjects();
        virtual ~LandObjects();
        void load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model, Terrain* terrain, irr::IrrlichtDevice* dev);
        irr::u32 getNumber() const;
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);

    private:
        std::vector<LandObject> landObjects;
};

#endif
