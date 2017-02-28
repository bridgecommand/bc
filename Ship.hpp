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

//Parent class for own and other ships - not used un-extended

#ifndef __SHIP_HPP_INCLUDED__
#define __SHIP_HPP_INCLUDED__

#include "irrlicht.h"
#include <string>

//Forward declarations
class SimulationModel;

class Ship
{
    public:
        Ship();
        virtual ~Ship();

        irr::scene::IMeshSceneNode* getSceneNode() const;
        irr::core::vector3df getRotation() const;
        irr::core::vector3df getPosition() const;
        irr::f32 getLength() const;
        irr::f32 getWidth() const;
        irr::f32 getHeightCorrection() const;
        void setHeading(irr::f32 hdg);
        void setSpeed(irr::f32 spd);
        irr::f32 getHeading() const;
        irr::f32 getSpeed() const; //m/s
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);
        void setPosition(irr::f32 xPos, irr::f32 yPos);

    protected:

        irr::scene::IAnimatedMeshSceneNode* ship; //The scene node for the own ship.
        irr::f32 hdg;
        irr::f32 xPos;
        irr::f32 yPos;
        irr::f32 zPos;
        irr::f32 spd;
        irr::f32 length;
        irr::f32 width;
        irr::f32 heightCorrection;
        irr::f32 angleCorrection;
        int controlMode;
        bool positionManuallyUpdated; //If position has been updated, and shouldn't be updated again this loop
        enum CONTROL_MODE
        {
            MODE_AUTO = 0,
            MODE_ENGINE = 1
        };
};

#endif // __SHIP_HPP_INCLUDED__
