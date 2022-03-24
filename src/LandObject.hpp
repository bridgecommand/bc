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

#ifndef __LANDOBJECT_HPP_INCLUDED__
#define __LANDOBJECT_HPP_INCLUDED__

#include "irrlicht.h"

#include <string>

//Forward declarations
class Terrain;

class LandObject
{
    public:
        LandObject(const std::string& name, const std::string& worldName, const irr::core::vector3df& location, irr::f32 rotation, bool collisionObject, bool radarObject, Terrain* terrain, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev);
        virtual ~LandObject();
        irr::core::vector3df getPosition() const;
        void moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ);
    protected:
    private:
        irr::scene::IMeshSceneNode* landObject; //The scene node for the object.
        irr::IrrlichtDevice* device;
        irr::f32 findContactYFromRay(irr::core::line3d<irr::f32> ray);
};

#endif
