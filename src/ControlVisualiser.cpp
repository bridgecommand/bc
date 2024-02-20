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

#include "ControlVisualiser.hpp"

ControlVisualiser::ControlVisualiser()
{

}

ControlVisualiser::~ControlVisualiser()
{
    //dtor
}

void ControlVisualiser::load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, irr::core::vector3df offset, irr::f32 scale, irr::u32 rotationAxis)
{
    // Initialise to null in case we don't want to display the control;
    controlNode = 0;
    
    this->displayValue = 0;
    this->parent = parent;
    this->offset = offset;
    this->rotationAxis = rotationAxis;
    
    // Load unless 'no data' marker for position is set
    if (offset.X != -999 || offset.Y != -999 || offset.Z != -999) 
    {
        // Load from media/throttle.x
        // TODO: Check path is sensible here?
        irr::scene::IMesh* controlMesh = smgr->getMesh("media/throttle.x");
        if (controlMesh != 0) 
        {
            controlNode = smgr->addMeshSceneNode(controlMesh);
            controlNode->setScale(irr::core::vector3df(scale,scale,scale));
        }
    }
}

void ControlVisualiser::update(irr::f32 displayValue)
{

    this->displayValue = displayValue;

    if (controlNode != 0) 
    {
        irr::core::matrix4 m;
        irr::core::vector3df offsetTransformed;

        //link camera rotation to shipNode
        // get transformation matrix of node
        m.setRotationDegrees(parent->getRotation());

        // transform offset('offset' is relative to the local ship coordinates, and stays the same.)
        //'offsetTransformed' is transformed into the global coordinates

        m.transformVect(offsetTransformed,offset);

        // Set display position
        controlNode->setPosition(parent->getPosition() + offsetTransformed);
        
        // Set position, choosing which axis to use
        if (rotationAxis == 0) 
        {
            controlNode->setRotation(parent->getRotation()+irr::core::vector3df(displayValue,0,0));
        } 
        else if (rotationAxis == 1) 
        {
            controlNode->setRotation(parent->getRotation()+irr::core::vector3df(0,displayValue,0));
        }
        else 
        {
            controlNode->setRotation(parent->getRotation()+irr::core::vector3df(0,0,displayValue));
        }
    }

}