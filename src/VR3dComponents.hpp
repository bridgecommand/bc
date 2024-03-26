/*   Bridge Command 5 Ship Simulator
     Copyright (C) 2024 James Packer

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

#ifndef __VR3DCOMPONENTS_HPP_INCLUDED__
#define __VR3DCOMPONENTS_HPP_INCLUDED__

#include "irrlicht.h"

class VR3dComponents
{
public:
    VR3dComponents(irr::scene::ISceneManager* smgr, 
        irr::video::IVideoDriver* driver, 
        irr::scene::ISceneNode* mainCameraSceneNode, 
        irr::u32 su, 
        irr::u32 sh);
    void showHUDScreen(bool shown);
    void updateHUDTexture();
    void updateControllerPositions(
        irr::core::vector3df vrLeftGripPosition,
        irr::core::vector3df vrRightGripPosition,
        irr::core::vector3df vrLeftAimPosition,
        irr::core::vector3df vrRightAimPosition,
        irr::core::quaternion vrLeftGripOrientation,
        irr::core::quaternion vrRightGripOrientation,
        irr::core::quaternion vrLeftAimOrientation,
        irr::core::quaternion vrRightAimOrientation
    );

private:
    irr::scene::ISceneManager* smgr;
    irr::video::IVideoDriver* driver;
    irr::scene::ISceneNode* hudScreen;
    irr::video::ITexture* hudTexture;

    irr::scene::ISceneNode* leftController;
    irr::scene::ISceneNode* rightController;
};

#endif