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

#ifndef __CONTROLVISUALISER_HPP_INCLUDED__
#define __CONTROLVISUALISER_HPP_INCLUDED__

#include <cstdint>
#include "graphics/Types.hpp"

namespace irr { namespace scene { class ISceneManager; class ISceneNode; class IMeshSceneNode; } }

class ControlVisualiser
{
    public:
        ControlVisualiser();
        virtual ~ControlVisualiser();

        void load(irr::scene::ISceneManager* smgr, irr::scene::ISceneNode* parent, bc::graphics::Vec3 offset, float scale, uint32_t rotationAxis, uint32_t controlType);
        void update(float displayValue);
        irr::scene::ISceneNode* getSceneNode();

    private:
        irr::scene::IMeshSceneNode* controlNode;
        float displayValue;
        uint32_t rotationAxis;
};

#endif