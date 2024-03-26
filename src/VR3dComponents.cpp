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

#include "VR3dComponents.hpp"

VR3dComponents::VR3dComponents(irr::scene::ISceneManager* smgr,
    irr::video::IVideoDriver* driver, 
    irr::scene::ISceneNode* mainCameraSceneNode, 
    irr::u32 su, 
    irr::u32 sh) {
    
    // Store pointers to scene manager and driver
    this->smgr = smgr;
    this->driver = driver;

    // Create mesh and scene node for HUD
    irr::f32 hudRatio = 0.75;
    if (su > 0 && sh > 0) {
        hudRatio = (irr::f32)sh / (irr::f32)su;
    }

    irr::scene::IMesh* hudPlane = smgr->getGeometryCreator()->createPlaneMesh(irr::core::dimension2d<irr::f32>(3.0, 3.0 * hudRatio));
    smgr->getMeshManipulator()->setVertexColorAlpha(hudPlane, 128); // Set to be 50% transparent
    hudScreen = smgr->addMeshSceneNode(hudPlane,
        mainCameraSceneNode,
        -1,
        irr::core::vector3df(0.0, 0.0, 2.0),
        irr::core::vector3df(-90.0, 0.0, 0.0));

    hudScreen->setMaterialFlag(irr::video::EMF_LIGHTING, false);
    hudScreen->setMaterialFlag(irr::video::EMF_ZBUFFER, false);
    hudScreen->setMaterialType(irr::video::EMT_TRANSPARENT_VERTEX_ALPHA);
    hudScreen->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);

    hudTexture = 0;
    if (driver->queryFeature(irr::video::EVDF_RENDER_TO_TARGET)) {
        hudTexture = driver->addRenderTargetTexture(irr::core::dimension2d<irr::u32>(su, sh), "HUD");
        hudScreen->setMaterialTexture(0, hudTexture); // set material to render target
    }

    // Hide HUD by default
    hudScreen->setVisible(false);

    // Create simple model for the controllers
    leftController = smgr->addCubeSceneNode(1.0, 
        0, 
        -1, 
        irr::core::vector3df(0, 0, 0),
        irr::core::vector3df(0, 0, 0),
	    irr::core::vector3df(0.05f, 0.10f, 0.05f));
    
    rightController = smgr->addCubeSceneNode(1.0, 
        0, 
        -1, 
        irr::core::vector3df(0, 0, 0),
        irr::core::vector3df(0, 0, 0),
	    irr::core::vector3df(0.05f, 0.10f, 0.05f));
}

void VR3dComponents::showHUDScreen(bool shown) {
    if (hudScreen) {
        hudScreen->setVisible(shown);
    }
}

void VR3dComponents::updateHUDTexture() {
    if (hudTexture) {
        driver->setRenderTarget(hudTexture, true, true, irr::video::SColor(0, 128, 128, 128));
        // Draw GUI, this should have been updated in guiMain.drawGUI() above
        smgr->getGUIEnvironment()->drawAll();
        //set back usual render target
        driver->setRenderTarget(0, 0);
    }
}

void VR3dComponents::updateControllerPositions(
    irr::core::vector3df baseViewPosition,
    irr::core::matrix4 baseViewRotation,
    irr::core::vector3df vrLeftGripPosition,
    irr::core::vector3df vrRightGripPosition,
    irr::core::vector3df vrLeftAimPosition,
    irr::core::vector3df vrRightAimPosition,
    irr::core::quaternion vrLeftGripOrientation,
    irr::core::quaternion vrRightGripOrientation,
    irr::core::quaternion vrLeftAimOrientation,
    irr::core::quaternion vrRightAimOrientation
) {
    
    // Transform left grip position based on orientation of the camera's parent
    baseViewRotation.transformVect(vrLeftGripPosition);
    baseViewRotation.transformVect(vrRightGripPosition);
    baseViewRotation.transformVect(vrLeftAimPosition);
    baseViewRotation.transformVect(vrRightAimPosition);

    // TODO: Transform orientation based on parent
    
    // Set these positions
    leftController->setPosition(baseViewPosition + vrLeftAimPosition);
    rightController->setPosition(baseViewPosition + vrRightAimPosition);

    // TODO: Set the orientation


}