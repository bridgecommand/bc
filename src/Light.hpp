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

#ifndef __LIGHT_HPP_INCLUDED__
#define __LIGHT_HPP_INCLUDED__

#include "graphics/Types.hpp"

#include <cstdint>

// Forward declarations for Irrlicht pointer types
namespace irr {
    namespace scene {
        class ISceneManager;
        class ISceneNode;
        class ILightSceneNode;
    }
}

class Light
{
    public:
        Light();
        virtual ~Light();
        void load(irr::scene::ISceneManager* smgr, float sunRise, float sunSet, irr::scene::ISceneNode* parent);
        void update(float scenarioTime);
        bc::graphics::Color getLightSColor() const;
        uint32_t getLightLevel() const;

    private:
        uint32_t lightLevel;
        bc::graphics::Color ambientColor;
        irr::scene::ISceneManager* smgr;
        float sunRise;
        float sunSet;
        irr::scene::ISceneNode* parent;
        irr::scene::ILightSceneNode* directionalLight;
};

#endif // __LIGHT_HPP_INCLUDED__
