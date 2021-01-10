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

#ifndef __BUOY_HPP_INCLUDED__
#define __BUOY_HPP_INCLUDED__

#include "irrlicht.h"

#include <string>
#include <cmath>

struct RadarData;

class Buoy
{
    public:
        Buoy(const std::string& name, const irr::core::vector3df& location, irr::f32 radarCrossSection, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev);
        virtual ~Buoy();
        irr::core::vector3df getPosition() const;
        void setPosition(irr::core::vector3df position);
        void setRotation(irr::core::vector3df rotation);
        irr::f32 getLength() const;
        irr::f32 getHeight() const;
        irr::f32 getRCS() const;
        RadarData getRadarData(irr::core::vector3df scannerPosition) const;
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);
        irr::scene::ISceneNode* getSceneNode() const;
    protected:
    private:
        irr::scene::IMeshSceneNode* buoy; //The scene node for the buoy.
        irr::f32 length; //For radar calculation
        irr::f32 height; //For radar calculation
        irr::f32 rcs;
};

#endif // __BUOY_HPP_INCLUDED__
