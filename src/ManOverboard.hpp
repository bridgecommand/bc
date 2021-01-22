/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2017 James Packer

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

#ifndef __MANOVERBOARD_HPP_INCLUDED__
#define __MANOVERBOARD_HPP_INCLUDED__

#include "irrlicht.h"

//Forward declarations
class SimulationModel;
class Terrain;

class ManOverboard
{
    public:
        ManOverboard(const irr::core::vector3df& location, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev, SimulationModel* model, Terrain* terrain);
        irr::core::vector3df getPosition() const;
        void setVisible(bool isVisible);
        bool getVisible() const;
        void setPosition(irr::core::vector3df position);
        void setRotation(irr::core::vector3df rotation);
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);
        void update(irr::f32 deltaTime, irr::f32 tideHeight);
        irr::scene::ISceneNode* getSceneNode() const;
    protected:
    private:
        irr::scene::IMeshSceneNode* man; //The scene node for the man overboard model.
        SimulationModel* model;
        Terrain* terrain;
};

#endif
