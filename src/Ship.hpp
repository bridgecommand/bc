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
        float getLength() const;
        float getBreadth() const;
        float getHeightCorrection() const;
        float getEstimatedDisplacement() const;
        void setHeading(float hdg);
        void setSpeed(float spd);
        float getHeading() const;
        float getSpeed() const; //m/s
        void moveNode(float deltaX, float deltaY, float deltaZ);
        void setPosition(float xPos, float yPos);
        irr::u32 getMMSI() const;
        void setMMSI(irr::u32 mmsi);

    protected:

        irr::scene::IAnimatedMeshSceneNode* ship; //The scene node for the own ship.
        float hdg;
        float xPos;
        float yPos;
        float zPos;
        float axialSpd;
        float length;
        float breadth;
        float draught;
        float airDraught;
        float heightCorrection;
        float angleCorrection;
// DEE_DEC22 vvvv angle corrections about other axis to allow easier import of other cood systems models and to model trim and list
        float angleCorrectionRoll;
        float angleCorrectionPitch;
// DEE_DEC22
	int controlMode;
        bool positionManuallyUpdated; //If position has been updated, and shouldn't be updated again this loop
        irr::u32 mmsi;
        enum CONTROL_MODE
        {
            MODE_AUTO = 0,
            MODE_ENGINE = 1
        };
};

#endif // __SHIP_HPP_INCLUDED__
